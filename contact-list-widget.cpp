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

#include <KTp/Models/accounts-model.h>
#include <KTp/Models/groups-model.h>
#include <KTp/Models/accounts-filter-model.h>
#include <KTp/Models/contact-model-item.h>
#include <KTp/Models/accounts-model-item.h>
#include <KTp/Models/groups-model-item.h>

#include <KGlobal>
#include <KSharedConfig>
#include <KConfigGroup>
#include <KDebug>
#include <KMessageBox>
#include <KLocalizedString>
#include <KDialog>
#include <KFileDialog>
#include <KSettings/Dialog>

#include <QHeaderView>
#include <QLabel>
#include <QApplication>
#include <QDropEvent>
#include <QDragMoveEvent>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QPainter>
#include <QPixmap>

#include "contact-delegate.h"
#include "contact-delegate-compact.h"
#include "contact-overlays.h"

#define PREFERRED_TEXTCHAT_HANDLER "org.freedesktop.Telepathy.Client.KTp.TextUi"
#define PREFERRED_FILETRANSFER_HANDLER "org.freedesktop.Telepathy.Client.KTp.FileTransfer"
#define PREFERRED_AUDIO_VIDEO_HANDLER "org.freedesktop.Telepathy.Client.KTp.CallUi"
#define PREFERRED_RFB_HANDLER "org.freedesktop.Telepathy.Client.krfb_rfb_handler"

ContactListWidget::ContactListWidget(QWidget *parent)
    : QTreeView(parent),
      d_ptr(new ContactListWidgetPrivate)
{
    Q_D(ContactListWidget);

    KSharedConfigPtr config = KGlobal::config();
    KConfigGroup guiConfigGroup(config, "GUI");

    d->delegate = new ContactDelegate(this);
    d->compactDelegate = new ContactDelegateCompact(this);

    d->model = new AccountsModel(this);
    d->groupsModel = new GroupsModel(d->model, this);
    d->modelFilter = new AccountsFilterModel(this);
    d->modelFilter->setDynamicSortFilter(true);
    d->modelFilter->setSortRole(Qt::DisplayRole);

    setModel(d->modelFilter);
    setSortingEnabled(true);
    sortByColumn(0, Qt::AscendingOrder);

    connect(d->modelFilter, SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SLOT(onNewGroupModelItemsInserted(QModelIndex,int,int)));

    connect(d->groupsModel, SIGNAL(operationFinished(Tp::PendingOperation*)),
            this, SIGNAL(genericOperationFinished(Tp::PendingOperation*)));

    header()->hide();
    setRootIsDecorated(false);
    setSortingEnabled(true);
    setContextMenuPolicy(Qt::CustomContextMenu);
    setIndentation(0);
    setMouseTracking(true);
    setExpandsOnDoubleClick(false); //the expanding/collapsing is handled manually
    setDragEnabled(true);
    viewport()->setAcceptDrops(true);
    setDropIndicatorShown(true);

    if (guiConfigGroup.readEntry("selected_delegate", "compact") == QLatin1String("compact")) {
        setItemDelegate(d->compactDelegate);
    } else {
        setItemDelegate(d->delegate);
    }

    addOverlayButtons();
    emit enableOverlays(guiConfigGroup.readEntry("selected_delegate", "compact") == QLatin1String("full"));

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
    d->model->setAccountManager(accountManager);

    connect(accountManager.data(), SIGNAL(newAccount(Tp::AccountPtr)),
                this, SLOT(onNewAccountAdded(Tp::AccountPtr)));



    QList<Tp::AccountPtr> accounts = accountManager->allAccounts();

    if(accounts.count() == 0) {
        if (KMessageBox::questionYesNo(this,
                                       i18n("You have no IM accounts configured. Would you like to do that now?"),
                                       i18n("No Accounts Found")) == KMessageBox::Yes) {

            showSettingsKCM();
        }
    }

    foreach (const Tp::AccountPtr &account, accounts) {
        onNewAccountAdded(account);
    }

    expandAll();

}

AccountsModel* ContactListWidget::accountsModel()
{
    Q_D(ContactListWidget);

    return d->model;
}

void ContactListWidget::showSettingsKCM()
{
    KSettings::Dialog *dialog = new KSettings::Dialog(this);

    KService::Ptr tpAccKcm = KService::serviceByDesktopName("kcm_ktp_accounts");

    if (!tpAccKcm) {
        KMessageBox::error(this,
                           i18n("It appears you do not have the IM Accounts control module installed. Please install telepathy-accounts-kcm package."),
                           i18n("IM Accounts KCM Plugin Is Not Installed"));
    }

    dialog->addModule("kcm_ktp_accounts");
    dialog->addModule("kcm_ktp_integration_module");

    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->exec();
}

void ContactListWidget::onAccountConnectionStatusChanged(Tp::ConnectionStatus status)
{
    Q_D(ContactListWidget);

    kDebug() << "Connection status is" << status;

    Tp::AccountPtr account(qobject_cast< Tp::Account* >(sender()));
    QModelIndex index = d->model->index(qobject_cast<AccountsModelItem*>(d->model->accountItemForId(account->uniqueIdentifier())));

    switch (status) {
        case Tp::ConnectionStatusConnected:
            setExpanded(index, true);
            break;
        case Tp::ConnectionStatusConnecting:
            setExpanded(index, false);
        default:
            break;
    }
}

void ContactListWidget::onNewAccountAdded(const Tp::AccountPtr& account)
{
    Q_ASSERT(account->isReady(Tp::Account::FeatureCore));

    connect(account.data(),
            SIGNAL(connectionStatusChanged(Tp::ConnectionStatus)),
            this, SLOT(onAccountConnectionStatusChanged(Tp::ConnectionStatus)));

    //FIXME get rid of that thing already
//     m_avatarButton->loadAvatar(account);
//     KSharedConfigPtr config = KGlobal::config();
//     KConfigGroup avatarGroup(config, "Avatar");
//     if (avatarGroup.readEntry("method", QString()) == QLatin1String("account")) {
//         //this also updates the avatar if it was changed somewhere else
//         m_avatarButton->selectAvatarFromAccount(avatarGroup.readEntry("source", QString()));
//     }
}

void ContactListWidget::onContactListClicked(const QModelIndex& index)
{
    if (!index.isValid()) {
        return;
    }

    if (index.data(AccountsModel::ItemRole).userType() == qMetaTypeId<AccountsModelItem*>()
        || index.data(AccountsModel::ItemRole).userType() == qMetaTypeId<GroupsModelItem*>()) {

        KSharedConfigPtr config = KSharedConfig::openConfig(QLatin1String("ktelepathyrc"));
        KConfigGroup groupsConfig = config->group("GroupsState");

        if (isExpanded(index)) {
            collapse(index);
            groupsConfig.writeEntry(index.data(AccountsModel::IdRole).toString(), false);
        } else {
            expand(index);
            groupsConfig.writeEntry(index.data(AccountsModel::IdRole).toString(), true);
        }

        groupsConfig.config()->sync();
    }
}

void ContactListWidget::onContactListDoubleClicked(const QModelIndex& index)
{
    if (!index.isValid()) {
        return;
    }

    if (index.data(AccountsModel::ItemRole).userType() == qMetaTypeId<ContactModelItem*>()) {
        kDebug() << "Text chat requested for index" << index;
        startTextChannel(index.data(AccountsModel::ItemRole).value<ContactModelItem*>());
    }
}

void ContactListWidget::addOverlayButtons()
{
    Q_D(ContactListWidget);

    TextChannelContactOverlay *textOverlay  = new TextChannelContactOverlay(this);
    AudioChannelContactOverlay *audioOverlay = new AudioChannelContactOverlay(this);
    VideoChannelContactOverlay *videoOverlay = new VideoChannelContactOverlay(this);

    FileTransferContactOverlay *fileOverlay  = new FileTransferContactOverlay(this);
    DesktopSharingContactOverlay *desktopOverlay = new DesktopSharingContactOverlay(this);

    d->delegate->installOverlay(textOverlay);
    d->delegate->installOverlay(audioOverlay);
    d->delegate->installOverlay(videoOverlay);
    d->delegate->installOverlay(fileOverlay);
    d->delegate->installOverlay(desktopOverlay);

    d->delegate->setViewOnAllOverlays(this);
    d->delegate->setAllOverlaysActive(true);

    connect(textOverlay, SIGNAL(overlayActivated(QModelIndex)),
            d->delegate, SLOT(hideStatusMessageSlot(QModelIndex)));

    connect(textOverlay, SIGNAL(overlayHidden()),
            d->delegate, SLOT(reshowStatusMessageSlot()));


    connect(textOverlay, SIGNAL(activated(ContactModelItem*)),
            this, SLOT(startTextChannel(ContactModelItem*)));

    connect(fileOverlay, SIGNAL(activated(ContactModelItem*)),
            this, SLOT(startFileTransferChannel(ContactModelItem*)));

    connect(audioOverlay, SIGNAL(activated(ContactModelItem*)),
            this, SLOT(startAudioChannel(ContactModelItem*)));

    connect(videoOverlay, SIGNAL(activated(ContactModelItem*)),
            this, SLOT(startVideoChannel(ContactModelItem*)));

    connect(desktopOverlay, SIGNAL(activated(ContactModelItem*)),
            this, SLOT(startDesktopSharing(ContactModelItem*)));


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
}

void ContactListWidget::toggleGroups(bool show)
{
    Q_D(ContactListWidget);

    if (show) {
        d->modelFilter->setSourceModel(d->groupsModel);
    } else {
        d->modelFilter->setSourceModel(d->model);
    }
}

void ContactListWidget::toggleOfflineContacts(bool show)
{
    Q_D(ContactListWidget);

    d->showOffline = show;
    d->modelFilter->setPresenceTypeFilterFlags(show ? AccountsFilterModel::DoNotFilterByPresence : AccountsFilterModel::ShowOnlyConnected);
}

void ContactListWidget::toggleSortByPresence(bool sort)
{
    Q_D(ContactListWidget);

    d->modelFilter->setSortMode(sort ? AccountsFilterModel::SortByPresence : AccountsFilterModel::DoNotSort);
}

void ContactListWidget::startTextChannel(ContactModelItem *contactItem)
{
    Q_D(ContactListWidget);

    Q_ASSERT(contactItem);
    Tp::ContactPtr contact = contactItem->contact();

    kDebug() << "Requesting chat for contact" << contact->alias();

    Tp::AccountPtr account = d->model->accountForContactItem(contactItem);

    Tp::ChannelRequestHints hints;
    hints.setHint("org.freedesktop.Telepathy.ChannelRequest","DelegateToPreferredHandler", QVariant(true));

    Tp::PendingChannelRequest *channelRequest = account->ensureTextChat(contact,
                                                                        QDateTime::currentDateTime(),
                                                                        PREFERRED_TEXTCHAT_HANDLER,
                                                                        hints);
    connect(channelRequest, SIGNAL(finished(Tp::PendingOperation*)),
            SIGNAL(genericOperationFinished(Tp::PendingOperation*)));
}

void ContactListWidget::startAudioChannel(ContactModelItem *contactItem)
{
    Q_D(ContactListWidget);

    Q_ASSERT(contactItem);
    Tp::ContactPtr contact = contactItem->contact();

    kDebug() << "Requesting audio for contact" << contact->alias();

    Tp::AccountPtr account = d->model->accountForContactItem(contactItem);

    Tp::PendingChannelRequest *channelRequest = account->ensureAudioCall(contact,
            QLatin1String("audio"), QDateTime::currentDateTime(), PREFERRED_AUDIO_VIDEO_HANDLER);

    connect(channelRequest, SIGNAL(finished(Tp::PendingOperation*)),
            SIGNAL(genericOperationFinished(Tp::PendingOperation*)));
}

void ContactListWidget::startVideoChannel(ContactModelItem *contactItem)
{
    Q_D(ContactListWidget);

    Q_ASSERT(contactItem);
    Tp::ContactPtr contact = contactItem->contact();

    kDebug() << "Requesting video for contact" << contact->alias();

    Tp::AccountPtr account = d->model->accountForContactItem(contactItem);

    Tp::PendingChannelRequest* channelRequest = account->ensureAudioVideoCall(contact,
            QLatin1String("audio"), QLatin1String("video"),
            QDateTime::currentDateTime(), PREFERRED_AUDIO_VIDEO_HANDLER);

    connect(channelRequest, SIGNAL(finished(Tp::PendingOperation*)),
            SIGNAL(genericOperationFinished(Tp::PendingOperation*)));
}

void ContactListWidget::startDesktopSharing(ContactModelItem* contactItem)
{
    Q_D(ContactListWidget);

    Q_ASSERT(contactItem);
    Tp::ContactPtr contact = contactItem->contact();

    kDebug() << "Requesting desktop sharing for contact" << contact->alias();

    Tp::AccountPtr account = d->model->accountForContactItem(contactItem);

    Tp::PendingChannelRequest* channelRequest = account->createStreamTube(contact,
                                                                          QLatin1String("rfb"),
                                                                          QDateTime::currentDateTime(),
                                                                          PREFERRED_RFB_HANDLER);

    connect(channelRequest, SIGNAL(finished(Tp::PendingOperation*)),
            SIGNAL(genericOperationFinished(Tp::PendingOperation*)));
}

void ContactListWidget::startFileTransferChannel(ContactModelItem *contactItem)
{
    Q_D(ContactListWidget);

    Q_ASSERT(contactItem);
    Tp::ContactPtr contact = contactItem->contact();

    kDebug() << "Requesting file transfer for contact" << contact->alias();

    Tp::AccountPtr account = d->model->accountForContactItem(contactItem);

    QStringList filenames = KFileDialog::getOpenFileNames(KUrl("kfiledialog:///FileTransferLastDirectory"),
                                                          QString(),
                                                          this,
                                                          i18n("Choose files to send to %1", contact->alias()));

    if (filenames.isEmpty()) { // User hit cancel button
        return;
    }

    QDateTime now = QDateTime::currentDateTime();

    requestFileTransferChannels(account, contact, filenames, now);
}

void ContactListWidget::requestFileTransferChannels(const Tp::AccountPtr& account,
                                                    const Tp::ContactPtr& contact,
                                                    const QStringList& filenames,
                                                    const QDateTime& userActionTime)
{
    Q_FOREACH (const QString &filename, filenames) {
        kDebug() << "Filename:" << filename;
        kDebug() << "Content type:" << KMimeType::findByFileContent(filename)->name();

        Tp::FileTransferChannelCreationProperties fileTransferProperties(filename,
                                                                        KMimeType::findByFileContent(filename)->name());

        Tp::PendingChannelRequest* channelRequest = account->createFileTransfer(contact,
                                                                                fileTransferProperties,
                                                                                userActionTime,
                                                                                PREFERRED_FILETRANSFER_HANDLER);

        connect(channelRequest, SIGNAL(finished(Tp::PendingOperation*)),
                SIGNAL(genericOperationFinished(Tp::PendingOperation*)));
    }

}

void ContactListWidget::onNewGroupModelItemsInserted(const QModelIndex& index, int start, int end)
{
    Q_UNUSED(start);
    Q_UNUSED(end);
    if (!index.isValid()) {
        return;
    }

    //if there is no parent, we deal with top-level item that we want to expand/collapse, ie. group or account
    if (!index.parent().isValid()) {
        KSharedConfigPtr config = KSharedConfig::openConfig(QLatin1String("ktelepathyrc"));
        KConfigGroup groupsConfig = config->group("GroupsState");

        //we're probably dealing with group item, so let's check if it is expanded first
        if (!isExpanded(index)) {
            //if it's not expanded, check the config if we should expand it or not
            if (groupsConfig.readEntry(index.data(AccountsModel::IdRole).toString(), false)) {
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
    doItemsLayout();

    emit enableOverlays(false);

    KSharedConfigPtr config = KGlobal::config();
    KConfigGroup guiConfigGroup(config, "GUI");
    guiConfigGroup.writeEntry("selected_delegate", "compact");
    guiConfigGroup.config()->sync();
}

void ContactListWidget::onShowAllContacts()
{
    Q_D(ContactListWidget);

    d->modelFilter->setSubscriptionStateFilterFlags(AccountsFilterModel::DoNotFilterBySubscription);

    KSharedConfigPtr config = KGlobal::config();
    KConfigGroup guiConfigGroup(config, "GUI");
    guiConfigGroup.writeEntry("shown_contacts", "all");
    guiConfigGroup.config()->sync();
}

void ContactListWidget::onShowUnblockedContacts()
{
    Q_D(ContactListWidget);

    d->modelFilter->setSubscriptionStateFilterFlags(AccountsFilterModel::HideBlocked);

    KSharedConfigPtr config = KGlobal::config();
    KConfigGroup guiConfigGroup(config, "GUI");
    guiConfigGroup.writeEntry("shown_contacts", "unblocked");
    guiConfigGroup.config()->sync();
}

void ContactListWidget::onShowBlockedContacts()
{
    Q_D(ContactListWidget);

    d->modelFilter->setSubscriptionStateFilterFlags(AccountsFilterModel::ShowOnlyBlocked);

    KSharedConfigPtr config = KGlobal::config();
    KConfigGroup guiConfigGroup(config, "GUI");
    guiConfigGroup.writeEntry("shown_contacts", "blocked");
    guiConfigGroup.config()->sync();
}

void ContactListWidget::setFilterString(const QString& string)
{
    Q_D(ContactListWidget);

    d->modelFilter->setPresenceTypeFilterFlags(string.isEmpty() && !d->showOffline ? AccountsFilterModel::ShowOnlyConnected : AccountsFilterModel::DoNotFilterByPresence);
    d->modelFilter->setDisplayNameFilterString(string);
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

void ContactListWidget::mousePressEvent(QMouseEvent *event)
{
    Q_D(ContactListWidget);

    QTreeView::mousePressEvent(event);

    QModelIndex index = indexAt(event->pos());
    d->shouldDrag = false;

    // if no contact, no drag
    if (!index.data(AccountsModel::ItemRole).canConvert<ContactModelItem*>()) {
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
        ContactModelItem *contactItem = index.data(AccountsModel::ItemRole).value<ContactModelItem*>();
        //We put a contact ID and its account ID to the stream, so we can later recreate the contact using AccountsModel
        stream << contactItem->contact().data()->id() << d->model->accountForContactItem(contactItem).data()->objectPath();
    }

    mimeData->setData("application/vnd.telepathy.contact", encodedData);
    QPixmap dragIndicator = QPixmap::grabWidget(this, visualRect(index).adjusted(3,3,3,3));

    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->setPixmap(dragIndicator);

    drag->exec(Qt::CopyAction);
}

void ContactListWidget::dropEvent(QDropEvent *event)
{
    Q_D(ContactListWidget);

    QModelIndex index = indexAt(event->pos());

    if (event->mimeData()->hasUrls()) {
        kDebug() << "It's a file!";

        ContactModelItem* contactItem = index.data(AccountsModel::ItemRole).value<ContactModelItem*>();
        Q_ASSERT(contactItem);

        Tp::ContactPtr contact = contactItem->contact();

        kDebug() << "Requesting file transfer for contact" << contact->alias();

        Tp::AccountPtr account = d->model->accountForContactItem(contactItem);

        QStringList filenames;
        Q_FOREACH (const QUrl &url, event->mimeData()->urls()) {
            filenames << url.toLocalFile();
        }

        if (filenames.isEmpty()) {
            return;
        }

        QDateTime now = QDateTime::currentDateTime();
        requestFileTransferChannels(account, contact, filenames, now);

        event->acceptProposedAction();
    } else if (event->mimeData()->hasFormat("application/vnd.telepathy.contact")) {
        kDebug() << "It's a contact!";

        QByteArray encodedData = event->mimeData()->data("application/vnd.telepathy.contact");
        QDataStream stream(&encodedData, QIODevice::ReadOnly);
        QList<ContactModelItem*> contacts;

        while (!stream.atEnd()) {
            QString contact;
            QString account;

            //get contact and account out of the stream
            stream >> contact >> account;

            Tp::AccountPtr accountPtr = d->model->accountPtrForPath(account);

            //casted pointer is checked below, before first use
            contacts.append(qobject_cast<ContactModelItem*>(d->model->contactItemForId(accountPtr->uniqueIdentifier(), contact)));
        }

        Q_FOREACH (ContactModelItem *contact, contacts) {
            Q_ASSERT(contact);
            QString group;
            if (index.data(AccountsModel::ItemRole).canConvert<GroupsModelItem*>()) {
                // contact is dropped on a group, so take it's name
                group = index.data(GroupsModel::GroupNameRole).toString();
            } else {
                // contact is dropped on another contact, so take it's parents (group) name
                group = index.parent().data(GroupsModel::GroupNameRole).toString();
            }

            kDebug() << contact->contact().data()->alias() << "added to group" << group;

            if (!group.isEmpty()) {
                Tp::PendingOperation *op = contact->contact().data()->addToGroup(group);

                connect(op, SIGNAL(finished(Tp::PendingOperation*)),
                        this, SIGNAL(genericOperationFinished(Tp::PendingOperation*)));
            }
        }
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

    // urls can be dropped on a contact with file transfer capability,
    // contacts can be dropped either on a group or on another contact if GroupsModel is used
    if (event->mimeData()->hasUrls() && index.data(AccountsModel::FileTransferCapabilityRole).toBool()) {
        event->acceptProposedAction();
        setDropIndicatorRect(visualRect(index));
    } else if (event->mimeData()->hasFormat("application/vnd.telepathy.contact") &&
               d->modelFilter->sourceModel() == d->groupsModel &&
               (index.data(AccountsModel::ItemRole).canConvert<GroupsModelItem*>() ||
                index.data(AccountsModel::ItemRole).canConvert<ContactModelItem*>())) {
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
