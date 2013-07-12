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

#include <TelepathyQt/AccountManager>
#include <TelepathyQt/PendingChannelRequest>
#include <TelepathyQt/PendingReady>

#include <KTp/types.h>

#include <KTp/Models/contacts-model.h>
#include <KTp/global-contact-manager.h>
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
#include <QMenu>

#include "contact-delegate.h"
#include "contact-delegate-compact.h"
#include "contact-overlays.h"

ContactListWidget::ContactListWidget(QWidget *parent)
    : QTreeView(parent),
      d_ptr(new ContactListWidgetPrivate)
{
    Q_D(ContactListWidget);

    KSharedConfigPtr config = KGlobal::config();
    KConfigGroup guiConfigGroup(config, "GUI");

    d->delegate = new ContactDelegate(this);
    d->compactDelegate = new ContactDelegateCompact(ContactDelegateCompact::Normal, this);

    d->model = new KTp::ContactsModel(this);
    d->model->setTrackUnreadMessages(true);
    d->model->setDynamicSortFilter(true);
    d->model->setSortRole(Qt::DisplayRole);


    setSortingEnabled(true);
    sortByColumn(0, Qt::AscendingOrder);
    loadGroupStatesFromConfig();

    connect(d->model, SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SLOT(onNewGroupModelItemsInserted(QModelIndex,int,int)));

    header()->hide();
    setRootIsDecorated(false);
    setSortingEnabled(true);
    setEditTriggers(NoEditTriggers);
    setContextMenuPolicy(Qt::CustomContextMenu);
    setIndentation(0);
    setMouseTracking(true);
    setExpandsOnDoubleClick(false); //the expanding/collapsing is handled manually
    setDragEnabled(false); // we handle drag&drop ourselves
    viewport()->setAcceptDrops(true);
    setDropIndicatorShown(true);

    QString delegateMode = guiConfigGroup.readEntry("selected_delegate", "normal");

    itemDelegate()->deleteLater();
    if (delegateMode == QLatin1String("full")) {
        setItemDelegate(d->delegate);
    } else if (delegateMode == QLatin1String("mini")) {
        setItemDelegate(d->compactDelegate);
        d->compactDelegate->setListMode(ContactDelegateCompact::Mini);
    } else {
        setItemDelegate(d->compactDelegate);
        d->compactDelegate->setListMode(ContactDelegateCompact::Normal);
    }

    addOverlayButtons();
    emit enableOverlays(guiConfigGroup.readEntry("selected_delegate", "normal") == QLatin1String("full"));

    QString shownContacts = guiConfigGroup.readEntry("shown_contacts", "unblocked");
    if (shownContacts == "unblocked") {
        d->model->setSubscriptionStateFilterFlags(KTp::ContactsFilterModel::HideBlocked);
    } else if (shownContacts == "blocked") {
        d->model->setSubscriptionStateFilterFlags(KTp::ContactsFilterModel::ShowOnlyBlocked);
    } else {
        d->model->setSubscriptionStateFilterFlags(KTp::ContactsFilterModel::DoNotFilterBySubscription);
    }

    connect(this, SIGNAL(clicked(QModelIndex)),
            this, SLOT(onContactListClicked(QModelIndex)));

    connect(this, SIGNAL(doubleClicked(QModelIndex)),
            this, SLOT(onContactListDoubleClicked(QModelIndex)));

    connect(d->delegate, SIGNAL(repaintItem(QModelIndex)),
            this->viewport(), SLOT(repaint())); //update(QModelIndex)
}


ContactListWidget::~ContactListWidget()
{
    delete d_ptr;
}

void ContactListWidget::setAccountManager(const Tp::AccountManagerPtr &accountManager)
{
    Q_D(ContactListWidget);

    d->accountManager = accountManager;
    d->model->setAccountManager(accountManager);

    // We set the model only when the account manager is set.
    // This fixes the weird horizontal scrollbar bug
    // See https://bugs.kde.org/show_bug.cgi?id=316260
    setModel(d->model);

    QList<Tp::AccountPtr> accounts = accountManager->allAccounts();

    if(accounts.count() == 0) {
        if (KMessageBox::questionYesNo(this,
                                       i18n("You have no IM accounts configured. Would you like to do that now?"),
                                       i18n("No Accounts Found")) == KMessageBox::Yes) {

            showSettingsKCM();
        }
    }
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
}

void ContactListWidget::onContactListDoubleClicked(const QModelIndex& index)
{
    if (!index.isValid()) {
        return;
    }

    if (index.data(KTp::RowTypeRole).toInt() == KTp::ContactRowType) {
        KTp::ContactPtr contact = index.data(KTp::ContactRole).value<KTp::ContactPtr>();
        Tp::AccountPtr account = index.data(KTp::AccountRole).value<Tp::AccountPtr>();
        startTextChannel(account, contact);
    }
}

void ContactListWidget::addOverlayButtons()
{
    Q_D(ContactListWidget);

    TextChannelContactOverlay *textOverlay  = new TextChannelContactOverlay(d->delegate);
    AudioChannelContactOverlay *audioOverlay = new AudioChannelContactOverlay(d->delegate);
    VideoChannelContactOverlay *videoOverlay = new VideoChannelContactOverlay(d->delegate);
    FileTransferContactOverlay *fileOverlay  = new FileTransferContactOverlay(d->delegate);
    DesktopSharingContactOverlay *desktopOverlay = new DesktopSharingContactOverlay(d->delegate);
    LogViewerOverlay *logViewerOverlay = new LogViewerOverlay(d->delegate);

    d->delegate->installOverlay(textOverlay);
    d->delegate->installOverlay(audioOverlay);
    d->delegate->installOverlay(videoOverlay);
    d->delegate->installOverlay(fileOverlay);
    d->delegate->installOverlay(desktopOverlay);
    d->delegate->installOverlay(logViewerOverlay);

    d->delegate->setViewOnAllOverlays(this);
    d->delegate->setAllOverlaysActive(true);

    connect(textOverlay, SIGNAL(overlayActivated(QModelIndex)),
            d->delegate, SLOT(hideStatusMessageSlot(QModelIndex)));

    connect(textOverlay, SIGNAL(overlayHidden()),
            d->delegate, SLOT(reshowStatusMessageSlot()));


    connect(textOverlay, SIGNAL(activated(Tp::AccountPtr, Tp::ContactPtr)),
            this, SLOT(startTextChannel(Tp::AccountPtr, Tp::ContactPtr)));

    connect(fileOverlay, SIGNAL(activated(Tp::AccountPtr, Tp::ContactPtr)),
            this, SLOT(startFileTransferChannel(Tp::AccountPtr, Tp::ContactPtr)));

    connect(audioOverlay, SIGNAL(activated(Tp::AccountPtr, Tp::ContactPtr)),
            this, SLOT(startAudioChannel(Tp::AccountPtr, Tp::ContactPtr)));

    connect(videoOverlay, SIGNAL(activated(Tp::AccountPtr, Tp::ContactPtr)),
            this, SLOT(startVideoChannel(Tp::AccountPtr, Tp::ContactPtr)));

    connect(desktopOverlay, SIGNAL(activated(Tp::AccountPtr, Tp::ContactPtr)),
            this, SLOT(startDesktopSharing(Tp::AccountPtr, Tp::ContactPtr)));

    connect(logViewerOverlay, SIGNAL(activated(Tp::AccountPtr,Tp::ContactPtr)),
            this, SLOT(startLogViewer(Tp::AccountPtr, Tp::ContactPtr)));

    connect(this, SIGNAL(enableOverlays(bool)),
            textOverlay, SLOT(setActive(bool)));

    connect(this, SIGNAL(enableOverlays(bool)),
            audioOverlay, SLOT(setActive(bool)));

    connect(this, SIGNAL(enableOverlays(bool)),
            videoOverlay, SLOT(setActive(bool)));

    connect(this, SIGNAL(enableOverlays(bool)),
            fileOverlay, SLOT(setActive(bool)));

    connect(this, SIGNAL(enableOverlays(bool)),
            desktopOverlay, SLOT(setActive(bool)));

    connect(this, SIGNAL(enableOverlays(bool)),
            logViewerOverlay, SLOT(setActive(bool)));
}

void ContactListWidget::toggleGroups(bool show)
{
    Q_D(ContactListWidget);

    if (show) {
        d->model->setGroupMode(KTp::ContactsModel::GroupGrouping);
    } else {
        d->model->setGroupMode(KTp::ContactsModel::AccountGrouping);
    }
    d->groupMode = d->model->groupMode();

    for (int i = 0; i < d->model->rowCount(); i++) {
        onNewGroupModelItemsInserted(d->model->index(i, 0, QModelIndex()), 0, 0);
    }
}

void ContactListWidget::toggleOfflineContacts(bool show)
{
    Q_D(ContactListWidget);

    d->showOffline = show;
    d->model->setPresenceTypeFilterFlags(show ? KTp::ContactsFilterModel::DoNotFilterByPresence : KTp::ContactsFilterModel::ShowOnlyConnected);
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

void ContactListWidget::onNewGroupModelItemsInserted(const QModelIndex& index, int start, int end)
{
    Q_UNUSED(start);
    Q_UNUSED(end);
    Q_D(ContactListWidget);

    if (!index.isValid()) {
        return;
    }

    //if there is no parent, we deal with top-level item that we want to expand/collapse, ie. group or account
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

void ContactListWidget::onSwitchToFullView()
{
    Q_D(ContactListWidget);

    setItemDelegate(d->delegate);
    doItemsLayout();

    emit enableOverlays(true);

    KSharedConfigPtr config = KGlobal::config();
    KConfigGroup guiConfigGroup(config, "GUI");
    guiConfigGroup.writeEntry("selected_delegate", "full");
    guiConfigGroup.config()->sync();
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

    d->model->setSubscriptionStateFilterFlags(KTp::ContactsFilterModel::DoNotFilterBySubscription);

    KSharedConfigPtr config = KGlobal::config();
    KConfigGroup guiConfigGroup(config, "GUI");
    guiConfigGroup.writeEntry("shown_contacts", "all");
    guiConfigGroup.config()->sync();
}

void ContactListWidget::onShowUnblockedContacts()
{
    Q_D(ContactListWidget);

    d->model->setSubscriptionStateFilterFlags(KTp::ContactsFilterModel::HideBlocked);

    KSharedConfigPtr config = KGlobal::config();
    KConfigGroup guiConfigGroup(config, "GUI");
    guiConfigGroup.writeEntry("shown_contacts", "unblocked");
    guiConfigGroup.config()->sync();
}

void ContactListWidget::onShowBlockedContacts()
{
    Q_D(ContactListWidget);

    d->model->setSubscriptionStateFilterFlags(KTp::ContactsFilterModel::ShowOnlyBlocked);

    KSharedConfigPtr config = KGlobal::config();
    KConfigGroup guiConfigGroup(config, "GUI");
    guiConfigGroup.writeEntry("shown_contacts", "blocked");
    guiConfigGroup.config()->sync();
}

void ContactListWidget::setFilterString(const QString& string)
{
    Q_D(ContactListWidget);

    if (string.isEmpty()) {
        d->model->setGroupMode(d->groupMode);
    } else {
        d->model->setGroupMode(KTp::ContactsModel::NoGrouping);
    }

    d->model->setPresenceTypeFilterFlags(string.isEmpty() && !d->showOffline ? KTp::ContactsFilterModel::ShowOnlyConnected : KTp::ContactsFilterModel::DoNotFilterByPresence);
    d->model->setGlobalFilterString(string);
}

void ContactListWidget::setDropIndicatorRect(const QRect &rect)
{
    Q_D(ContactListWidget);

    if (d->dropIndicatorRect != rect) {
        d->dropIndicatorRect = rect;
        viewport()->update();
    }
}

bool ContactListWidget::event(QEvent *event)
{
    Q_D(ContactListWidget);
    if (event->type() == QEvent::Leave && d->delegate) {
        d->delegate->reshowStatusMessageSlot();
        return true;
    }

    return QTreeView::event(event);
}

void ContactListWidget::keyPressEvent(QKeyEvent *event)
{
    //this would be normally handled by activated() signal but since we decided
    //we don't want people starting chats using single click, we can't use activated()
    //and have to do it ourselves, therefore this. Change only after discussing with the team!
    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
        //start the chat only if the index is valid and index is a valid contact or person
        if (currentIndex().isValid() &&
            (currentIndex().data(KTp::RowTypeRole).toInt() == KTp::ContactRowType
            || currentIndex().data(KTp::RowTypeRole).toInt() == KTp::PersonRowType)) {
            onContactListDoubleClicked(currentIndex());
        }
    }

    QTreeView::keyPressEvent(event);
}

void ContactListWidget::mousePressEvent(QMouseEvent *event)
{
    Q_D(ContactListWidget);

    QTreeView::mousePressEvent(event);

    QModelIndex index = indexAt(event->pos());
    d->shouldDrag = false;
    d->dragSourceGroup.clear();

    // if no contact, no drag
    if (index.data(KTp::RowTypeRole).toInt() != KTp::ContactRowType) {
        return;
    }

    if (event->button() == Qt::LeftButton) {
        d->shouldDrag = true;
        d->dragStartPosition = event->pos();
    }
}

void ContactListWidget::mouseMoveEvent(QMouseEvent *event)
{
    Q_D(ContactListWidget);

    QTreeView::mouseMoveEvent(event);

    QModelIndex index = indexAt(event->pos());

    if (!(event->buttons() & Qt::LeftButton)) {
        return;
    }

    if (!d->shouldDrag) {
        return;
    }

    if ((event->pos() - d->dragStartPosition).manhattanLength() < QApplication::startDragDistance()) {
        return;
    }

    QMimeData *mimeData = new QMimeData;
    QByteArray encodedData;
    QDataStream stream(&encodedData, QIODevice::WriteOnly);

    if (index.isValid()) {
        Tp::ContactPtr contact = index.data(KTp::ContactRole).value<KTp::ContactPtr>();
        Tp::AccountPtr account = index.data(KTp::AccountRole).value<Tp::AccountPtr>();

        if (account && contact) {
            //We put a contact ID and its account ID to the stream, so we can later recreate the contact using ContactsModel
            stream << contact->id() << account->objectPath();

            //Store source group name so that we can remove the contact from it on move-drop */
            d->dragSourceGroup = index.parent().data(KTp::IdRole).toString();
        }
    }

    mimeData->setData("application/vnd.telepathy.contact", encodedData);
    QPixmap dragIndicator = QPixmap::grabWidget(this, visualRect(index).adjusted(3,3,3,3));

    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->setPixmap(dragIndicator);

    Qt::DropActions actions;
    if (event->modifiers() & Qt::ShiftModifier) {
        actions = Qt::MoveAction;
    } else if (event->modifiers() & Qt::ControlModifier) {
        actions = Qt::CopyAction;
    } else {
        actions = Qt::MoveAction | Qt::CopyAction;
    }
    drag->exec(actions);
}

void ContactListWidget::dropEvent(QDropEvent *event)
{
    Q_D(ContactListWidget);

    QModelIndex index = indexAt(event->pos());

    if (!index.isValid()) {
        return;
    }

    if (event->mimeData()->hasUrls()) {
        kDebug() << "Filed dropped";

        Tp::ContactPtr contact = index.data(KTp::ContactRole).value<KTp::ContactPtr>();
        Tp::AccountPtr account = index.data(KTp::AccountRole).value<Tp::AccountPtr>();

        QStringList filenames;
        Q_FOREACH (const QUrl &url, event->mimeData()->urls()) {
            filenames << url.toLocalFile();
        }

        if (account && contact && !filenames.isEmpty()) {
            kDebug() << "Requesting file transfer for contact" << contact->alias();
            requestFileTransferChannels(account, contact, filenames);
            event->acceptProposedAction();
        }

    } else if (event->mimeData()->hasFormat("application/vnd.telepathy.contact")) {
        kDebug() << "Contact dropped";

        QByteArray encodedData = event->mimeData()->data("application/vnd.telepathy.contact");
        QDataStream stream(&encodedData, QIODevice::ReadOnly);
        QList<Tp::ContactPtr> contacts;

        while (!stream.atEnd()) {
            QString contactId;
            QString accountId;

            //get contact and account out of the stream
            stream >> contactId >> accountId;

            Tp::AccountPtr account = d->accountManager->accountForObjectPath(accountId);

            if (!account->connection()) {
                continue;
            }

            Q_FOREACH(const Tp::ContactPtr &contact, account->connection()->contactManager()->allKnownContacts()) {
                if (contact->id() == contactId) {
                    contacts.append(contact);
                }
            }
        }

        Qt::DropAction action = Qt::IgnoreAction;
        if ((event->possibleActions() & Qt::CopyAction) &&
            (event->possibleActions() & Qt::MoveAction)) {

            KMenu menu;
            QString seq = QKeySequence(Qt::ShiftModifier).toString();
            seq.chop(1);
            QAction *move = menu.addAction(KIcon("go-jump"), i18n("&Move here") + QLatin1Char('\t') + seq);

            seq = QKeySequence(Qt::ControlModifier).toString();
            seq.chop(1);
            QAction *copy = menu.addAction(KIcon("edit-copy"), i18n("&Copy here") + QLatin1Char('\t') + seq);

            menu.addSeparator();
            seq = QKeySequence(Qt::Key_Escape).toString();
            menu.addAction(KIcon("process-stop"), i18n("C&ancel") + QLatin1Char('\t') + seq);

            QAction *result = menu.exec(mapToGlobal(event->pos()));

            if (result == move) {
                action = Qt::MoveAction;
            } else if (result == copy) {
                action = Qt::CopyAction;
            }
        } else if (event->possibleActions() & Qt::MoveAction) {
            action = Qt::MoveAction;
        } else if (event->possibleActions() & Qt::CopyAction) {
            action = Qt::CopyAction;
        }

        Q_FOREACH(const Tp::ContactPtr &contact, contacts) {
            QString targetGroup;

            if (action == Qt::IgnoreAction) {
                continue;
            }

            if (d->model->groupMode() != KTp::ContactsModel::GroupGrouping) {
                // If contacts grouping is disabled, dropping inside the contact list makes no sense.
                continue;
            }

            if (index.data(KTp::RowTypeRole).toInt() == KTp::GroupRowType) {
                // contact is dropped on a group, so take it's name
                targetGroup = index.data(KTp::IdRole).toString();
            } else if (index.data(KTp::RowTypeRole).toInt() == KTp::ContactRowType) {
                // contact is dropped on another contact, so take it's parents (group) name
                targetGroup = index.parent().data(KTp::IdRole).toString();
            }

            if (targetGroup.isEmpty() || (targetGroup == QLatin1String("_unsorted")) ||
                contact->groups().contains(targetGroup)) {
                continue;
            }

            kDebug() << contact->alias() << "added to group" << targetGroup;

            if (action == Qt::MoveAction) {
                Tp::PendingOperation *rmOp = contact->removeFromGroup(d->dragSourceGroup);
                connect(rmOp, SIGNAL(finished(Tp::PendingOperation*)),
                        this, SIGNAL(genericOperationFinished(Tp::PendingOperation*)));
            }

            Tp::PendingOperation *addOp = contact->addToGroup(targetGroup);
            connect(addOp, SIGNAL(finished(Tp::PendingOperation*)),
                    this, SIGNAL(genericOperationFinished(Tp::PendingOperation*)));
        }
        d->dragSourceGroup.clear();

        event->acceptProposedAction();

    } else {
        event->ignore();
    }

    setDropIndicatorRect(QRect());
}

void ContactListWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        bool accepted = true;
        // check if one of the urls isn't a local file and abort if so
        Q_FOREACH (const QUrl& url, event->mimeData()->urls()) {
            if (!QFileInfo(url.toLocalFile()).isFile()) {
                    accepted = false;
            }
        }

        if (accepted) {
            event->acceptProposedAction();
        } else {
            event->ignore();
        }
    } else if (event->mimeData()->hasFormat("application/vnd.telepathy.contact")) {
        event->acceptProposedAction();
    } else {
        event->ignore();
    }
}

void ContactListWidget::dragMoveEvent(QDragMoveEvent *event)
{
    Q_D(ContactListWidget);

    QModelIndex index = indexAt(event->pos());
    setDropIndicatorRect(QRect());

    QAbstractItemView::dragMoveEvent(event);

    // urls can be dropped on a contact with file transfer capability,
    // contacts can be dropped either on a group or on another contact if GroupsModel is used
    if (event->mimeData()->hasUrls() && index.data(KTp::ContactCanFileTransferRole).toBool()) {
        event->acceptProposedAction();
        setDropIndicatorRect(visualRect(index));
    } else if (event->mimeData()->hasFormat("application/vnd.telepathy.contact") &&
               (index.data(KTp::RowTypeRole).toInt() == KTp::GroupRowType ||
                index.data(KTp::RowTypeRole).toInt() == KTp::ContactRowType)
               && d->model->groupMode() == KTp::ContactsModel::GroupGrouping) {
        // Contacts dropping is only allowed if groups are enabled in appearance settings.
        event->acceptProposedAction();
        setDropIndicatorRect(visualRect(index));
    } else {
        event->ignore();
    }
}

void ContactListWidget::dragLeaveEvent(QDragLeaveEvent *event)
{
    Q_UNUSED(event);
    setDropIndicatorRect(QRect());
}

void ContactListWidget::paintEvent(QPaintEvent *event)
{
    Q_D(ContactListWidget);

    QTreeView::paintEvent(event);
    if (!d->dropIndicatorRect.isNull()) {
        QStyleOption option;
        option.init(this);
        option.rect = d->dropIndicatorRect.adjusted(0,0,-1,-1);
        QPainter painter(viewport());
        style()->drawPrimitive(QStyle::PE_IndicatorItemViewItemDrop, &option, &painter, this);
    }
}

void ContactListWidget::drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const
{
    Q_UNUSED(painter);
    Q_UNUSED(rect);
    Q_UNUSED(index);

    // There is a 0px identation set in the constructor, with setIndentation(0).
    // Because of that, no branches are shown, so they should be disabled completely (overriding drawBranches).
    // Leaving branches enabled with 0px identation results in a 1px branch line on the left of all items,
    // which looks like an artifact.
    //See https://bugreports.qt-project.org/browse/QTBUG-26305
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
