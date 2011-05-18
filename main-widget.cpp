/*
 * This file is part of telepathy-contactslist-prototype
 *
 * Copyright (C) 2009-2010 Collabora Ltd. <info@collabora.co.uk>
 *   @Author George Goldberg <george.goldberg@collabora.co.uk>
 *   @Author Martin Klapetek <martin.klapetek@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "main-widget.h"

#include <QtGui/QSortFilterProxyModel>
#include <QtGui/QPainter>
#include <QtGui/QMenu>
#include <QtGui/QLabel>
#include <QtGui/QCheckBox>
#include <QtGui/QPushButton>
#include <QtGui/QToolButton>
#include <QtCore/QWeakPointer>
#include <QWidgetAction>

#include <TelepathyQt4/PendingReady>
#include <TelepathyQt4/PendingChannelRequest>
#include <TelepathyQt4/PendingContacts>
#include <TelepathyQt4/ClientRegistrar>
#include <TelepathyQt4/Constants>
#include <TelepathyQt4/ContactManager>

#include <KDebug>
#include <KDialog>
#include <KIO/Job>
#include <KUser>
#include <KMenu>
#include <KMessageBox>
#include <KSettings/Dialog>
#include <KSharedConfig>
#include <KFileDialog>
#include <KStandardShortcut>
#include <KNotification>

#include "ui_main-widget.h"
#include "account-button.h"
#include "contact-overlays.h"
#include "accounts-model.h"
#include "account-filter-model.h"
#include "contact-delegate.h"
#include "contact-model-item.h"
#include "add-contact-dialog.h"
#include "remove-contact-dialog.h"
#include "fetch-avatar-job.h"

#define PREFERRED_TEXTCHAT_HANDLER "org.freedesktop.Telepathy.Client.KDE.TextUi"
#define PREFERRED_FILETRANSFER_HANDLER "org.freedesktop.Telepathy.Client.KDE.FileTransfer"
#define PREFERRED_AUDIO_VIDEO_HANDLER "org.freedesktop.Telepathy.Client.KDE.CallUi"

bool kde_tp_filter_contacts_by_publication_status(const Tp::ContactPtr &contact)
{
    return contact->publishState() == Tp::Contact::PresenceStateAsk;
}

MainWidget::MainWidget(QWidget *parent)
    : KMainWindow(parent),
   m_model(0),
   m_modelFilter(0)
{
    Tp::registerTypes();
    KUser user;

    setupUi(this);
    m_filterBar->hide();
    setWindowIcon(KIcon("telepathy"));

    m_userAccountNameLabel->setText(user.property(KUser::FullName).isNull() ?
        user.loginName() : user.property(KUser::FullName).toString()
    );

    m_userAccountIconButton->setPopupMode(QToolButton::InstantPopup);

    m_avatarButtonMenu = new KMenu(m_userAccountIconButton);

    QToolButton *loadFromFileButton = new QToolButton(this);
    loadFromFileButton->setIcon(KIcon("document-open-folder"));
    loadFromFileButton->setIconSize(QSize(48, 48));
    loadFromFileButton->setText(i18n("Load from file..."));
    loadFromFileButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    QWidgetAction *loadFromFileAction = new QWidgetAction(this);
    loadFromFileAction->setDefaultWidget(loadFromFileButton);

    connect(loadFromFileButton, SIGNAL(clicked(bool)),
            loadFromFileAction, SIGNAL(triggered(bool)));

    connect(loadFromFileAction, SIGNAL(triggered(bool)),
            this, SLOT(loadAvatarFromFile()));

    m_avatarButtonMenu->addAction(loadFromFileAction);

    m_userAccountIconButton->setMenu(m_avatarButtonMenu);

    m_addContactAction = new KAction(KIcon("list-add-user"), QString(), this);
    m_addContactAction->setToolTip(i18n("Add new contacts.."));

    m_toolBar->addAction(m_addContactAction);

    m_groupContactsAction = new KAction(KIcon("user-group-properties"), QString(), this);
    m_groupContactsAction->setCheckable(true);
    m_groupContactsAction->setChecked(true);
    //TODO: Toggle the tooltip with the button? eg. once its Show, after click its Hide .. ?
    m_groupContactsAction->setToolTip(i18n("Show/Hide groups"));

    m_toolBar->addAction(m_groupContactsAction);

    m_hideOfflineAction = new KAction(KIcon("meeting-attending-tentative"), QString(), this);
    m_hideOfflineAction->setCheckable(true);
    m_hideOfflineAction->setChecked(true);
    m_hideOfflineAction->setToolTip(i18n("Show/Hide offline users"));

    m_toolBar->addAction(m_hideOfflineAction);

    m_sortByPresenceAction = new KAction(KIcon("view-sort-ascending"), QString(), this);
    m_sortByPresenceAction->setCheckable(true);
    m_sortByPresenceAction->setChecked(false);
    m_sortByPresenceAction->setToolTip(i18n("Sort by presence"));

    m_toolBar->addAction(m_sortByPresenceAction);

    m_searchContactAction = new KAction(KIcon("edit-find-user"), QString(), this );
    m_searchContactAction->setShortcut(KStandardShortcut::find());
    m_searchContactAction->setCheckable(true);
    m_searchContactAction->setChecked(false);
    m_searchContactAction->setToolTip(i18n("Find contact"));

    m_toolBar->addAction(m_searchContactAction);

    QToolButton *settingsButton = new QToolButton(this);
    settingsButton->setIcon(KIcon("configure"));
    settingsButton->setPopupMode(QToolButton::InstantPopup);

    KMenu *settingsButtonMenu = new KMenu(settingsButton);
    settingsButtonMenu->addAction(i18n("Configure accounts..."), this, SLOT(showSettingsKCM()));
    settingsButtonMenu->addSeparator();
    settingsButtonMenu->addMenu(helpMenu());

    settingsButton->setMenu(settingsButtonMenu);

    m_toolBar->addSeparator();
    m_toolBar->addWidget(settingsButton);

    // Start setting up the Telepathy AccountManager.
    Tp::AccountFactoryPtr  accountFactory = Tp::AccountFactory::create(QDBusConnection::sessionBus(),
                                                                       Tp::Features() << Tp::Account::FeatureCore
                                                                       << Tp::Account::FeatureAvatar
                                                                       << Tp::Account::FeatureProtocolInfo
                                                                       << Tp::Account::FeatureProfile);

    Tp::ConnectionFactoryPtr connectionFactory = Tp::ConnectionFactory::create(QDBusConnection::sessionBus(),
                                                                               Tp::Features() << Tp::Connection::FeatureCore
                                                                               << Tp::Connection::FeatureRosterGroups
                                                                               << Tp::Connection::FeatureRoster
                                                                               << Tp::Connection::FeatureSelfContact);

    Tp::ContactFactoryPtr contactFactory = Tp::ContactFactory::create(Tp::Features()  << Tp::Contact::FeatureAlias
                                                                      << Tp::Contact::FeatureAvatarData
                                                                      << Tp::Contact::FeatureSimplePresence
                                                                      << Tp::Contact::FeatureCapabilities);

    Tp::ChannelFactoryPtr channelFactory = Tp::ChannelFactory::create(QDBusConnection::sessionBus());

    m_accountManager = Tp::AccountManager::create(QDBusConnection::sessionBus(),
                                                  accountFactory,
                                                  connectionFactory,
                                                  channelFactory,
                                                  contactFactory);

    connect(m_accountManager->becomeReady(),
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onAccountManagerReady(Tp::PendingOperation*)));

    connect(m_accountManager.data(), SIGNAL(newAccount(Tp::AccountPtr)),
            this, SLOT(onNewAccountAdded(Tp::AccountPtr)));

    m_delegate = new ContactDelegate(this);

    m_contactsListView->header()->hide();
    m_contactsListView->setRootIsDecorated(false);
    m_contactsListView->setSortingEnabled(true);
    m_contactsListView->setContextMenuPolicy(Qt::CustomContextMenu);
    m_contactsListView->setItemDelegate(m_delegate);
    m_contactsListView->setIndentation(0);
    m_contactsListView->setMouseTracking(true);
    m_contactsListView->setExpandsOnDoubleClick(false); //the expanding/collapsing is handled manually

    addOverlayButtons();

    connect(m_contactsListView, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(onCustomContextMenuRequested(QPoint)));

    connect(m_contactsListView, SIGNAL(doubleClicked(QModelIndex)),
            this, SLOT(onContactListDoubleClick(QModelIndex)));

    connect(m_delegate, SIGNAL(repaintItem(QModelIndex)),
            m_contactsListView->viewport(), SLOT(repaint())); //update(QModelIndex)

    connect(m_addContactAction, SIGNAL(triggered(bool)),
            this, SLOT(onAddContactRequest()));

    connect(m_groupContactsAction, SIGNAL(triggered(bool)),
            this, SLOT(onGroupContacts(bool)));

    connect(m_searchContactAction, SIGNAL(triggered(bool)),
            this, SLOT(toggleSearchWidget(bool)));

    connect(m_presenceMessageEdit, SIGNAL(returnPressed(QString)),
            this, SLOT(setCustomPresenceMessage(QString)));

    KSharedConfigPtr config = KGlobal::config();
    KConfigGroup configGroup(config, "GUI");
    if (configGroup.readEntry("pin_filterbar", true)) {
        toggleSearchWidget(true);
        m_searchContactAction->setChecked(true);
    }
}

MainWidget::~MainWidget()
{
    //save the state of the filter bar, pinned or not
    KSharedConfigPtr config = KGlobal::config();
    KConfigGroup configGroup(config, "GUI");
    configGroup.writeEntry("pin_filterbar", m_searchContactAction->isChecked());
    configGroup.config()->sync();
}

void MainWidget::onAccountManagerReady(Tp::PendingOperation* op)
{
    if (op->isError()) {
        kDebug() << op->errorName();
        kDebug() << op->errorMessage();
    }

    m_model = new AccountsModel(m_accountManager, this);
    m_modelFilter = new AccountFilterModel(this);
    m_modelFilter->setSourceModel(m_model);
    m_modelFilter->setDynamicSortFilter(true);
    m_modelFilter->filterOfflineUsers(m_hideOfflineAction->isChecked());
    m_modelFilter->clearFilterString();
    m_modelFilter->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_modelFilter->setSortRole(Qt::DisplayRole);
    m_modelFilter->setSortByPresence(m_sortByPresenceAction->isChecked());
    m_contactsListView->setModel(m_modelFilter);
    m_contactsListView->setSortingEnabled(true);
    m_contactsListView->sortByColumn(0, Qt::AscendingOrder);

    connect(m_hideOfflineAction, SIGNAL(toggled(bool)),
            m_modelFilter, SLOT(filterOfflineUsers(bool)));

    connect(m_filterBar, SIGNAL(filterChanged(QString)),
            m_modelFilter, SLOT(setFilterString(QString)));

    connect(m_filterBar, SIGNAL(closeRequest()),
            m_modelFilter, SLOT(clearFilterString()));

    connect(m_filterBar, SIGNAL(closeRequest()),
            m_filterBar, SLOT(hide()));

    connect(m_filterBar, SIGNAL(closeRequest()),
            m_searchContactAction, SLOT(toggle()));

    connect(m_sortByPresenceAction, SIGNAL(toggled(bool)),
            m_modelFilter, SLOT(setSortByPresence(bool)));

    connect(m_modelFilter, SIGNAL(rowsAboutToBeRemoved(QModelIndex, int, int)),
        m_delegate, SLOT(contactRemoved(QModelIndex,int,int)));

    m_accountButtonsLayout->insertStretch(-1);

    QList<Tp::AccountPtr> accounts = m_accountManager->allAccounts();

    if(accounts.count() == 0) {
        KDialog *dialog = new KDialog(this);
        dialog->setCaption(i18n("No Accounts Found"));
        dialog->setButtons(KDialog::Ok | KDialog::Cancel);
        dialog->setMainWidget(new QLabel(i18n("No Accounts Found")));
        dialog->setButtonText(KDialog::Ok, i18n("Configure Accounts"));
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        dialog->setInitialSize(dialog->sizeHint());
        connect(dialog, SIGNAL(okClicked()), this, SLOT(showSettingsKCM()));
        connect(dialog, SIGNAL(okClicked()), dialog, SLOT(close()));
        connect(dialog, SIGNAL(cancelClicked()), dialog, SLOT(close()));
        dialog->show();
    }

    foreach (const Tp::AccountPtr account, accounts) {
        onNewAccountAdded(account);
    }
    m_contactsListView->expandAll();
}

void MainWidget::onAccountConnectionStatusChanged(Tp::ConnectionStatus status)
{
    kDebug() << "Connection status is" << status;

    Tp::AccountPtr account(qobject_cast< Tp::Account* >(sender()));
    QModelIndex index = m_model->index(qobject_cast<AccountsModelItem*>(m_model->accountItemForId(account->uniqueIdentifier())));

    switch (status) {
    case Tp::ConnectionStatusConnected:
        m_contactsListView->setExpanded(index, true);
        break;
    case Tp::ConnectionStatusDisconnected:
        handleConnectionError(account);
        break;
    case Tp::ConnectionStatusConnecting:
        m_contactsListView->setExpanded(index, false);
    default:
        break;
    }
}

void MainWidget::onNewAccountAdded(const Tp::AccountPtr& account)
{
    Tp::PendingReady *ready = account->becomeReady();

    connect(ready,
            SIGNAL(finished(Tp::PendingOperation*)),
            this, SLOT(onAccountReady(Tp::PendingOperation*)));

    connect(account.data(),
            SIGNAL(connectionChanged(Tp::ConnectionPtr)),
            this, SLOT(onConnectionChanged(Tp::ConnectionPtr)));

    connect(account.data(),
            SIGNAL(connectionStatusChanged(Tp::ConnectionStatus)),
            this, SLOT(onAccountConnectionStatusChanged(Tp::ConnectionStatus)));

    connect(account.data(), SIGNAL(stateChanged(bool)),
            this, SLOT(onAccountStateChanged(bool)));

    connect(account.data(),
            SIGNAL(removed()),
            this, SLOT(onAccountRemoved()));

    AccountButton *bt = new AccountButton(account, this);
    bt->setObjectName(account->uniqueIdentifier());
    bt->hide();

    m_accountButtonsLayout->insertWidget(m_accountButtonsLayout->count() - 1, bt);

    if(account->isEnabled()) {
        bt->show();
        loadAvatar(account);
    }

    KSharedConfigPtr config = KGlobal::config();
    KConfigGroup avatarGroup(config, "Avatar");
    if (avatarGroup.readEntry("method", QString()) == QLatin1String("account")) {
        //this also updates the avatar if it was changed somewhere else
        selectAvatarFromAccount(avatarGroup.readEntry("source", QString()));
    }
}

void MainWidget::onAccountReady(Tp::PendingOperation *operation)
{
    Tp::AccountPtr account = Tp::AccountPtr::dynamicCast(operation->object());

    if (account->connection()) {
        monitorPresence(account->connection());
    }
}

void MainWidget::monitorPresence(const Tp::ConnectionPtr &connection)
{
    kDebug();
    connect(connection->contactManager().data(), SIGNAL(presencePublicationRequested(Tp::Contacts)),
            this, SLOT(onPresencePublicationRequested(Tp::Contacts)));

    connect(connection->contactManager().data(),
            SIGNAL(stateChanged(Tp::ContactListState)),
            this, SLOT(onContactManagerStateChanged(Tp::ContactListState)));
    onContactManagerStateChanged(connection->contactManager(),
                                 connection->contactManager()->state());
}

void MainWidget::onContactManagerStateChanged(Tp::ContactListState state)
{
    onContactManagerStateChanged(Tp::ContactManagerPtr(qobject_cast< Tp::ContactManager* >(sender())), state);
}

void MainWidget::onContactManagerStateChanged(const Tp::ContactManagerPtr &contactManager, Tp::ContactListState state)
{
    if (state == Tp::ContactListStateSuccess) {
        QFutureWatcher< Tp::ContactPtr > watcher;
        connect(&watcher, SIGNAL(finished()), this, SLOT(onAccountsPresenceStatusFiltered()));
        watcher.setFuture(QtConcurrent::filtered(contactManager->allKnownContacts(),
                                                kde_tp_filter_contacts_by_publication_status));

        kDebug() << "Watcher is on";
    }
}

void MainWidget::onAccountStateChanged(bool enabled)
{
    Tp::AccountPtr account(qobject_cast<Tp::Account*>(sender()));

    if(enabled) {
        findChild<AccountButton *>(account->uniqueIdentifier())->show();
    } else {
        findChild<AccountButton *>(account->uniqueIdentifier())->hide();
        showMessageToUser(i18n("Account %1 was disabled!", account->displayName()),
                          MainWidget::SystemMessageError);
    }
}

void MainWidget::onAccountRemoved()
{
    Tp::AccountPtr account(qobject_cast<Tp::Account*>(sender()));
    delete findChild<AccountButton *>(account->uniqueIdentifier());

    showMessageToUser(i18n("Account %1 was removed!", account->displayName()),
                      MainWidget::SystemMessageError);
}

void MainWidget::onConnectionChanged(const Tp::ConnectionPtr& connection)
{
    if(! connection.isNull()) {
        monitorPresence(connection);
    }
}

void MainWidget::onContactListDoubleClick(const QModelIndex& index)
{
    if(!index.isValid()) {
        return;
    }

    if(index.data(AccountsModel::AliasRole).toString().isEmpty()) {
        if(m_contactsListView->isExpanded(index))
            m_contactsListView->collapse(index);
        else m_contactsListView->expand(index);
    }
    else {
        kDebug() << "Text chat requested for index" << index;
        startTextChannel(index);
    }
}

void MainWidget::startTextChannel(const QModelIndex &index)
{
    if (! index.isValid()) {
        return;
    }

    QModelIndex realIndex = m_modelFilter->mapToSource(index);
    Tp::ContactPtr contact = m_model->data(realIndex, AccountsModel::ItemRole).value<ContactModelItem*>()->contact();

    kDebug() << "Requesting chat for contact" << contact->alias();

    Tp::AccountPtr account = m_model->accountForContactIndex(realIndex);

    Tp::PendingChannelRequest* channelRequest = account->ensureTextChat(contact,
                                                                        QDateTime::currentDateTime(),
                                                                        PREFERRED_TEXTCHAT_HANDLER);
    connect(channelRequest, SIGNAL(finished(Tp::PendingOperation*)),
            this, SLOT(slotGenericOperationFinished(Tp::PendingOperation*)));
}

void MainWidget::startAudioChannel(const QModelIndex& index)
{
    if (! index.isValid()) {
        return;
    }

    QModelIndex realIndex = m_modelFilter->mapToSource(index);
    Tp::ContactPtr contact = m_model->data(realIndex, AccountsModel::ItemRole).value<ContactModelItem*>()->contact();

    kDebug() << "Requesting audio for contact" << contact->alias();

    Tp::AccountPtr account = m_model->accountForContactIndex(realIndex);

    Tp::PendingChannelRequest* channelRequest = account->ensureStreamedMediaAudioCall(contact,
                                                                        QDateTime::currentDateTime(),
                                                                        PREFERRED_AUDIO_VIDEO_HANDLER);
    connect(channelRequest, SIGNAL(finished(Tp::PendingOperation*)),
            this, SLOT(slotGenericOperationFinished(Tp::PendingOperation*)));
}

void MainWidget::startVideoChannel(const QModelIndex& index)
{
    if (! index.isValid()) {
        return;
    }

    QModelIndex realIndex = m_modelFilter->mapToSource(index);
    Tp::ContactPtr contact = m_model->data(realIndex, AccountsModel::ItemRole).value<ContactModelItem*>()->contact();

    kDebug() << "Requesting video for contact" << contact->alias();

    Tp::AccountPtr account = m_model->accountForContactIndex(realIndex);

    Tp::PendingChannelRequest* channelRequest = account->ensureStreamedMediaVideoCall(contact, true,
                                                                        QDateTime::currentDateTime(),
                                                                        PREFERRED_AUDIO_VIDEO_HANDLER);
    connect(channelRequest, SIGNAL(finished(Tp::PendingOperation*)),
            this, SLOT(slotGenericOperationFinished(Tp::PendingOperation*)));
}


void MainWidget::startFileTransferChannel(const QModelIndex &index)
{
    if (! index.isValid()) {
        return;
    }

    QModelIndex realIndex = m_modelFilter->mapToSource(index);
    Tp::ContactPtr contact = m_model->data(realIndex, AccountsModel::ItemRole).value<ContactModelItem*>()->contact();

    kDebug() << "Requesting file transfer for contact" << contact->alias();

    Tp::AccountPtr account = m_model->accountForContactIndex(realIndex);

    QString filename = KFileDialog::getOpenFileName(KUrl(), // TODO Remember directory
                                                    QString(),
                                                    this,
                                                    i18n("Choose a file")
    );

    QFileInfo fileinfo(filename);

    kDebug() << "Filename:" << filename;
    kDebug() << "Content type:" << KMimeType::findByFileContent(filename)->name();
    kDebug() << "Size:" << fileinfo.size();
    kDebug() << "Last modified:" << fileinfo.lastModified();

    Tp::FileTransferChannelCreationProperties fileTransferProperties(filename,
                                                                     KMimeType::findByFileContent(filename)->name(),
                                                                     fileinfo.size());
    // TODO Add file hash? -- fileTransferProperties.setContentHash();
    fileTransferProperties.setLastModificationTime(fileinfo.lastModified());
    // TODO Let the user set a description? -- fileTransferProperties.setDescription();

    Tp::PendingChannelRequest* channelRequest = account->createFileTransfer(contact,
                                                                            fileTransferProperties,
                                                                            QDateTime::currentDateTime(),
                                                                            PREFERRED_FILETRANSFER_HANDLER);
    connect(channelRequest, SIGNAL(finished(Tp::PendingOperation*)), SLOT(slotGenericOperationFinished(Tp::PendingOperation*)));
}

void MainWidget::showMessageToUser(const QString& text, const MainWidget::SystemMessageType type)
{
    //The pointer is automatically deleted when the event is closed
    KNotification *notification;
    if (type == MainWidget::SystemMessageError) {
        notification = new KNotification("telepathyError", this);
    } else {
        notification = new KNotification("telepathyInfo", this);
    }

    notification->setText(text);
    notification->sendEvent();
}

void MainWidget::addOverlayButtons()
{
    TextChannelContactOverlay*  textOverlay  = new TextChannelContactOverlay(this);
    AudioChannelContactOverlay* audioOverlay = new AudioChannelContactOverlay(this);
    VideoChannelContactOverlay* videoOverlay = new VideoChannelContactOverlay(this);

    FileTransferContactOverlay* fileOverlay  = new FileTransferContactOverlay(this);

    m_delegate->installOverlay(textOverlay);
    m_delegate->installOverlay(audioOverlay);
    m_delegate->installOverlay(videoOverlay);
    m_delegate->installOverlay(fileOverlay);

    textOverlay->setView(m_contactsListView);
    textOverlay->setActive(true);

    audioOverlay->setView(m_contactsListView);
    audioOverlay->setActive(true);

    videoOverlay->setView(m_contactsListView);
    videoOverlay->setActive(true);

    fileOverlay->setView(m_contactsListView);
    fileOverlay->setActive(true);

    connect(textOverlay, SIGNAL(overlayActivated(QModelIndex)),
            m_delegate, SLOT(hideStatusMessageSlot(QModelIndex)));

    connect(textOverlay, SIGNAL(overlayHidden()),
            m_delegate, SLOT(reshowStatusMessageSlot()));

    connect(textOverlay, SIGNAL(activated(QModelIndex)),
            this, SLOT(startTextChannel(QModelIndex)));

    connect(fileOverlay, SIGNAL(activated(QModelIndex)),
            this, SLOT(startFileTransferChannel(QModelIndex)));

    connect(audioOverlay, SIGNAL(activated(QModelIndex)),
            this, SLOT(startAudioChannel(QModelIndex)));

    connect(videoOverlay, SIGNAL(activated(QModelIndex)),
            this, SLOT(startVideoChannel(QModelIndex)));
}

void MainWidget::toggleSearchWidget(bool show)
{
    if(show) {
        m_filterBar->show();
    }
    else {
        m_modelFilter->clearFilterString();
        m_filterBar->clear();
        m_filterBar->hide();
    }
}

void MainWidget::onAddContactRequest() {
    QWeakPointer<AddContactDialog> dialog = new AddContactDialog(m_model, this);
    if (dialog.data()->exec() == QDialog::Accepted) {
    Tp::AccountPtr account = dialog.data()->account();
        QStringList identifiers = QStringList() << dialog.data()->screenName();
        Tp::PendingContacts* pendingContacts = account->connection()->contactManager()->contactsForIdentifiers(identifiers);
        connect(pendingContacts, SIGNAL(finished(Tp::PendingOperation*)), SLOT(onAddContactRequestFoundContacts(Tp::PendingOperation*)));
    }
    delete dialog.data();
}

void MainWidget::onAddContactRequestFoundContacts(Tp::PendingOperation *operation) {
    Tp::PendingContacts *pendingContacts = qobject_cast<Tp::PendingContacts*>(operation);

    if (! pendingContacts->isError()) {
        //request subscription
        pendingContacts->manager()->requestPresenceSubscription(pendingContacts->contacts());
    }
    else {
        kDebug() << pendingContacts->errorName();
        kDebug() << pendingContacts->errorMessage();
    }
}

void MainWidget::onCustomContextMenuRequested(const QPoint &)
{
    QModelIndex index = m_contactsListView->currentIndex();

    Tp::ContactPtr contact = m_model->contactForIndex(m_modelFilter->mapToSource(index));
    if (contact.isNull()) {
        kDebug() << "Contact is nulled";
        return;
    }

    Tp::AccountPtr account = m_model->accountForContactIndex(m_modelFilter->mapToSource(index));
    if (account.isNull()) {
        kDebug() << "Account is nulled";
        return;
    }

    QScopedPointer<KMenu> menu(new KMenu);
    menu->addTitle(contact->alias());

    QAction* action = menu->addAction(i18n("Start Chat..."));
    action->setIcon(KIcon("mail-message-new"));
    action->setDisabled(true);
    connect(action, SIGNAL(triggered(bool)),
            SLOT(slotStartTextChat()));

    if (index.data(AccountsModel::TextChatCapabilityRole).toBool()) {
        action->setEnabled(true);
    }

    Tp::ConnectionPtr accountConnection = account->connection();
    if (accountConnection.isNull()) {
        kDebug() << "Account connection is nulled.";
        return;
    }

    action = menu->addAction(i18n("Start Audio Call..."));
    action->setIcon(KIcon("voicecall"));
    action->setDisabled(true);
    connect(action, SIGNAL(triggered(bool)),
            SLOT(slotStartAudioChat()));

    if (index.data(AccountsModel::AudioCallCapabilityRole).toBool()) {
        action->setEnabled(true);
    }

    action = menu->addAction(i18n("Start Video Call..."));
    action->setIcon(KIcon("webcamsend"));
    action->setDisabled(true);
    connect(action, SIGNAL(triggered(bool)),
            SLOT(slotStartVideoChat()));

    if (index.data(AccountsModel::VideoCallCapabilityRole).toBool()) {
        action->setEnabled(true);
    }

    action = menu->addAction(i18n("Send File..."));
    action->setIcon(KIcon("mail-attachment"));
    action->setDisabled(true);
    connect(action, SIGNAL(triggered(bool)),
            SLOT(slotStartFileTransfer()));

    if (index.data(AccountsModel::FileTransferCapabilityRole).toBool()) {
        action->setEnabled(true);
    }

    menu->addSeparator();

    // remove contact action
    QAction *removeAction = menu->addAction(KIcon("list-remove-user"), i18n("Remove Contact"));
    connect(removeAction, SIGNAL(triggered(bool)), this, SLOT(slotDeleteContact()));

    if (accountConnection->actualFeatures().contains(Tp::Connection::FeatureRosterGroups)) {
        QMenu* groupAddMenu = menu->addMenu(i18n("Move to Group"));

        QStringList currentGroups = contact->groups();
        QStringList allGroups = accountConnection->contactManager()->allKnownGroups();
        foreach (const QString &group, currentGroups) {
            allGroups.removeAll(group);
        }

        groupAddMenu->addAction(i18n("Create New Group..."));
        groupAddMenu->addSeparator();

        foreach (const QString &group, allGroups) {
            connect(groupAddMenu->addAction(group), SIGNAL(triggered(bool)),
                    SLOT(slotAddContactToGroupTriggered()));
        }
    } else {
        kDebug() << "Unable to support Groups";
    }

    //menu->addSeparator();

    // TODO: Remove when Telepathy actually supports blocking.
    /*if (contact->isBlocked()) {
        action = menu->addAction(i18n("Unblock User"));
        connect(action, SIGNAL(triggered(bool)),
                SLOT(slotUnblockContactTriggered()));
    } else {
        action = menu->addAction(i18n("Blocked"));
        connect(action, SIGNAL(triggered(bool)),
                SLOT(slotBlockContactTriggered()));
    }*/

    menu->exec(QCursor::pos());
}

void MainWidget::slotAddContactToGroupTriggered()
{
    QModelIndex index = m_contactsListView->currentIndex();
    Tp::ContactPtr contact = m_model->contactForIndex(m_modelFilter->mapToSource(index));
    if (contact.isNull()) {
        kDebug() << "Contact is nulled";
        return;
    }

    QAction *action = qobject_cast<QAction*>(sender());
    if (!action) {
        kDebug() << "Invalid action";
        return;
    }

    const QStringList currentGroups = contact->groups();

    Tp::PendingOperation* operation = contact->addToGroup(action->text().remove('&'));

    if (operation) {
        connect(operation, SIGNAL(finished(Tp::PendingOperation*)),
                SLOT(slotGenericOperationFinished(Tp::PendingOperation*)));

        foreach (const QString &group, currentGroups) {
            Tp::PendingOperation* operation = contact->removeFromGroup(group);
            connect(operation, SIGNAL(finished(Tp::PendingOperation*)),
                    SLOT(slotGenericOperationFinished(Tp::PendingOperation*)));
        }
    }
}

void MainWidget::slotBlockContactTriggered()
{
    QModelIndex index = m_contactsListView->currentIndex();
    Tp::ContactPtr contact = m_model->contactForIndex(m_modelFilter->mapToSource(index));

    if (contact.isNull()) {
        kDebug() << "Contact is nulled";
        return;
    }

    Tp::PendingOperation *operation = contact->block(true);
    connect(operation, SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(slotGenericOperationFinished(Tp::PendingOperation*)));
}

void MainWidget::slotDeleteContact()
{
    QModelIndex index = m_contactsListView->currentIndex();
    Tp::ContactPtr contact = m_model->contactForIndex(m_modelFilter->mapToSource(index));

    if (contact.isNull()) {
        kDebug() << "Contact is null";
        return;
    }

    QList<Tp::ContactPtr>contactList;
    contactList.append(contact);

    // ask for confirmation
    QWeakPointer<RemoveContactDialog> removeDialog = new RemoveContactDialog(contact, this);

    if (removeDialog.data()->exec() == QDialog::Accepted) {
        // remove from contact list
        Tp::PendingOperation *deleteOp = contact->manager()->removeContacts(contactList);
        connect(deleteOp, SIGNAL(finished(Tp::PendingOperation*)), this, SLOT(slotGenericOperationFinished(Tp::PendingOperation*)));

        if (removeDialog.data()->blockContact()) {
            // block contact
            Tp::PendingOperation *blockOp = contact->manager()->blockContacts(contactList);
            connect(blockOp, SIGNAL(finished(Tp::PendingOperation*)), this, SLOT(slotGenericOperationFinished(Tp::PendingOperation*)));
        }
    }

    delete removeDialog.data();
}

void MainWidget::slotGenericOperationFinished(Tp::PendingOperation* operation)
{
    if (operation->isError()) {
        QString errorMsg(operation->errorName() + ": " + operation->errorMessage());
        showMessageToUser(errorMsg, SystemMessageError);
    }
}

void MainWidget::slotStartTextChat()
{
    QModelIndex index = m_contactsListView->currentIndex();
    if (!index.isValid()) {
        kDebug() << "Invalid index provided.";
        return;
    }

    startTextChannel(index);
}

void MainWidget::slotStartAudioChat()
{
    QModelIndex index = m_contactsListView->currentIndex();
    if (!index.isValid()) {
        kDebug() << "Invalid index provided.";
        return;
    }

    startAudioChannel(index);
}

void MainWidget::slotStartVideoChat()
{
    QModelIndex index = m_contactsListView->currentIndex();
    if (!index.isValid()) {
        kDebug() << "Invalid index provided.";
        return;
    }

    startVideoChannel(index);
}

void MainWidget::slotStartFileTransfer()
{
    QModelIndex index = m_contactsListView->currentIndex();
    if (!index.isValid()) {
        kDebug() << "Invalid index provided.";
        return;
    }

    startFileTransferChannel(index);
}

void MainWidget::slotUnblockContactTriggered()
{
    QModelIndex index = m_contactsListView->currentIndex();
    Tp::ContactPtr contact = m_model->contactForIndex(m_modelFilter->mapToSource(index));
    if (contact.isNull()) {
        kDebug() << "Contact is nulled";
        return;
    }

    Tp::PendingOperation *operation = contact->block(false);
    connect(operation, SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(slotGenericOperationFinished(Tp::PendingOperation*)));
}


void MainWidget::setCustomPresenceMessage(const QString& message)
{
    for (int i = 0; i < m_accountButtonsLayout->count() - 1; i++) {
        qobject_cast<AccountButton*>(m_accountButtonsLayout->itemAt(i)->widget())->setCustomPresenceMessage(message);

    }

    m_presenceMessageEdit->clearFocus();
}

void MainWidget::showSettingsKCM()
{
    KSettings::Dialog *dialog = new KSettings::Dialog(this);

    dialog->addModule("kcm_telepathy_accounts");

    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->exec();
}

void MainWidget::loadAvatar(const Tp::AccountPtr &account)
{
    if (!account->avatar().avatarData.isEmpty()) {
        QIcon icon;
        Tp::Avatar avatar = account->avatar();
        icon.addPixmap(QPixmap::fromImage(QImage::fromData(avatar.avatarData)).scaled(48, 48));

        QToolButton *avatarButton = new QToolButton(this);
        avatarButton->setIcon(icon);
        avatarButton->setIconSize(QSize(48, 48));
        avatarButton->setText(i18nc("String in menu saying Use avatar from account X",
                                    "Use from %1", account->displayName()));
        avatarButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

        QWidgetAction *avatarAction = new QWidgetAction(m_avatarButtonMenu);
        avatarAction->setDefaultWidget(avatarButton);
        avatarAction->setData(account->uniqueIdentifier());

        connect(avatarButton, SIGNAL(clicked(bool)),
                avatarAction, SIGNAL(triggered(bool)));

        connect(avatarAction, SIGNAL(triggered(bool)),
                this, SLOT(selectAvatarFromAccount()));


        m_avatarButtonMenu->addAction(avatarAction);
    }
}

void MainWidget::selectAvatarFromAccount()
{
    selectAvatarFromAccount(qobject_cast<QWidgetAction*>(sender())->data().toString());
}

void MainWidget::selectAvatarFromAccount(const QString &accountUID)
{
    if (accountUID.isEmpty()) {
        kDebug() << "Supplied accountUID is empty, aborting...";
        return;
    }

    Tp::Avatar avatar = qobject_cast<AccountsModelItem*>(m_model->accountItemForId(accountUID))->data(AccountsModel::AvatarRole).value<Tp::Avatar>();

    foreach (const Tp::AccountPtr account, m_accountManager->allAccounts()) {
        //don't set the avatar for the account from where it was taken
        if (account->uniqueIdentifier() == accountUID) {
            continue;
        }

        account->setAvatar(avatar);
    }

    //add the selected avatar as the icon of avatar button
    QIcon icon;
    icon.addPixmap(QPixmap::fromImage(QImage::fromData(avatar.avatarData)).scaled(48, 48));
    m_userAccountIconButton->setIcon(icon);

    m_avatarButtonMenu->close();

    //save the selected account into config
    KSharedConfigPtr config = KGlobal::config();
    KConfigGroup avatarGroup(config, "Avatar");
    avatarGroup.writeEntry("method", "account");
    avatarGroup.writeEntry("source", accountUID);
    avatarGroup.config()->sync();

}

void MainWidget::loadAvatarFromFile()
{
    if (m_accountManager->allAccounts().isEmpty()) {
        int returnCode = KMessageBox::warningYesNo(this,
                                           i18nc("Dialog text", "You have no accounts set. Would you like to set one now?"),
                                           i18nc("Dialog caption", "No accounts set"));

        if (returnCode == KMessageBox::Yes) {
            showSettingsKCM();
            loadAvatarFromFile();
        } else {
            return;
        }
    } else {
        FetchAvatarJob *job = new FetchAvatarJob(KFileDialog::getImageOpenUrl(KUrl(), this,
                                                                              i18n("Please choose your avatar")),
                                                                              this);

        connect(job, SIGNAL(result(KJob*)),
                this, SLOT(onAvatarFetched(KJob*)));

        job->start();
    }
}

void MainWidget::onAvatarFetched(KJob *job)
{
    if (job->error()) {
        KMessageBox::error(this, job->errorString());
        return;
    }

    //this should never be true, but better one "if" than a crash
    if (m_accountManager->allAccounts().isEmpty()) {
        int returnCode = KMessageBox::warningYesNo(this,
                                                   i18nc("Dialog text", "You have no accounts set. Would you like to set one now?"),
                                                   i18nc("Dialog caption", "No accounts set"));

        if (returnCode == KMessageBox::Yes) {
            showSettingsKCM();
        } else {
            return;
        }
    } else {

        FetchAvatarJob *fetchJob = qobject_cast< FetchAvatarJob* >(job);

        Q_ASSERT(fetchJob);

        foreach (const Tp::AccountPtr account, m_accountManager->allAccounts()) {
            Tp::PendingOperation *op = account->setAvatar(fetchJob->avatar());

            //connect for eventual error displaying
            connect(op, SIGNAL(finished(Tp::PendingOperation*)),
                    this, SLOT(slotGenericOperationFinished(Tp::PendingOperation*)));
        }

        //add the selected avatar to the avatar button
        QIcon icon;
        icon.addPixmap(QPixmap::fromImage(QImage::fromData(fetchJob->avatar().avatarData)).scaled(48, 48));
        m_userAccountIconButton->setIcon(icon);

        //since all the accounts will have the same avatar,
        //we take simply the first in AM and use this in config
        KSharedConfigPtr config = KGlobal::config();
        KConfigGroup avatarGroup(config, "Avatar");
        avatarGroup.writeEntry("method", "account");
        avatarGroup.writeEntry("source", m_accountManager->allAccounts().first()->uniqueIdentifier());
        avatarGroup.config()->sync();
    }
}

void MainWidget::onAccountsPresenceStatusFiltered()
{
    kDebug() << "Watcher is here";
    QFutureWatcher< Tp::ContactPtr > *watcher = dynamic_cast< QFutureWatcher< Tp::ContactPtr > * >(sender());
    kDebug() << "Watcher is casted";
    Tp::Contacts contacts = watcher->future().results().toSet();
    kDebug() << "Watcher is used";
    if (!contacts.isEmpty()) {
        onPresencePublicationRequested(contacts);
    }
}

void MainWidget::onPresencePublicationRequested(const Tp::Contacts& contacts)
{
    foreach (const Tp::ContactPtr &contact, contacts) {
        if (KMessageBox::questionYesNo(this, i18n("The contact %1 added you to their contact list. "
                                                  "Do you want to allow this person to see your presence "
                                                  "and add them to your contact list?", contact->id()),
                                       i18n("Subscription request")) == KDialog::Yes) {
            Tp::ContactManagerPtr manager = contact->manager();
            manager->authorizePresencePublication(QList< Tp::ContactPtr >() << contact);

            if (manager->canRequestPresenceSubscription() && contact->subscriptionState() == Tp::Contact::PresenceStateNo) {
                manager->requestPresenceSubscription(QList< Tp::ContactPtr >() << contact);
            }
        }
    }
}

void MainWidget::handleConnectionError(const Tp::AccountPtr& account)
{
    QString connectionError = account->connectionError();

    // ignore user disconnect
    if (connectionError == "org.freedesktop.Telepathy.Error.Cancelled") {
        return;
    }

    Tp::ConnectionStatusReason reason = account->connectionStatusReason();

    if (reason == Tp::ConnectionStatusReasonAuthenticationFailed) {
        showMessageToUser(i18n("Couldn't connect %1. Authentication failed (is your password correct?)", account->displayName()), MainWidget::SystemMessageError);
    } else if (reason == Tp::ConnectionStatusReasonNetworkError) {
        showMessageToUser(i18n("Couldn't connect %1. There was a network error, check your connection", account->displayName()), MainWidget::SystemMessageError);
    } else {
        // other errors
        showMessageToUser(i18n("An unexpected error has occurred with %1: '%2'", account->displayName(), account->connectionError()), MainWidget::SystemMessageError);
    }
}


#include "main-widget.moc"
