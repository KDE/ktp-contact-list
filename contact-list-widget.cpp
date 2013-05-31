/*
 *  Contact List Widget
 *  Copyright (C) 2011  Martin Klapetek <martin.klapetek@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */


#include "contact-list-widget.h"
#include "contact-list-widget_p.h"

#include <TelepathyQt/PendingChannelRequest>

#include <KTp/types.h>

#include <KTp/Models/contacts-filter-model.h>
#include <KTp/Models/accounts-tree-proxy-model.h>
#include <KTp/Models/groups-tree-proxy-model.h>

#include <KTp/actions.h>
#include <KTp/contact.h>

#include <KGlobal>
#include <KSharedConfig>
#include <KConfigGroup>
#include <KDebug>
#include <KMessageBox>
#include <KLocalizedString>
#include <KDialog>
#include <KFileDialog>
#include <KSettings/Dialog>
#include <KMenu>
#include <KPixmapSequence>
#include <KPixmapSequenceWidget>
#include <KNotifyConfigWidget>

#include <QHeaderView>
#include <QLabel>
#include <QApplication>
#include <QDropEvent>
#include <QDragMoveEvent>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QPainter>
#include <QPixmap>

#include <QAction>

#include <kpeople/personsmodel.h>
#include <kpeople/ktptranslationproxy.h>
#include <kpeople/personpluginmanager.h>
#include <kpeople/basepersonsdatasource.h>
#include <kpeople/impersonsdatasource.h>

#include "contact-delegate.h"
#include "contact-delegate-compact.h"
#include "contact-overlays.h"
#include "kpeople-proxy.h"
#include "context-menu.h"

ContactListWidget::ContactListWidget(QWidget *parent)
    : QTreeView(parent),
      d_ptr(new ContactListWidgetPrivate)
{
    Q_D(ContactListWidget);

    KSharedConfigPtr config = KGlobal::config();
    KConfigGroup guiConfigGroup(config, "GUI");

    d->busyWidget = new KPixmapSequenceWidget(this);
    //apparently KPixmapSequence has only few sizes, 22 is one of them
    d->busyWidget->setSequence(KPixmapSequence("process-working", 22));

//     d->delegate = new ContactDelegate();
    d->compactDelegate = new ContactDelegateCompact(ContactDelegateCompact::Normal, this);
    connect(d->compactDelegate, SIGNAL(repaintItem(QModelIndex)),
            this->viewport(), SLOT(update(QModelIndex))); //update(QModelIndex)

    d->model = new PersonsModel(PersonsModel::FeatureIM,
                                PersonsModel::FeatureAvatars |
                                PersonsModel::FeatureGroups,
                                this);
    connect(d->model, SIGNAL(peopleAdded()),
            this, SLOT(reset()));
    connect(d->model, SIGNAL(peopleAdded()),
            this, SLOT(onShowAllContacts()));

    d->translationProxy = new KTpTranslationProxy(this);
    d->translationProxy->setSourceModel(d->model);

//     d->groupsProxy = new KTp::GroupsTreeProxyModel(d->translationProxy);

    d->modelFilter = new KTp::ContactsFilterModel(this);
    d->modelFilter->setDynamicSortFilter(true);
    d->modelFilter->setSortRole(Qt::DisplayRole);
    d->modelFilter->setSourceModel(d->translationProxy);
    d->modelFilter->setCapabilityFilterFlags(KTp::ContactsFilterModel::DoNotFilterByCapability);
    d->modelFilter->setSubscriptionStateFilterFlags(KTp::ContactsFilterModel::DoNotFilterBySubscription);
    d->modelFilter->sort(0);

    setItemDelegate(d->compactDelegate);
    setModel(d->modelFilter);
    d->compactDelegate->setListMode(ContactDelegateCompact::Normal);

//     d->contextMenu = new ContextMenu(this);
//     d->contextMenu->setAccountManager(d->presenceModel->accountManager());

    loadGroupStatesFromConfig();

//
//     connect(d->groupsModel, SIGNAL(operationFinished(Tp::PendingOperation*)),
//             this, SIGNAL(genericOperationFinished(Tp::PendingOperation*)));
//     connect(d->modelFilter, SIGNAL(rowsInserted(QModelIndex,int,int)),
//             this, SLOT(onNewGroupModelItemsInserted(QModelIndex,int,int)));

    header()->hide();
    setRootIsDecorated(false);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setContextMenuPolicy(Qt::CustomContextMenu);
    setIndentation(0);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setMouseTracking(true);
    setExpandsOnDoubleClick(false); //the expanding/collapsing is handled manually
//     setDragEnabled(true);
//     viewport()->setAcceptDrops(true);
//     setDropIndicatorShown(true);

//     QString delegateMode = guiConfigGroup.readEntry("selected_delegate", "normal");

//     if (delegateMode == QLatin1String("full")) {
//         setItemDelegate(d->delegate);
//     } else if (delegateMode == QLatin1String("mini")) {
//         setItemDelegate(d->compactDelegate);
//         d->compactDelegate->setListMode(ContactDelegateCompact::Mini);
//     } else {

//     }

    QString shownContacts = guiConfigGroup.readEntry("shown_contacts", "unblocked");
    if (shownContacts == "unblocked") {
        d->modelFilter->setSubscriptionStateFilterFlags(KTp::ContactsFilterModel::HideBlocked);
    } else if (shownContacts == "blocked") {
        d->modelFilter->setSubscriptionStateFilterFlags(KTp::ContactsFilterModel::ShowOnlyBlocked);
    } else {
        d->modelFilter->setSubscriptionStateFilterFlags(KTp::ContactsFilterModel::DoNotFilterBySubscription);
    }

    connect(this, SIGNAL(clicked(QModelIndex)),
            this, SLOT(onContactListClicked(QModelIndex)));

    connect(this, SIGNAL(doubleClicked(QModelIndex)),
            this, SLOT(onContactListDoubleClicked(QModelIndex)));

    connect(this, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(onCustomContextMenuRequested(QPoint)));
//     connect(d->delegate, SIGNAL(repaintItem(QModelIndex)),
//             this->viewport(), SLOT(repaint())); //update(QModelIndex)
}


ContactListWidget::~ContactListWidget()
{
    delete d_ptr;
}

Tp::AccountManagerPtr ContactListWidget::accountManager() const
{
    Q_D(const ContactListWidget);
    IMPersonsDataSource *b = dynamic_cast<IMPersonsDataSource*>(PersonPluginManager::presencePlugin());

    if (!b) {
        kDebug() << "no cast! will now crash"; //FIXME?
    }

    return b->accountManager();
}

void ContactListWidget::showSettingsKCM()
{
    KSettings::Dialog *dialog = new KSettings::Dialog(this);

    KService::Ptr tpAccKcm = KService::serviceByDesktopName("kcm_ktp_accounts");

    if (!tpAccKcm) {
        KMessageBox::error(this,
                           i18n("It appears you do not have the IM Accounts control module installed. Please install ktp-accounts-kcm package."),
                           i18n("IM Accounts KCM Plugin Is Not Installed"));
    }

    dialog->addModule("kcm_ktp_accounts");
    dialog->addModule("kcm_ktp_integration_module");

    // Setup notifications menu
    KNotifyConfigWidget *notificationWidget = new KNotifyConfigWidget(dialog);
    notificationWidget->setApplication("ktelepathy");
    connect(dialog, SIGNAL(accepted()),
            notificationWidget, SLOT(save()));

    connect(notificationWidget, SIGNAL(changed(bool)),
            dialog, SLOT(enableButtonApply(bool)));

    connect(dialog,SIGNAL(applyClicked()),
            notificationWidget, SLOT(save()));

    KPageWidgetItem* notificationPage = new KPageWidgetItem(notificationWidget, i18n("Notifications"));
    notificationPage->setIcon(KIcon("preferences-desktop-notification"));
    dialog->addPage(notificationPage);

    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->exec();
}

void ContactListWidget::onContactListClicked(const QModelIndex& index)
{
    Q_D(ContactListWidget);

    kDebug() << index.data(PersonsModel::UriRole).toString();
    kDebug() << index.data(PersonsModel::PresenceTypeRole).toString();

    kDebug() << d->model->rowCount(index);

    if (!index.isValid()) {
        return;
    }

    if (index.data(KTp::RowTypeRole).toInt() == KTp::AccountRowType
        || index.data(KTp::RowTypeRole).toInt() == KTp::GroupRowType) {

        KSharedConfigPtr config = KSharedConfig::openConfig(QLatin1String("ktelepathyrc"));
        KConfigGroup groupsConfig = config->group("GroupsState");

        QString groupId = index.data(KTp::IdRole).toString();

        if (isExpanded(index)) {
            collapse(index);
            groupsConfig.writeEntry(groupId, false);
        } else {
            expand(index);
            groupsConfig.writeEntry(groupId, true);
        }

        groupsConfig.config()->sync();

        //replace the old value or insert new value if it isn't there yet
        d->groupStates.insert(groupId, isExpanded(index));
    }

    //In order to collapse previously selected metacontact when normal contact is selected,
    //we need to change the selected index inside the delegate, that will return the default
    //row height for the previously selected metacontact and thus collapse it.
    if (index.data(KTp::RowTypeRole).toUInt() == KTp::PersonRowType) {
        if (d->compactDelegate->selectedIndex().isValid()) {
            collapse(d->compactDelegate->selectedIndex());
        }
        d->compactDelegate->setSelectedIndex(index);
        expand(index);
    }

    if (index.data(KTp::RowTypeRole).toUInt() == KTp::ContactRowType) {
        if (index.parent().data(KTp::RowTypeRole).toUInt() != KTp::PersonRowType) {
            if (d->compactDelegate->selectedIndex().isValid()) {
                collapse(d->compactDelegate->selectedIndex());
            }
            d->compactDelegate->setSelectedIndex((index));
        }
    }
}

void ContactListWidget::onContactListDoubleClicked(const QModelIndex& index)
{
    if (!index.isValid()) {
        return;
    }

    if (index.data(KTp::RowTypeRole).toInt() == KTp::ContactRowType) {
        Tp::AccountPtr account = index.data(KTp::AccountRole).value<Tp::AccountPtr>();

        if (!account->isOnline()) {
            KGuiItem yes(i18n("Connect account %1", account->displayName()), QLatin1String("dialog-ok"));
            if (KMessageBox::questionYesNo(this,
                                           i18n("The account for this contact is disconnected. Do you want to connect it?"),
                                           i18n("Account offline"),
                                           yes,
                                           KStandardGuiItem::no()) == KMessageBox::Yes) {

                if (!account->isEnabled()) {
                    Tp::PendingOperation *op = account->setEnabled(true);
                    op->setProperty("contactId", index.data(KTp::IdRole).toString());
                    connect(op, SIGNAL(finished(Tp::PendingOperation*)),
                            this, SLOT(accountEnablingFinished(Tp::PendingOperation*)));
                } else {
                    account->ensureTextChat(index.data(KTp::IdRole).toString(),
                                            QDateTime::currentDateTime(),
                                            QLatin1String("org.freedesktop.Telepathy.Client.KTp.TextUi"));
                }

                return;
            }
        }

        KTp::ContactPtr contact = index.data(KTp::ContactRole).value<KTp::ContactPtr>();

        if (!contact.isNull()) {
            startTextChannel(account, contact);
        }
    } else if (index.data(KTp::RowTypeRole).toInt() == KTp::PersonRowType) {
        Tp::ConnectionPresenceType mostOnlinePresence = Tp::ConnectionPresenceTypeOffline;
        QModelIndex mostOnlineIndex = QModelIndex();

        for (int i = 0; i < index.model()->rowCount(index); i++) {
            Tp::ConnectionPresenceType presence = (Tp::ConnectionPresenceType)index.child(i, 0).data(KTp::ContactPresenceTypeRole).toUInt();
            if (KTp::Presence::sortPriority(presence) < KTp::Presence::sortPriority(mostOnlinePresence)) {
                mostOnlinePresence = presence;
                mostOnlineIndex = index.child(i, 0);
            }
        }

        if (mostOnlineIndex.isValid()) {
            onContactListDoubleClicked(mostOnlineIndex);
        }
    }
}

void ContactListWidget::accountEnablingFinished(Tp::PendingOperation *op)
{
    if (op->isError()) {
        kWarning() << "Account enabling failed" << op->errorMessage();
        return;
    }

    Tp::AccountPtr account = Tp::AccountPtr(qobject_cast<Tp::Account*>(sender()));

    if (account.isNull()) {
        kWarning() << "Null account passed!";
        return;
    }

    account->ensureTextChat(op->property("contactId").toString(),
                            QDateTime::currentDateTime(),
                            QLatin1String("org.freedesktop.Telepathy.Client.KTp.TextUi"));
}

void ContactListWidget::addOverlayButtons()
{
//     Q_D(ContactListWidget);
//
//     TextChannelContactOverlay *textOverlay  = new TextChannelContactOverlay(this);
//     AudioChannelContactOverlay *audioOverlay = new AudioChannelContactOverlay(this);
//     VideoChannelContactOverlay *videoOverlay = new VideoChannelContactOverlay(this);
//
//     FileTransferContactOverlay *fileOverlay  = new FileTransferContactOverlay(this);
//     DesktopSharingContactOverlay *desktopOverlay = new DesktopSharingContactOverlay(this);
//     LogViewerOverlay *logViewerOverlay = new LogViewerOverlay(this);
//
//     d->delegate->installOverlay(textOverlay);
//     d->delegate->installOverlay(audioOverlay);
//     d->delegate->installOverlay(videoOverlay);
//     d->delegate->installOverlay(fileOverlay);
//     d->delegate->installOverlay(desktopOverlay);
//     d->delegate->installOverlay(logViewerOverlay);
//
//     d->delegate->setViewOnAllOverlays(this);
//     d->delegate->setAllOverlaysActive(true);
//
//     connect(textOverlay, SIGNAL(overlayActivated(QModelIndex)),
//             d->delegate, SLOT(hideStatusMessageSlot(QModelIndex)));
//
//     connect(textOverlay, SIGNAL(overlayHidden()),
//             d->delegate, SLOT(reshowStatusMessageSlot()));
//
//
//     connect(textOverlay, SIGNAL(activated(Tp::AccountPtr, Tp::ContactPtr)),
//             this, SLOT(startTextChannel(Tp::AccountPtr, Tp::ContactPtr)));
//
//     connect(fileOverlay, SIGNAL(activated(Tp::AccountPtr, Tp::ContactPtr)),
//             this, SLOT(startFileTransferChannel(Tp::AccountPtr, Tp::ContactPtr)));
//
//     connect(audioOverlay, SIGNAL(activated(Tp::AccountPtr, Tp::ContactPtr)),
//             this, SLOT(startAudioChannel(Tp::AccountPtr, Tp::ContactPtr)));
//
//     connect(videoOverlay, SIGNAL(activated(Tp::AccountPtr, Tp::ContactPtr)),
//             this, SLOT(startVideoChannel(Tp::AccountPtr, Tp::ContactPtr)));
//
//     connect(desktopOverlay, SIGNAL(activated(Tp::AccountPtr, Tp::ContactPtr)),
//             this, SLOT(startDesktopSharing(Tp::AccountPtr, Tp::ContactPtr)));
//
//     connect(logViewerOverlay, SIGNAL(activated(Tp::AccountPtr,Tp::ContactPtr)),
//             this, SLOT(startLogViewer(Tp::AccountPtr, Tp::ContactPtr)));
//
//     connect(this, SIGNAL(enableOverlays(bool)),
//             textOverlay, SLOT(setActive(bool)));
//
//     connect(this, SIGNAL(enableOverlays(bool)),
//             audioOverlay, SLOT(setActive(bool)));
//
//     connect(this, SIGNAL(enableOverlays(bool)),
//             videoOverlay, SLOT(setActive(bool)));
//
//     connect(this, SIGNAL(enableOverlays(bool)),
//             fileOverlay, SLOT(setActive(bool)));
//
//     connect(this, SIGNAL(enableOverlays(bool)),
//             desktopOverlay, SLOT(setActive(bool)));
//
//     connect(this, SIGNAL(enableOverlays(bool)),
//             logViewerOverlay, SLOT(setActive(bool)));
}

void ContactListWidget::toggleGroups(bool show)
{
    Q_D(ContactListWidget);

    if (show) {
        d->modelFilter->setSourceModel(d->groupsProxy);
        for (int i = 0; i < d->modelFilter->rowCount(); i++) {
            expand(d->modelFilter->index(i, 0));
        }
    } else {
        d->modelFilter->setSourceModel(d->translationProxy);
    }
}

void ContactListWidget::toggleOfflineContacts(bool show)
{
    Q_D(ContactListWidget);

    d->showOffline = show;
    d->modelFilter->setPresenceTypeFilterFlags(show ? KTp::ContactsFilterModel::DoNotFilterByPresence : KTp::ContactsFilterModel::ShowOnlyConnected);
    d->modelFilter->sort(0);
}

void ContactListWidget::toggleSortByPresence(bool sort)
{
    Q_D(ContactListWidget);

    //typecast to int before passing to setSortRole to avoid false cpp warning about mixing enum types
    d->model->setSortRole(sort ? (int)KTp::ContactPresenceTypeRole : (int)Qt::DisplayRole);
}

void ContactListWidget::startTextChannel(const Tp::AccountPtr &account, const Tp::ContactPtr &contact)
{
    Tp::PendingOperation *op = KTp::Actions::startChat(account, contact, true);
    connect(op, SIGNAL(finished(Tp::PendingOperation*)),
            SIGNAL(genericOperationFinished(Tp::PendingOperation*)));
}

void ContactListWidget::startAudioChannel(const Tp::AccountPtr &account, const Tp::ContactPtr &contact)
{
    Tp::PendingOperation *op = KTp::Actions::startAudioCall(account, contact);
    connect(op, SIGNAL(finished(Tp::PendingOperation*)),
            SIGNAL(genericOperationFinished(Tp::PendingOperation*)));
}

void ContactListWidget::startVideoChannel(const Tp::AccountPtr &account, const Tp::ContactPtr &contact)
{
    Tp::PendingOperation *op = KTp::Actions::startAudioVideoCall(account, contact);
    connect(op, SIGNAL(finished(Tp::PendingOperation*)),
            SIGNAL(genericOperationFinished(Tp::PendingOperation*)));
}

void ContactListWidget::startDesktopSharing(const Tp::AccountPtr &account, const Tp::ContactPtr &contact)
{
    Tp::PendingOperation *op = KTp::Actions::startDesktopSharing(account, contact);
    connect(op, SIGNAL(finished(Tp::PendingOperation*)),
            SIGNAL(genericOperationFinished(Tp::PendingOperation*)));
}

void ContactListWidget::startLogViewer(const Tp::AccountPtr &account, const Tp::ContactPtr &contact)
{
    //log viewer is not a Tp handler so does not return a pending operation
    KTp::Actions::openLogViewer(account, contact);
}

void ContactListWidget::startFileTransferChannel(const Tp::AccountPtr &account, const Tp::ContactPtr &contact)
{
    kDebug() << "Requesting file transfer for contact" << contact->alias();

    QStringList filenames = KFileDialog::getOpenFileNames(KUrl("kfiledialog:///FileTransferLastDirectory"),
                                                          QString(),
                                                          this,
                                                          i18n("Choose files to send to %1", contact->alias()));

    if (filenames.isEmpty()) { // User hit cancel button
        return;
    }

    requestFileTransferChannels(account, contact, filenames);
}

void ContactListWidget::requestFileTransferChannels(const Tp::AccountPtr &account,
                                                    const Tp::ContactPtr &contact,
                                                    const QStringList &filenames)
{
    Q_FOREACH (const QString &filename, filenames) {
        Tp::PendingOperation *op = KTp::Actions::startFileTransfer(account, contact, filename);
        connect(op, SIGNAL(finished(Tp::PendingOperation*)),
                SIGNAL(genericOperationFinished(Tp::PendingOperation*)));
    }
}

void ContactListWidget::onSwitchToFullView()
{
//     Q_D(ContactListWidget);
//
//     setItemDelegate(d->delegate);
//     doItemsLayout();
//
//     emit enableOverlays(true);
//
//     KSharedConfigPtr config = KGlobal::config();
//     KConfigGroup guiConfigGroup(config, "GUI");
//     guiConfigGroup.writeEntry("selected_delegate", "full");
//     guiConfigGroup.config()->sync();
}

void ContactListWidget::onSwitchToCompactView()
{
    Q_D(ContactListWidget);

    setItemDelegate(d->compactDelegate);
    d->compactDelegate->setListMode(ContactDelegateCompact::Normal);
    doItemsLayout();

    emit enableOverlays(false);

    KSharedConfigPtr config = KGlobal::config();
    KConfigGroup guiConfigGroup(config, "GUI");
    guiConfigGroup.writeEntry("selected_delegate", "normal");
    guiConfigGroup.config()->sync();
}

void ContactListWidget::onSwitchToMiniView()
{
    Q_D(ContactListWidget);

    setItemDelegate(d->compactDelegate);
    d->compactDelegate->setListMode(ContactDelegateCompact::Mini);;
    doItemsLayout();

    emit enableOverlays(false);

    KSharedConfigPtr config = KGlobal::config();
    KConfigGroup guiConfigGroup(config, "GUI");
    guiConfigGroup.writeEntry("selected_delegate", "mini");
    guiConfigGroup.config()->sync();
}

void ContactListWidget::onShowAllContacts()
{
    Q_D(ContactListWidget);

    d->modelFilter->setSubscriptionStateFilterFlags(KTp::ContactsFilterModel::DoNotFilterBySubscription);

    for (int i = 0; i < d->modelFilter->rowCount(); i++) {

        QModelIndex index = d->modelFilter->index(i, 0);

        if (!index.parent().isValid()) {

            //we're probably dealing with group item, so let's check if it is expanded first
            if (!isExpanded(index)) {
                //if it's not expanded, check the config if we should expand it or not
                QString groupId = index.data(KTp::IdRole).toString();
                if (d->groupStates.value(groupId)) {
                    expand(index);
                }
            }
        }
    }

    KSharedConfigPtr config = KGlobal::config();
    KConfigGroup guiConfigGroup(config, "GUI");
    guiConfigGroup.writeEntry("shown_contacts", "all");
    guiConfigGroup.config()->sync();

    d->busyWidget->hide();
}

void ContactListWidget::onShowUnblockedContacts()
{
    Q_D(ContactListWidget);

    d->modelFilter->setSubscriptionStateFilterFlags(KTp::ContactsFilterModel::HideBlocked);

    KSharedConfigPtr config = KGlobal::config();
    KConfigGroup guiConfigGroup(config, "GUI");
    guiConfigGroup.writeEntry("shown_contacts", "unblocked");
    guiConfigGroup.config()->sync();
}

void ContactListWidget::onShowBlockedContacts()
{
    Q_D(ContactListWidget);

    d->modelFilter->setSubscriptionStateFilterFlags(KTp::ContactsFilterModel::ShowOnlyBlocked);

    KSharedConfigPtr config = KGlobal::config();
    KConfigGroup guiConfigGroup(config, "GUI");
    guiConfigGroup.writeEntry("shown_contacts", "blocked");
    guiConfigGroup.config()->sync();
}

void ContactListWidget::setFilterString(const QString& string)
{
    Q_D(ContactListWidget);

//     if (string.isEmpty()) {
//         d->model->setGroupMode(d->groupMode);
//     } else {
//         d->model->setGroupMode(KTp::ContactsModel::NoGrouping);
//     }

    d->modelFilter->setPresenceTypeFilterFlags(string.isEmpty() && !d->showOffline ? KTp::ContactsFilterModel::ShowOnlyConnected : KTp::ContactsFilterModel::DoNotFilterByPresence);
    d->modelFilter->setGlobalFilterString(string);
}

void ContactListWidget::setDropIndicatorRect(const QRect &rect)
{
//     Q_D(ContactListWidget);
//
//     if (d->dropIndicatorRect != rect) {
//         d->dropIndicatorRect = rect;
//         viewport()->update();
//     }
}

// bool ContactListWidget::event(QEvent *event)
// {
//     Q_D(ContactListWidget);
//     if (event->type() == QEvent::Leave && d->delegate) {
//         d->delegate->reshowStatusMessageSlot();
//         return true;
//     }
//
//     return QTreeView::event(event);
// }

void ContactListWidget::mousePressEvent(QMouseEvent *event)
{
    Q_D(ContactListWidget);

    QTreeView::mousePressEvent(event);

//     QModelIndex index = indexAt(event->pos());
//     d->shouldDrag = false;
//     d->dragSourceGroup.clear();
//
//     // if no contact, no drag
//     if (index.data(KTp::RowTypeRole).toInt() != KTp::ContactRowType) {
//         return;
//     }
//
//     if (event->button() == Qt::LeftButton) {
//         d->shouldDrag = true;
//         d->dragStartPosition = event->pos();
//     }
}

// void ContactListWidget::mouseMoveEvent(QMouseEvent *event)
// {
//     Q_D(ContactListWidget);
//
//     QTreeView::mouseMoveEvent(event);
//
//     QModelIndex index = indexAt(event->pos());
//
//     if (!(event->buttons() & Qt::LeftButton)) {
//         return;
//     }
//
//     if (!d->shouldDrag) {
//         return;
//     }
//
//     if ((event->pos() - d->dragStartPosition).manhattanLength() < QApplication::startDragDistance()) {
//         return;
//     }
//
//     QMimeData *mimeData = new QMimeData;
//     QByteArray encodedData;
//     QDataStream stream(&encodedData, QIODevice::WriteOnly);
//
//     if (index.isValid()) {
//         ContactModelItem *contactItem = index.data(AccountsModel::ItemRole).value<ContactModelItem*>();
//         //We put a contact ID and its account ID to the stream, so we can later recreate the contact using AccountsModel
//         stream << contactItem->contact().data()->id() << d->model->accountForContactItem(contactItem).data()->objectPath();
//     }
//
//     mimeData->setData("application/vnd.telepathy.contact", encodedData);
//     QPixmap dragIndicator = QPixmap::grabWidget(this, visualRect(index).adjusted(3,3,3,3));
//
//     QDrag *drag = new QDrag(this);
//     drag->setMimeData(mimeData);
//     drag->setPixmap(dragIndicator);
//
//     drag->exec(Qt::CopyAction);
// }

// void ContactListWidget::dropEvent(QDropEvent *event)
// {
//     Q_D(ContactListWidget);
//
//     QModelIndex index = indexAt(event->pos());
//
//     if (event->mimeData()->hasUrls()) {
//         kDebug() << "It's a file!";
//
//         ContactModelItem* contactItem = index.data(AccountsModel::ItemRole).value<ContactModelItem*>();
//         Q_ASSERT(contactItem);
//
//         Tp::ContactPtr contact = contactItem->contact();
//
//         kDebug() << "Requesting file transfer for contact" << contact->alias();
//
//         Tp::AccountPtr account = d->model->accountForContactItem(contactItem);
//
//         QStringList filenames;
//         Q_FOREACH (const QUrl &url, event->mimeData()->urls()) {
//             filenames << url.toLocalFile();
//         }
//
//         if (filenames.isEmpty()) {
//             return;
//         }
//
//         QDateTime now = QDateTime::currentDateTime();
//         requestFileTransferChannels(account, contact, filenames, now);
//
//         event->acceptProposedAction();
//     } else if (event->mimeData()->hasFormat("application/vnd.telepathy.contact")) {
//         kDebug() << "It's a contact!";
//
//         QByteArray encodedData = event->mimeData()->data("application/vnd.telepathy.contact");
//         QDataStream stream(&encodedData, QIODevice::ReadOnly);
//         QList<ContactModelItem*> contacts;
//
//         while (!stream.atEnd()) {
//             QString contact;
//             QString account;
//
//             //get contact and account out of the stream
//             stream >> contact >> account;
//
//             Tp::AccountPtr accountPtr = d->model->accountPtrForPath(account);
//
//             //casted pointer is checked below, before first use
//             contacts.append(qobject_cast<ContactModelItem*>(d->model->contactItemForId(accountPtr->uniqueIdentifier(), contact)));
//         }
//
//         Q_FOREACH (ContactModelItem *contact, contacts) {
//             Q_ASSERT(contact);
//             QString group;
//             if (index.data(AccountsModel::ItemRole).canConvert<GroupsModelItem*>()) {
//                 // contact is dropped on a group, so take it's name
//                 group = index.data(GroupsModel::GroupNameRole).toString();
//             } else {
//                 // contact is dropped on another contact, so take it's parents (group) name
//                 group = index.parent().data(GroupsModel::GroupNameRole).toString();
//             }
//
//             kDebug() << contact->contact().data()->alias() << "added to group" << group;
//
//             if (!group.isEmpty()) {
//                 Tp::PendingOperation *op = contact->contact().data()->addToGroup(group);
//
//                 connect(op, SIGNAL(finished(Tp::PendingOperation*)),
//                         this, SIGNAL(genericOperationFinished(Tp::PendingOperation*)));
//             }
//         }
//         event->acceptProposedAction();
//     } else {
//         event->ignore();
//     }
//
//     setDropIndicatorRect(QRect());
// }

// void ContactListWidget::dragEnterEvent(QDragEnterEvent *event)
// {
//     if (event->mimeData()->hasUrls()) {
//         bool accepted = true;
//         // check if one of the urls isn't a local file and abort if so
//         Q_FOREACH (const QUrl& url, event->mimeData()->urls()) {
//             if (!QFileInfo(url.toLocalFile()).isFile()) {
//                     accepted = false;
//             }
//         }
//
//         if (accepted) {
//             event->acceptProposedAction();
//         } else {
//             event->ignore();
//         }
//     } else if (event->mimeData()->hasFormat("application/vnd.telepathy.contact")) {
//         event->acceptProposedAction();
//     } else {
//         event->ignore();
//     }
// }

// void ContactListWidget::dragMoveEvent(QDragMoveEvent *event)
// {
//     Q_D(ContactListWidget);
//
//     QModelIndex index = indexAt(event->pos());
//     setDropIndicatorRect(QRect());
//
//     // urls can be dropped on a contact with file transfer capability,
//     // contacts can be dropped either on a group or on another contact if GroupsModel is used
//     if (event->mimeData()->hasUrls() && index.data(AccountsModel::FileTransferCapabilityRole).toBool()) {
//         event->acceptProposedAction();
//         setDropIndicatorRect(visualRect(index));
//     } else if (event->mimeData()->hasFormat("application/vnd.telepathy.contact") &&
//                d->modelFilter->sourceModel() == d->groupsModel &&
//                (index.data(AccountsModel::ItemRole).canConvert<GroupsModelItem*>() ||
//                 index.data(AccountsModel::ItemRole).canConvert<ContactModelItem*>())) {
//         event->acceptProposedAction();
//         setDropIndicatorRect(visualRect(index));
//     } else {
//         event->ignore();
//     }
// }

// void ContactListWidget::dragLeaveEvent(QDragLeaveEvent *event)
// {
//     Q_UNUSED(event);
//     setDropIndicatorRect(QRect());
// }

// void ContactListWidget::paintEvent(QPaintEvent *event)
// {
//     Q_D(ContactListWidget);
//
//     QTreeView::paintEvent(event);
//     if (!d->dropIndicatorRect.isNull()) {
//         QStyleOption option;
//         option.init(this);
//         option.rect = d->dropIndicatorRect.adjusted(0,0,-1,-1);
//         QPainter painter(viewport());
//         style()->drawPrimitive(QStyle::PE_IndicatorItemViewItemDrop, &option, &painter, this);
//     }
// }

void ContactListWidget::drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const
{
    Q_UNUSED(painter);
    Q_UNUSED(rect);
    Q_UNUSED(index);

//     There is a 0px identation set in the constructor, with setIndentation(0).
//     Because of that, no branches are shown, so they should be disabled completely (overriding drawBranches).
//     Leaving branches enabled with 0px identation results in a 1px branch line on the left of all items,
//     which looks like an artifact.
//     See https://bugreports.qt-project.org/browse/QTBUG-26305
}

void ContactListWidget::keyPressEvent(QKeyEvent *event)
{
    //this would be normally handled by activated() signal but since we decided
    //we don't want people starting chats using single click, we can't use activated()
    //and have to do it ourselves, therefore this. Change only after discussing with the team!
    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
        //start the chat only if the index is valid and has a parent (ie. is not a group/account)
        if (currentIndex().isValid() && currentIndex().parent().isValid()) {
            onContactListDoubleClicked(currentIndex());
        }
    }

    QTreeView::keyPressEvent(event);
}

void ContactListWidget::loadGroupStatesFromConfig()
{
    Q_D(ContactListWidget);
    d->groupStates.clear();

    KConfig config(QLatin1String("ktelepathyrc"));
    KConfigGroup groupsConfig = config.group("GroupsState");

    Q_FOREACH(const QString &key, groupsConfig.keyList()) {
        bool expanded = groupsConfig.readEntry(key, false);
        d->groupStates.insert(key, expanded);
    }
}

void ContactListWidget::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_D(ContactListWidget);

    if (selectedIndexes().size() > 1) {
        bool contactsSelected = false;
        bool groupedContactSelected = false;
        bool personsSelected = false;
        Q_FOREACH (const QModelIndex &index, selectedIndexes()) {
            if (index.data(KTp::RowTypeRole).toUInt() == KTp::PersonRowType) {
                kDebug() << "Person added to selection";
                personsSelected = true;
            } else if (index.data(KTp::RowTypeRole).toUInt() == KTp::ContactRowType) {
                if (index.parent().isValid() && index.parent().data(KTp::RowTypeRole).toUInt() == KTp::PersonRowType) {
                    kDebug() << "Grouped contact added to selection";
                    groupedContactSelected = true;
                } else {
                    kDebug() << "Contact added to selection";
                    contactsSelected = true;
                }
            }
        }

        if (contactsSelected && !personsSelected && !groupedContactSelected) {
            d->listSelection = ContactListWidget::NonGroupedContact;
        } else if (contactsSelected && !personsSelected && groupedContactSelected) {
            d->listSelection = ContactListWidget::NonAndGroupedContact;
        } else if (contactsSelected && personsSelected && !groupedContactSelected) {
            d->listSelection = ContactListWidget::PersonAndNonGroupedContact;
        } else if (!contactsSelected && personsSelected && groupedContactSelected) {
            d->listSelection = ContactListWidget::PersonAndGroupedContact;
        } else {
            d->listSelection = ContactListWidget::NoSelection;
        }
    } else if (selectedIndexes().size() == 1) {
        if (selectedIndexes().at(0).data(KTp::RowTypeRole).toUInt() == KTp::PersonRowType) {
            kDebug() << "Person selected";
            d->listSelection = ContactListWidget::Person;
        } else if (selectedIndexes().at(0).data(KTp::RowTypeRole).toUInt() == KTp::ContactRowType
            && selectedIndexes().at(0).parent().data(KTp::RowTypeRole).toUInt() == KTp::PersonRowType) {

            // when a person's subcontact is selected
            kDebug() << "Grouped contact selected";
            d->listSelection = ContactListWidget::GroupedContact;
        } else if (selectedIndexes().at(0).data(KTp::RowTypeRole).toUInt() == KTp::ContactRowType
                && selectedIndexes().at(0).parent().data(KTp::RowTypeRole).toUInt() == KTp::GroupRowType) {

            // when a normal contact is selected
            kDebug() << "Ungrouped contact selected";
            d->listSelection = ContactListWidget::NoSelection;
        }  else {
            //this needs to be noselection becase we cannot operate on a single contact
            kDebug() << "Nothing selected";
            d->listSelection = ContactListWidget::NoSelection;
        }
    } else {
        d->listSelection = ContactListWidget::NoSelection;
    }


    Q_EMIT listSelectionChanged(d->listSelection);

    QTreeView::selectionChanged(selected, deselected);
}

void ContactListWidget::onGroupSelectedContacts()
{
    Q_D(ContactListWidget);

    d->model->createPersonFromIndexes(selectedIndexes());

    clearSelection();
}

void ContactListWidget::onUngroupSelectedContacts()
{
    Q_D(ContactListWidget);

    QUrl personUri;
    QList<QUrl> selectedContacts;

    if (d->listSelection == ContactListWidget::Person) {
        foreach(const QModelIndex &index, selectedIndexes()) {
            d->model->removePerson(index.data(PersonsModel::UriRole).toUrl());
        }
    } else if (d->listSelection == ContactListWidget::GroupedContact) {
        //this case is valid for only single selected contact which is a grounding occurance
        d->model->removeContactsFromPerson(selectedIndexes().at(0).parent().data(PersonsModel::UriRole).toUrl(),
                                           QList<QUrl>() << selectedIndexes().at(0).data(PersonsModel::UriRole).toUrl());
    }

    clearSelection();
}

void ContactListWidget::onCustomContextMenuRequested(const QPoint &pos)
{
    Q_D(ContactListWidget);

    QModelIndex index = indexAt(pos);

    onContactListClicked(index);

//     if (!index.isValid()) {
//         return;
//     }
//
//     KTp::RowType type = (KTp::RowType)index.data(KTp::RowTypeRole).toInt();
//
//     KMenu *menu = 0;
//
//     if (type == KTp::ContactRowType || type == KTp::PersonRowType) {
//         menu = d->contextMenu->contactContextMenu(index);
//     } else if (type == KTp::GroupRowType) {
//         menu = d->contextMenu->groupContextMenu(index);
//     }
//
//     if (menu) {
//         menu->exec(QCursor::pos());
//         menu->deleteLater();
//     }
}

void ContactListWidget::resizeEvent(QResizeEvent* event)
{
    Q_D(ContactListWidget);

    QTreeView::resizeEvent(event);

    d->busyWidget->setGeometry(size().width() / 2 - 11,
                               size().height() / 2 - 11,
                               22,
                               22);
}
