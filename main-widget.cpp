/*
 * This file is part of ktp-contact-list
 *
 * Copyright (C) 2009-2010 Collabora Ltd. <info@collabora.co.uk>
 *   @Author George Goldberg <george.goldberg@collabora.co.uk>
 * Copyright (C) 2011 Keith Rusler <xzekecomax@gmail.com>
 * Copyright (C) 2011-2013 Martin Klapetek <martin.klapetek@gmail.com>
 * Copyright (C) 2012-2012 David Edmundson <kde@davidedmundson.co.uk>
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
#include "ktp-contactlist-debug.h"

#include <QSortFilterProxyModel>
#include <QPainter>
#include <QMenu>
#include <QToolButton>
#include <QtCore/QWeakPointer>
#include <QWidgetAction>
#include <QCloseEvent>
#include <QDialog>
#include <QMenu>
#include <QMenuBar>
#include <QDebug>

#include <TelepathyQt/PendingChannelRequest>
#include <TelepathyQt/PendingContacts>
#include <TelepathyQt/ContactManager>
#include <TelepathyQt/PendingReady>
#include <TelepathyQt/TextChannel>

#include <KTp/actions.h>
#include <KTp/contact-factory.h>
#include <KTp/types.h>
#include <KTp/Widgets/add-contact-dialog.h>
#include <KTp/Widgets/join-chat-room-dialog.h>
#include <KTp/Widgets/start-chat-dialog.h>

#include <KIO/Job>
#include <KMessageBox>
#include <KProtocolInfo>
#include <ksettings/Dialog>
#include <KSharedConfig>
#include <KStandardShortcut>
#include <KNotification>
#include <KToolInvocation>
#include <KStandardAction>
#include <KWindowSystem>
#include <KLocalizedString>

#ifdef HAVE_KPEOPLE
// #include <kpeople/widgets/mergedialog.h> //Disable for now, re-enable when kpeople re-enables it
#include <KPeople/PersonsModel>
#endif

#include "ui_main-widget.h"
#include "tooltips/tooltipmanager.h"
#include "context-menu.h"
#include "filter-bar.h"

bool kde_tp_filter_contacts_by_publication_status(const Tp::ContactPtr &contact)
{
    return contact->publishState() == Tp::Contact::PresenceStateAsk;
}

MainWidget::MainWidget(QWidget *parent)
    : KMainWindow(parent),
      m_globalMenu(NULL),
      m_settingsDialog(NULL),
      m_joinChatRoom(NULL),
      m_makeCall(NULL),
      m_mergeContacts(NULL),
      m_contactListTypeGroup(NULL),
      m_blockedFilterGroup(NULL),
      m_quitAction(NULL)
{
    setupUi(this);

    m_filterBar->hide();
    setAutoSaveSettings();
    setupTelepathy();

    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup guiConfigGroup(config, "GUI");
    setupActions(guiConfigGroup);
    setupToolBar();
    setupGlobalMenu();

    // Restore window geometry, global/by-account presence, search widget state
    restoreGeometry(guiConfigGroup.readEntry("window_geometry", QByteArray()));
    toggleSearchWidget(guiConfigGroup.readEntry("pin_filterbar", true));
    m_presenceChooser->show();

    m_contextMenu = new ContextMenu(m_contactsListView);
    new ToolTipManager(m_contactsListView);

    m_messageWidget->setWordWrap(true);
    m_messageWidget->setCloseButtonVisible(true);
    m_messageWidget->hide();

    connect(m_contactsListView, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(onCustomContextMenuRequested(QPoint)));
    connect(m_contactsListView->contactsModel(), SIGNAL(modelInitialized(bool)),
            this, SLOT(onModelInitialized(bool)));

    connect(m_showOfflineAction, SIGNAL(toggled(bool)),
            m_contactsListView, SLOT(toggleOfflineContacts(bool)));
    connect(m_sortByPresenceAction, SIGNAL(activeChanged(bool)),
            m_contactsListView, SLOT(toggleSortByPresence(bool)));
    connect(m_metacontactToggleAction, SIGNAL(triggered(bool)),
            this, SLOT(onMetacontactToggleTriggered()));

    connect(m_filterBar, SIGNAL(filterChanged(QString)),
            m_contactsListView, SLOT(setFilterString(QString)));
    connect(m_filterBar, SIGNAL(closeRequest()),
            m_filterBar, SLOT(hide()));
    connect(m_filterBar, SIGNAL(closeRequest()),
            m_searchContactAction, SLOT(trigger()));

    connect(m_contactsListView, SIGNAL(genericOperationFinished(Tp::PendingOperation*)),
            this, SLOT(onGenericOperationFinished(Tp::PendingOperation*)));

    connect(m_contactsListView, SIGNAL(actionStarted()),
            this, SLOT(clearSearch()));

    connect(m_contactsListView, SIGNAL(contactSelectionChanged()),
            this, SLOT(onContactSelectionChanged()));


    bool showOffline = guiConfigGroup.readEntry("show_offline", false);
    m_showOfflineAction->setChecked(showOffline);
    m_showOfflineAction->setActive(showOffline);

    bool sortByPresence = guiConfigGroup.readEntry("sort_by_presence", true);
    m_sortByPresenceAction->setActive(sortByPresence);

    bool useGroups = guiConfigGroup.readEntry("use_groups", true);
    m_contactsListView->toggleGroups(useGroups);
    m_contactsListView->toggleOfflineContacts(showOffline);
    m_contactsListView->toggleSortByPresence(sortByPresence);
}

MainWidget::~MainWidget()
{
    //save the state of the filter bar, pinned or not
    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup configGroup(config, "GUI");
    configGroup.writeEntry("pin_filterbar", m_searchContactAction->isChecked());
    configGroup.writeEntry("use_groups", m_groupContactsActionGroup->actions().first()->isChecked());
    configGroup.writeEntry("show_offline", m_showOfflineAction->isChecked());
    configGroup.writeEntry("sort_by_presence", m_sortByPresenceAction->isActive());
    configGroup.config()->sync();
}

void MainWidget::onAccountManagerReady(Tp::PendingOperation* op)
{
    if (op && op->isError()) {
        qCDebug(KTP_CONTACTLIST_MODULE) << op->errorName();
        qCDebug(KTP_CONTACTLIST_MODULE) << op->errorMessage();

        KMessageBox::error(this,
                           i18n("Something unexpected happened to the core part of your Instant Messaging system "
                           "and it couldn't be initialized. Try restarting the Contact List."),
                           i18n("IM system failed to initialize"));
        return;
    }

    m_presenceChooser->setAccountManager(m_accountManager);
    m_contactsListView->setAccountManager(m_accountManager);
    m_contextMenu->setAccountManager(m_accountManager);
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

    notification->setComponentName("ktelepathy");

    notification->setText(text);
    notification->sendEvent();
}

void MainWidget::onAddContactRequest()
{
    if (m_accountManager->isReady()) {
        KTp::AddContactDialog *dialog = new KTp::AddContactDialog(m_accountManager, this);
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        dialog->show();
    }
}

void MainWidget::onStartChatRequest()
{
    if (m_accountManager->isReady()) {
        KTp::StartChatDialog *dialog = new KTp::StartChatDialog(m_accountManager, this);
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        dialog->show();
    }
}


void MainWidget::onCustomContextMenuRequested(const QPoint &pos)
{
    QModelIndex index = m_contactsListView->indexAt(pos);

    if (!index.isValid()) {
        return;
    }

    KTp::RowType type = (KTp::RowType)index.data(KTp::RowTypeRole).toInt();

    QMenu *menu = 0;

    if (type == KTp::ContactRowType || type == KTp::PersonRowType) {
        menu = m_contextMenu->contactContextMenu(index);
    } else if (type == KTp::GroupRowType) {
        menu = m_contextMenu->groupContextMenu(index);
    }

    if (menu) {
        menu->exec(QCursor::pos());
        menu->deleteLater();
    }
}

void MainWidget::onGenericOperationFinished(Tp::PendingOperation* operation)
{
    if (operation->isError()) {
        QString errorMsg(operation->errorName() + ": " + operation->errorMessage());
        showMessageToUser(errorMsg, SystemMessageError);
    }
}

void MainWidget::onJoinChatRoomRequested()
{
    if (m_accountManager->isReady()) {
        KTp::JoinChatRoomDialog *dialog = new KTp::JoinChatRoomDialog(m_accountManager);
        dialog->setAttribute(Qt::WA_DeleteOnClose);

        dialog->show();
    }
}

void MainWidget::onMakeCallRequested()
{
    KToolInvocation::kdeinitExec(QLatin1String("ktp-dialout-ui"));
}

void MainWidget::onMergeContactsDialogRequested()
{
    /*
#ifdef HAVE_KPEOPLE
    KPeople::MergeDialog* mergeDialog = new KPeople::MergeDialog(this);
    //create a new model that queries all the data otherwise we will only show IM contacts
    KPeople::PersonsModel* model = new KPeople::PersonsModel(mergeDialog);
    mergeDialog->setPersonsModel(model);
    model->startQuery(KPeople::PersonsModelFeature::allFeatures());
    mergeDialog->show();
    mergeDialog->setAttribute(Qt::WA_DeleteOnClose);
#endif
*/
}

void MainWidget::closeEvent(QCloseEvent* e)
{
    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup generalConfigGroup(config, "General");
    KConfigGroup notifyConigGroup(config, "Notification Messages");
    KConfigGroup guiConfigGroup(config, "GUI");

    if (qApp->closingDown()) {
        //the standard KMessageBox control saves "true" if you select the checkbox, therefore the reversed var name
        bool dontCheckForPlasmoid = notifyConigGroup.readEntry("dont_check_for_plasmoid", false);

        if (isAnyAccountOnline() && !dontCheckForPlasmoid) {
            if (!isPresencePlasmoidPresent()) {
                switch (KMessageBox::warningYesNoCancel(this,
                        i18n("You do not have any other presence controls active (a Presence widget for example).\n"
                            "Do you want to stay online or would you rather go offline?"),
                        i18n("No Other Presence Controls Found"),
                        KGuiItem(i18n("Stay Online"), QIcon::fromTheme("user-online")),
                        KGuiItem(i18n("Go Offline"), QIcon::fromTheme("user-offline")),
                        KStandardGuiItem::cancel(),
                        QString("dont_check_for_plasmoid"))) {

                    case KMessageBox::No:
                        generalConfigGroup.writeEntry("go_offline_when_closing", true);
                        goOffline();
                        break;
                    case KMessageBox::Cancel:
                        e->ignore();
                        return;
                }
            }
        } else if (isAnyAccountOnline() && dontCheckForPlasmoid) {
            bool shouldGoOffline = generalConfigGroup.readEntry("go_offline_when_closing", false);
            if (shouldGoOffline) {
                goOffline();
            }
        }

        generalConfigGroup.config()->sync();
    }

    // Save window geometry
    guiConfigGroup.writeEntry("window_geometry", saveGeometry());
    guiConfigGroup.config()->sync();

    KMainWindow::closeEvent(e);
}

bool MainWidget::isPresencePlasmoidPresent() const
{
    QDBusReply<bool> serviceRegistered = QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.Telepathy.PresenceApplet");

    if (serviceRegistered.isValid() && serviceRegistered.value()) {
        return true;
    } else {
        return false;
    }
}

void MainWidget::goOffline()
{
    //FIXME use global presence
    qCDebug(KTP_CONTACTLIST_MODULE) << "Setting all accounts offline...";
    foreach (const Tp::AccountPtr &account, m_accountManager->allAccounts()) {
        if (account->isEnabled() && account->isValid()) {
            account->setRequestedPresence(Tp::Presence::offline());
        }
    }
}

bool MainWidget::isAnyAccountOnline() const
{
    foreach (const Tp::AccountPtr &account, m_accountManager->allAccounts()) {
        if (account->isEnabled() && account->isValid() && account->isOnline()) {
            return true;
        }
    }

    return false;
}

void MainWidget::toggleSearchWidget(bool show)
{
    m_searchContactAction->setChecked(show);
    if(show) {
        m_filterBar->show();
    } else {
        m_contactsListView->setFilterString(QString());
        m_filterBar->clear();
        m_filterBar->hide();
    }
}

void MainWidget::clearSearch()
{
    m_filterBar->clear();
}

void MainWidget::setupGlobalMenu()
{
    // Since our menu is hidden, its shortcuts do not work.
    // Here's a workarond: we must assign global menu's unique
    // items to main window. Since it's always active when an
    // application is active, shortcuts now will work properly.

    m_globalMenu = new QMenuBar(this);
    m_globalMenu->setVisible(false);

    QMenu *contacts = new QMenu(i18n("Contacts"), m_globalMenu);
    contacts->addAction(m_addContactAction);
    contacts->addAction(m_joinChatRoom);
    if (!QStandardPaths::findExecutable("ktp-dialout-ui").isEmpty()) {
        contacts->addAction(m_makeCall);
    }
    //temporarily disable until funcationality is restored in libkpeople
    if (KTp::kpeopleEnabled()) {
//         contacts->addAction(m_mergeContacts);
    }
    contacts->addAction(m_settingsDialog);
    contacts->addSeparator();
    contacts->addAction(m_quitAction);
    this->addAction(m_quitAction); // Shortcuts workaround.
    m_globalMenu->addMenu(contacts);

    QMenu *view = new QMenu(i18n("View"), m_globalMenu);
    view->addAction(m_showOfflineAction);
    view->addAction(m_sortByPresenceAction);
    view->addSeparator();
    QMenu *view_contactListTypeMenu = new QMenu(i18n("Contact List Type"), view);
    view_contactListTypeMenu->addActions(m_contactListTypeGroup->actions());
    view->addMenu(view_contactListTypeMenu);
    QMenu *view_blockedFilterMenu = new QMenu(i18n("Shown Contacts"), view);
    view_blockedFilterMenu->addActions(m_blockedFilterGroup->actions());
    view->addMenu(view_blockedFilterMenu);
    QMenu *view_showGroupedMenu = new QMenu(i18n("Contact Grouping"), view);
    view_showGroupedMenu->addActions(m_groupContactsActionGroup->actions());
    view->addMenu(view_showGroupedMenu);
    m_globalMenu->addMenu(view);

    m_globalMenu->addMenu(helpMenu());
}

void MainWidget::setupToolBar()
{
    m_toolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    if (KTp::kpeopleEnabled()) {
        m_toolBar->addAction(m_metacontactToggleAction);
    }
    m_toolBar->addAction(m_addContactAction);
    m_toolBar->addAction(m_searchContactAction);
    m_toolBar->addAction(m_showOfflineAction);
    m_toolBar->addAction(m_sortByPresenceAction);
    m_toolBar->addSeparator();
    m_toolBar->addAction(m_startChatAction);
    m_toolBar->addAction(m_joinChatRoom);

    QWidget *toolBarSpacer = new QWidget(this);
    toolBarSpacer->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    m_toolBar->addWidget(toolBarSpacer);

    QToolButton *settingsButton = new QToolButton(this);
    settingsButton->setIcon(QIcon::fromTheme("configure"));
    settingsButton->setPopupMode(QToolButton::InstantPopup);

    QMenu *settingsButtonMenu = new QMenu(settingsButton);
    settingsButtonMenu->addAction(m_settingsDialog);

    QActionGroup *delegateTypeGroup = new QActionGroup(this);
    delegateTypeGroup->setExclusive(true);

    QMenu *setDelegateTypeMenu = new QMenu(settingsButtonMenu);
    setDelegateTypeMenu->setTitle(i18n("Contact List Type"));
    setDelegateTypeMenu->addActions(m_contactListTypeGroup->actions());
    settingsButtonMenu->addMenu(setDelegateTypeMenu);

    QMenu *setBlockedFilterMenu = new QMenu(settingsButtonMenu);
    setBlockedFilterMenu->setTitle(i18n("Shown Contacts"));
    setBlockedFilterMenu->addActions(m_blockedFilterGroup->actions());
    settingsButtonMenu->addMenu(setBlockedFilterMenu);

    QMenu *showGroupedMenu = new QMenu(settingsButton);
    showGroupedMenu->setTitle(i18n("Contact Grouping"));
    showGroupedMenu->addActions(m_groupContactsActionGroup->actions());
    settingsButtonMenu->addMenu(showGroupedMenu);

    if (!QStandardPaths::findExecutable("ktp-dialout-ui").isEmpty()) {
        settingsButtonMenu->addAction(m_makeCall);
    }

    if (KTp::kpeopleEnabled()) {
//         settingsButtonMenu->addAction(m_mergeContacts);
    }

    settingsButtonMenu->addSeparator();
    settingsButtonMenu->addMenu(helpMenu());

    settingsButton->setMenu(settingsButtonMenu);

    m_toolBar->addWidget(settingsButton);
}

void MainWidget::setupTelepathy()
{
    Tp::registerTypes();

    if (KTp::kpeopleEnabled()) {
        m_accountManager = KTp::accountManager();
    } else {

        Tp::AccountFactoryPtr  accountFactory = Tp::AccountFactory::create(QDBusConnection::sessionBus(),
                                                                        Tp::Features() << Tp::Account::FeatureCore
                                                                        << Tp::Account::FeatureAvatar
                                                                        << Tp::Account::FeatureCapabilities
                                                                        << Tp::Account::FeatureProtocolInfo
                                                                        << Tp::Account::FeatureProfile);

        Tp::ConnectionFactoryPtr connectionFactory = Tp::ConnectionFactory::create(QDBusConnection::sessionBus(),
                                                                                Tp::Features() << Tp::Connection::FeatureCore
                                                                                << Tp::Connection::FeatureSelfContact);

        Tp::ContactFactoryPtr contactFactory = KTp::ContactFactory::create(Tp::Features()  << Tp::Contact::FeatureAlias
                                                                        << Tp::Contact::FeatureAvatarToken
                                                                        << Tp::Contact::FeatureAvatarData
                                                                        << Tp::Contact::FeatureSimplePresence
                                                                        << Tp::Contact::FeatureCapabilities
                                                                        << Tp::Contact::FeatureClientTypes);

        Tp::ChannelFactoryPtr channelFactory = Tp::ChannelFactory::create(QDBusConnection::sessionBus());

        m_accountManager = Tp::AccountManager::create(QDBusConnection::sessionBus(),
                                                    accountFactory,
                                                    connectionFactory,
                                                    channelFactory,
                                                    contactFactory);
    }

    connect(m_accountManager->becomeReady(), SIGNAL(finished(Tp::PendingOperation*)),
            this, SLOT(onAccountManagerReady(Tp::PendingOperation*)));
}

QAction *MainWidget::createAction(const QString &text, QObject *signalReceiver, const char *slot, const QIcon &icon = QIcon())
{
    QAction *action = new QAction(icon, text, this);
    action->setToolTip(text);
    connect(action, SIGNAL(triggered(bool)), signalReceiver, slot);
    return action;
}

QAction *MainWidget::createAction(const QString& text, QObject *signalReceiver, const char* slot, bool isChecked, const QIcon &icon = QIcon())
{
    QAction *action = createAction(text, signalReceiver, slot, icon);
    action->setCheckable(true);
    action->setChecked(isChecked);
    return action;
}

void MainWidget::setupActions(const KConfigGroup& guiConfigGroup)
{
    m_settingsDialog = KStandardAction::preferences(m_contactsListView, SLOT(showSettingsKCM()),this);
    m_settingsDialog->setText(i18n("Instant Messaging Settings...")); // We set text manually since standard name is too long

    m_quitAction = KStandardAction::quit(this, SLOT(close()), this);
    m_quitAction->setMenuRole(QAction::QuitRole);

    m_joinChatRoom = createAction(i18n("Join Chat Room..."), this, SLOT(onJoinChatRoomRequested()));
    m_joinChatRoom->setIcon(QIcon::fromTheme("im-irc"));
    m_makeCall = createAction(i18n("Make a Call..."), this, SLOT(onMakeCallRequested()));
    m_mergeContacts = createAction(i18n("Merge Contacts..."), this, SLOT(onMergeContactsDialogRequested()));
    m_addContactAction = createAction(i18n("Add New Contacts..."), this, SLOT(onAddContactRequest()), QIcon::fromTheme("list-add-user"));
    m_searchContactAction = createAction(i18n("Find Contact"), this, SLOT(toggleSearchWidget(bool)),
                                         guiConfigGroup.readEntry("pin_filterbar", true), QIcon::fromTheme("edit-find"));
    m_searchContactAction->setShortcuts(KStandardShortcut::find());
    m_startChatAction = createAction(i18n("Start a Chat..."), this, SLOT(onStartChatRequest()), QIcon::fromTheme("telepathy-kde"));

    // Dual actions
    m_metacontactToggleAction = new KDualAction(i18n("Split Selected Contacts"),
                                                i18n("Merge Selected Contacts"),
                                                this);
    m_metacontactToggleAction->setActiveIcon(QIcon::fromTheme("user-group-new"));
    m_metacontactToggleAction->setInactiveIcon(QIcon::fromTheme("user-group-delete"));
    m_metacontactToggleAction->setActive(true);
    m_metacontactToggleAction->setDisabled(true);
    m_metacontactToggleAction->setAutoToggle(false);

    m_groupContactsActionGroup = new QActionGroup(this);
    m_groupContactsActionGroup->setExclusive(true);
    m_groupContactsActionGroup->addAction(createAction(i18n("Show Contacts by Groups"), m_contactsListView, SLOT(showGrouped()),
                                          guiConfigGroup.readEntry("use_groups", true)));
    QString useGroupsDisabledText;
    if (KTp::kpeopleEnabled()) {
        useGroupsDisabledText = i18n("Do Not Group");
    }
    else {
        useGroupsDisabledText = i18n("Show Contacts by Accounts");
    }
    m_groupContactsActionGroup->addAction(createAction(useGroupsDisabledText, m_contactsListView, SLOT(showUngrouped()),
                                          ! guiConfigGroup.readEntry("use_groups", true)));

    m_showOfflineAction = new KDualAction(i18n("Show Offline Contacts"),
                                          i18n("Hide Offline Contacts"),
                                          this);
    m_showOfflineAction->setActiveIcon(QIcon::fromTheme("show-offline"));
    m_showOfflineAction->setInactiveIcon(QIcon::fromTheme("show-offline"));
    m_showOfflineAction->setCheckable(true);
    m_showOfflineAction->setChecked(false);
    m_showOfflineAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_S));

    m_sortByPresenceAction = new KDualAction(i18n("Sort by Presence"),
                                             i18n("Sort by Name"),
                                             this);
    m_sortByPresenceAction->setActiveIcon(QIcon::fromTheme("sort-presence"));
    m_sortByPresenceAction->setInactiveIcon(QIcon::fromTheme("sort-name"));

    // Setup contact list appearance
    m_contactListTypeGroup = new QActionGroup(this);
    m_contactListTypeGroup->setExclusive(true);
    m_contactListTypeGroup->addAction(createAction(i18n("Use Full List"), m_contactsListView, SLOT(onSwitchToFullView()),
                                                   guiConfigGroup.readEntry("selected_delegate", "normal") == QLatin1String("full")));
    m_contactListTypeGroup->addAction(createAction(i18n("Use Normal List"), m_contactsListView, SLOT(onSwitchToCompactView()),
                                                   guiConfigGroup.readEntry("selected_delegate", "normal") == QLatin1String("normal")
                                                   || guiConfigGroup.readEntry("selected_delegate", "normal") == QLatin1String("compact"))); //needed for backwards compatibility

    m_contactListTypeGroup->addAction(createAction(i18n("Use Minimalistic List"), m_contactsListView, SLOT(onSwitchToMiniView()),
                                                   guiConfigGroup.readEntry("selected_delegate", "normal") == QLatin1String("mini")));

    // Setup blocked contacts filtering
    QString shownContacts = guiConfigGroup.readEntry("shown_contacts", "unblocked");
    m_blockedFilterGroup = new QActionGroup(this);
    m_blockedFilterGroup->setExclusive(true);
    m_blockedFilterGroup->addAction(createAction(i18n("Show All Contacts"), m_contactsListView, SLOT(onShowAllContacts()),
                                                shownContacts == QLatin1String("all")));
    m_blockedFilterGroup->addAction(createAction(i18n("Show Unblocked Contacts"), m_contactsListView, SLOT(onShowUnblockedContacts()),
                                                shownContacts == QLatin1String("unblocked")));
    m_blockedFilterGroup->addAction(createAction(i18n("Show Blocked Contacts"), m_contactsListView, SLOT(onShowBlockedContacts()),
                                                shownContacts == QLatin1String("blocked")));
}

void MainWidget::toggleWindowVisibility()
{
    if (isActiveWindow()) {
        close();
    } else {
        KWindowSystem::forceActiveWindow(this->effectiveWinId());
    }
}

void MainWidget::onContactSelectionChanged()
{
    QModelIndexList selection = m_contactsListView->selectionModel()->selectedIndexes();
    if (selection.size() == 0) {
        //if nothing is selected, disable the button
        m_metacontactToggleAction->setActive(true);
        m_metacontactToggleAction->setDisabled(true);
        return;
    } else if (selection.size() == 1) {
        QModelIndex index = selection.first();
        if (index.data(KTp::RowTypeRole).toInt() == KTp::PersonRowType ||
            (index.parent().isValid() && index.parent().data(KTp::RowTypeRole).toInt() == KTp::PersonRowType)) {
            //if a person is selected or a subcontact is selected, switch to unlink action and enable
            m_metacontactToggleAction->setActive(false);
            m_metacontactToggleAction->setEnabled(true);
            return;
        }
    } else if (selection.size() > 1) {
        bool invalid = false;
        //we cannot merge child contact of a person with anything else
        Q_FOREACH (const QModelIndex &index, selection) {
            if (index.parent().isValid() && index.parent().data(KTp::RowTypeRole).toInt() == KTp::PersonRowType) {
                invalid = true;
                break;
            }
        }

        if (!invalid) {
            m_metacontactToggleAction->setActive(true);
            m_metacontactToggleAction->setEnabled(true);
            return;
        }
    }

    m_metacontactToggleAction->setActive(true);
    m_metacontactToggleAction->setDisabled(true);
}

void MainWidget::onMetacontactToggleTriggered()
{
#ifdef HAVE_KPEOPLE
    const QModelIndexList selection = m_contactsListView->selectionModel()->selectedIndexes();

    Q_ASSERT(!selection.isEmpty());
    if (m_metacontactToggleAction->isActive()) {
        //we're merging contacts
        bool invalid = false;
        QModelIndex person;
        QStringList uris;

        Q_FOREACH (const QModelIndex &index, selection) {
            if (index.parent().isValid()
                    && index.parent().data(KTp::RowTypeRole).toInt() == KTp::PersonRowType) {
                //we can merge only standalone contacts, not a contact that's already part of a person
                invalid = true;
                qCDebug(KTP_CONTACTLIST_MODULE) << "Found selected subcontact, aborting";
                break;
            }

            //the selection can have at most one person, so if we encounter second person
            //we break and do nothing
            if (index.data(KTp::RowTypeRole).toInt() == KTp::PersonRowType) {
                if (person.isValid()) {
                    invalid = true;
                    qCDebug(KTP_CONTACTLIST_MODULE) << "Found second person, aborting";
                    break;
                } else {
                    qCDebug(KTP_CONTACTLIST_MODULE) << "Found a person, adding";
                    person = index;
                }
            }

            //if we're dealing with contact that's a child of selected person
            //(we should never get here)
            if (index.parent().isValid() && index.parent() == person) {
                invalid = true;
                qCDebug(KTP_CONTACTLIST_MODULE) << "Found subcontact of selected person, aborting";
                break;
            }

            uris << index.data(KTp::PersonIdRole).toString();
        }

        if (!invalid) {
            KPeople::mergeContacts(uris);
        }
    } else {
        //we're removing contacts from person
        QStringList contacts;
        const QModelIndex &index = selection.first();
        QString uri;

        if (index.parent().isValid() && index.parent().data(KTp::RowTypeRole).toInt() == KTp::PersonRowType) {
            uri = index.data(KTp::ContactUriRole).toString();
        } else {
            uri = index.data(KTp::PersonIdRole).toString();
        }
        KPeople::unmergeContact(uri);
    }
#endif
}

void MainWidget::onModelInitialized(bool success)
{
    if (!success) {
        m_messageWidget->setMessageType(KMessageWidget::Warning);
        m_messageWidget->setText(i18n("Some data sources failed to initialize properly, your contact list might be incomplete."));
        m_messageWidget->animatedShow();
    }

    m_contactsListView->contactsModel()->setTrackUnreadMessages(true);
}
