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

#include <QtGui/QSortFilterProxyModel>
#include <QtGui/QPainter>
#include <QtGui/QMenu>
#include <QtGui/QToolButton>
#include <QtCore/QWeakPointer>
#include <QWidgetAction>
#include <QCloseEvent>

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

#include <KDebug>
#include <KDialog>
#include <KIO/Job>
#include <KMenu>
#include <KMessageBox>
#include <KProtocolInfo>
#include <KSettings/Dialog>
#include <KSharedConfig>
#include <KStandardDirs>
#include <KStandardShortcut>
#include <KNotification>
#include <KToolInvocation>
#include <KMenuBar>
#include <KStandardAction>
#include <KWindowSystem>

#include "ui_main-widget.h"
#include "account-buttons-panel.h"
#include "contact-list-application.h"
#include "tooltips/tooltipmanager.h"
#include "context-menu.h"

/** Start of bodge to work around https://bugs.freedesktop.org/show_bug.cgi?id=57739
    Due to a bug in TelepathyQt if we request a feature that the connection does not support, it simply fails.

    Local-XMPP does not support grouping, so salut the local-xmpp spec does not either.
    When we request the ContactRosterGroups feature on all connections, salut fails and therefore doesn't show any contacts.

    Fetching ContactRosterGroups seperately is not a viable option as contacts would move after loading

    In this hack we make a new ConnectionFactory that fetches ContactRosterGroups on all connections _Except_ salut
    by overriding the featuresFor method which determines which features should be added to a given DBus proxy, in this case a connection.

    When https://bugs.freedesktop.org/show_bug.cgi?id=57739 is fixes all this code should be removed and we should create a standard Tp::ConnectionFactory
 */
namespace KTp {
    class ConnectionFactory : Tp::ConnectionFactory {
    public:
        static Tp::ConnectionFactoryPtr create(const QDBusConnection &bus, const Tp::Features &features=Tp::Features());
    protected:
        ConnectionFactory(const QDBusConnection &bus, const Tp::Features& features);
        virtual Tp::Features featuresFor(const Tp::DBusProxyPtr &proxy) const;
    };
}

Tp::ConnectionFactoryPtr KTp::ConnectionFactory::create(const QDBusConnection &bus, const Tp::Features &features)
{
    return Tp::ConnectionFactoryPtr(new KTp::ConnectionFactory(bus, features));
}

KTp::ConnectionFactory::ConnectionFactory(const QDBusConnection &bus, const Tp::Features &features): Tp::ConnectionFactory(bus, features)
{
}

Tp::Features KTp::ConnectionFactory::featuresFor(const Tp::DBusProxyPtr &proxy) const
{
    Tp::Features features = Tp::FixedFeatureFactory::featuresFor(proxy);

    Tp::ConnectionPtr cm = Tp::ConnectionPtr::qObjectCast<>(proxy);
    if (cm && cm->cmName() == QLatin1String("salut")) {
        features.remove(Tp::Connection::FeatureRosterGroups);
    }
    return features;
}
/** End of bodge*/



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
      m_contactListTypeGroup(NULL),
      m_blockedFilterGroup(NULL),
      m_quitAction(NULL)
{
    setupUi(this);

    m_filterBar->hide();
    setWindowIcon(KIcon("telepathy-kde"));
    setAutoSaveSettings();
    setupTelepathy();

    KSharedConfigPtr config = KGlobal::config();
    KConfigGroup guiConfigGroup(config, "GUI");
    setupActions(guiConfigGroup);
    setupToolBar();
    setupGlobalMenu();

    // Restore window geometry, global/by-account presence, search widget state
    restoreGeometry(guiConfigGroup.readEntry("window_geometry", QByteArray()));
    toggleSearchWidget(guiConfigGroup.readEntry("pin_filterbar", true));
    if (guiConfigGroup.readEntry("selected_presence_chooser", "global") == QLatin1String("global")) {
        //hide account buttons and show global presence
        onUseGlobalPresenceTriggered();
    }

    m_contextMenu = new ContextMenu(m_contactsListView);
    new ToolTipManager(m_contactsListView);

    connect(m_contactsListView, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(onCustomContextMenuRequested(QPoint)));

    connect(m_groupContactsAction, SIGNAL(triggered(bool)),
            m_contactsListView, SLOT(toggleGroups(bool)));
    connect(m_showOfflineAction, SIGNAL(toggled(bool)),
            m_contactsListView, SLOT(toggleOfflineContacts(bool)));
    connect(m_sortByPresenceAction, SIGNAL(activeChanged(bool)),
            m_contactsListView, SLOT(toggleSortByPresence(bool)));

    connect(m_filterBar, SIGNAL(filterChanged(QString)),
            m_contactsListView, SLOT(setFilterString(QString)));
    connect(m_filterBar, SIGNAL(closeRequest()),
            m_filterBar, SLOT(hide()));
    connect(m_filterBar, SIGNAL(closeRequest()),
            m_searchContactAction, SLOT(trigger()));

    connect(m_contactsListView, SIGNAL(genericOperationFinished(Tp::PendingOperation*)),
            this, SLOT(onGenericOperationFinished(Tp::PendingOperation*)));

    bool useGroups = guiConfigGroup.readEntry("use_groups", true);
    m_groupContactsAction->setChecked(useGroups);
    m_groupContactsAction->setActive(useGroups);

    bool showOffline = guiConfigGroup.readEntry("show_offline", false);
    m_showOfflineAction->setChecked(showOffline);
    m_showOfflineAction->setActive(showOffline);

    bool sortByPresence = guiConfigGroup.readEntry("sort_by_presence", true);
    m_sortByPresenceAction->setActive(sortByPresence);

    m_contactsListView->toggleGroups(useGroups);
    m_contactsListView->toggleOfflineContacts(showOffline);
    m_contactsListView->toggleSortByPresence(sortByPresence);
}

MainWidget::~MainWidget()
{
    //save the state of the filter bar, pinned or not
    KSharedConfigPtr config = KGlobal::config();
    KConfigGroup configGroup(config, "GUI");
    configGroup.writeEntry("pin_filterbar", m_searchContactAction->isChecked());
    configGroup.writeEntry("use_groups", m_groupContactsAction->isChecked());
    configGroup.writeEntry("show_offline", m_showOfflineAction->isChecked());
    configGroup.writeEntry("sort_by_presence", m_sortByPresenceAction->isActive());
    configGroup.config()->sync();
}

void MainWidget::onAccountManagerReady(Tp::PendingOperation* op)
{
    if (op->isError()) {
        kDebug() << op->errorName();
        kDebug() << op->errorMessage();

        KMessageBox::error(this,
                           i18n("Something unexpected happened to the core part of your Instant Messaging system "
                           "and it couldn't be initialized. Try restarting the Contact List."),
                           i18n("IM system failed to initialize"));
        return;
    }

    m_accountButtons->setAccountManager(m_accountManager);
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

    KAboutData aboutData("ktelepathy",0,KLocalizedString(),0);
    notification->setComponentData(KComponentData(aboutData));

    notification->setText(text);
    notification->sendEvent();
}

void MainWidget::onAddContactRequest()
{
    KTp::AddContactDialog *dialog = new KTp::AddContactDialog(m_accountManager, this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}

void MainWidget::onCustomContextMenuRequested(const QPoint &pos)
{
    QModelIndex index = m_contactsListView->indexAt(pos);

    if (!index.isValid()) {
        return;
    }

    KTp::RowType type = (KTp::RowType)index.data(KTp::RowTypeRole).toInt();

    KMenu *menu = 0;

    if (type == KTp::ContactRowType) {
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
    QWeakPointer<KTp::JoinChatRoomDialog> dialog = new KTp::JoinChatRoomDialog(m_accountManager);

    if (dialog.data()->exec() == QDialog::Accepted) {
        Tp::AccountPtr account = dialog.data()->selectedAccount();

        // check account validity. Should NEVER be invalid
        if (!account.isNull()) {
            // ensure chat room
            Tp::PendingChannelRequest *channelRequest = KTp::Actions::startGroupChat(account, dialog.data()->selectedChatRoom());

            connect(channelRequest, SIGNAL(finished(Tp::PendingOperation*)), SLOT(onGenericOperationFinished(Tp::PendingOperation*)));
        }
    }

    delete dialog.data();
}

void MainWidget::onMakeCallRequested()
{
    KToolInvocation::kdeinitExec(QLatin1String("ktp-dialout-ui"));
}

void MainWidget::closeEvent(QCloseEvent* e)
{
    KSharedConfigPtr config = KGlobal::config();
    KConfigGroup generalConfigGroup(config, "General");
    KConfigGroup notifyConigGroup(config, "Notification Messages");
    KConfigGroup guiConfigGroup(config, "GUI");

    ContactListApplication *app = qobject_cast<ContactListApplication*>(kapp);
    if (!app->isShuttingDown()) {
        //the standard KMessageBox control saves "true" if you select the checkbox, therefore the reversed var name
        bool dontCheckForPlasmoid = notifyConigGroup.readEntry("dont_check_for_plasmoid", false);

        if (isAnyAccountOnline() && !dontCheckForPlasmoid) {
            if (!isPresencePlasmoidPresent()) {
                switch (KMessageBox::warningYesNoCancel(this,
                        i18n("You do not have any other presence controls active (a Presence widget for example).\n"
                            "Do you want to stay online or would you rather go offline?"),
                        i18n("No Other Presence Controls Found"),
                        KGuiItem(i18n("Stay Online"), KIcon("user-online")),
                        KGuiItem(i18n("Go Offline"), KIcon("user-offline")),
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
    QDBusReply<bool> serviceRegistered = QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.Telepathy.PresenceAppletActive");

    if (serviceRegistered.isValid() && serviceRegistered.value()) {
        return true;
    } else {
        return false;
    }
}

void MainWidget::goOffline()
{
    //FIXME use global presence
    kDebug() << "Setting all accounts offline...";
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

void MainWidget::onUseGlobalPresenceTriggered()
{
    KSharedConfigPtr config = KGlobal::config();
    KConfigGroup configGroup(config, "GUI");

    m_presenceChooser->show();
    m_accountButtons->hide();

    configGroup.writeEntry("selected_presence_chooser", "global");

    configGroup.config()->sync();
}

void MainWidget::onUsePerAccountPresenceTriggered()
{
    KSharedConfigPtr config = KGlobal::config();
    KConfigGroup configGroup(config, "GUI");

    m_presenceChooser->hide();
    m_accountButtons->show();

    configGroup.writeEntry("selected_presence_chooser", "per-account");

    configGroup.config()->sync();
}

void MainWidget::toggleSearchWidget(bool show)
{
        if(show) {
            m_filterBar->show();
        } else {
            m_contactsListView->setFilterString(QString());
            m_filterBar->clear();
            m_filterBar->hide();
        }
}

void MainWidget::setupGlobalMenu()
{
    // Since our menu is hidden, its shortcuts do not work.
    // Here's a workarond: we must assign global menu's unique
    // items to main window. Since it's always active when an
    // application is active, shortcuts now will work properly.

    m_globalMenu = new KMenuBar(this);
    m_globalMenu->setVisible(false);

    KMenu *contacts = new KMenu(i18n("Contacts"), m_globalMenu);
    contacts->addAction(m_addContactAction);
    contacts->addAction(m_joinChatRoom);
    if (!KStandardDirs::findExe("ktp-dialout-ui").isEmpty()) {
        contacts->addAction(m_makeCall);
    }
    contacts->addAction(m_settingsDialog);
    contacts->addSeparator();
    contacts->addAction(m_quitAction);
    this->addAction(m_quitAction); // Shortcuts workaround.
    m_globalMenu->addMenu(contacts);

    KMenu *view = new KMenu(i18n("View"), m_globalMenu);
    view->addAction(m_groupContactsAction);
    view->addAction(m_showOfflineAction);
    view->addAction(m_sortByPresenceAction);
    view->addSeparator();
    KMenu *view_contactListTypeMenu = new KMenu(i18n("Contact List Type"), view);
    view_contactListTypeMenu->addActions(m_contactListTypeGroup->actions());
    view->addMenu(view_contactListTypeMenu);
    KMenu *view_blockedFilterMenu = new KMenu(i18n("Shown Contacts"), view);
    view_blockedFilterMenu->addActions(m_blockedFilterGroup->actions());
    view->addMenu(view_blockedFilterMenu);
    m_globalMenu->addMenu(view);

    m_globalMenu->addMenu(helpMenu());
}

void MainWidget::setupToolBar()
{
    m_toolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_toolBar->addAction(m_addContactAction);
    m_toolBar->addAction(m_groupContactsAction);
    m_toolBar->addAction(m_showOfflineAction);
    m_toolBar->addAction(m_sortByPresenceAction);
    m_toolBar->addAction(m_searchContactAction);

    QWidget *toolBarSpacer = new QWidget(this);
    toolBarSpacer->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    m_toolBar->addWidget(toolBarSpacer);

    QToolButton *settingsButton = new QToolButton(this);
    settingsButton->setIcon(KIcon("configure"));
    settingsButton->setPopupMode(QToolButton::InstantPopup);

    KMenu *settingsButtonMenu = new KMenu(settingsButton);
    settingsButtonMenu->addAction(m_settingsDialog);

    QActionGroup *delegateTypeGroup = new QActionGroup(this);
    delegateTypeGroup->setExclusive(true);

    KMenu *setDelegateTypeMenu = new KMenu(settingsButtonMenu);
    setDelegateTypeMenu->setTitle(i18n("Contact List Type"));
    setDelegateTypeMenu->addActions(m_contactListTypeGroup->actions());
    settingsButtonMenu->addMenu(setDelegateTypeMenu);

    KMenu *setBlockedFilterMenu = new KMenu(settingsButtonMenu);
    setBlockedFilterMenu->setTitle(i18n("Shown Contacts"));
    setBlockedFilterMenu->addActions(m_blockedFilterGroup->actions());
    settingsButtonMenu->addMenu(setBlockedFilterMenu);

    settingsButtonMenu->addAction(m_joinChatRoom);

    if (!KStandardDirs::findExe("ktp-dialout-ui").isEmpty()) {
        settingsButtonMenu->addAction(m_makeCall);
    }

    settingsButtonMenu->addSeparator();
    settingsButtonMenu->addMenu(helpMenu());

    settingsButton->setMenu(settingsButtonMenu);

    m_toolBar->addWidget(settingsButton);
}

void MainWidget::setupTelepathy()
{
    Tp::registerTypes();
    Tp::AccountFactoryPtr  accountFactory = Tp::AccountFactory::create(QDBusConnection::sessionBus(),
                                                                       Tp::Features() << Tp::Account::FeatureCore
                                                                       << Tp::Account::FeatureAvatar
                                                                       << Tp::Account::FeatureCapabilities
                                                                       << Tp::Account::FeatureProtocolInfo
                                                                       << Tp::Account::FeatureProfile);

    Tp::ConnectionFactoryPtr connectionFactory = KTp::ConnectionFactory::create(QDBusConnection::sessionBus(),
                                                                               Tp::Features() << Tp::Connection::FeatureCore
                                                                               << Tp::Connection::FeatureRosterGroups
                                                                               << Tp::Connection::FeatureRoster
                                                                               << Tp::Connection::FeatureSelfContact);

    Tp::ContactFactoryPtr contactFactory = KTp::ContactFactory::create(Tp::Features()  << Tp::Contact::FeatureAlias
                                                                      << Tp::Contact::FeatureAvatarToken
                                                                      << Tp::Contact::FeatureAvatarData
                                                                      << Tp::Contact::FeatureSimplePresence
                                                                      << Tp::Contact::FeatureCapabilities
                                                                      << Tp::Contact::FeatureClientTypes);

    Tp::ChannelFactoryPtr channelFactory = Tp::ChannelFactory::create(QDBusConnection::sessionBus());
    channelFactory->addFeaturesForTextChats(Tp::Features() << Tp::Channel::FeatureCore << Tp::TextChannel::FeatureMessageQueue);

    m_accountManager = Tp::AccountManager::create(QDBusConnection::sessionBus(),
                                                  accountFactory,
                                                  connectionFactory,
                                                  channelFactory,
                                                  contactFactory);

    connect(m_accountManager->becomeReady(), SIGNAL(finished(Tp::PendingOperation*)),
            this, SLOT(onAccountManagerReady(Tp::PendingOperation*)));
}

KAction *MainWidget::createAction(const QString &text, QObject *signalReceiver, const char *slot, const KIcon &icon = KIcon())
{
    KAction *action = new KAction(icon, text, this);
    connect(action, SIGNAL(triggered(bool)), signalReceiver, slot);
    return action;
}

KAction *MainWidget::createAction(const QString& text, QObject *signalReceiver, const char* slot, bool isChecked, const KIcon& icon = KIcon())
{
    KAction *action = createAction(text, signalReceiver, slot, icon);
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
    m_makeCall = createAction(i18n("Make a Call..."), this, SLOT(onMakeCallRequested()));
    m_addContactAction = createAction(i18n("Add New Contacts..."), this, SLOT(onAddContactRequest()), KIcon("list-add-user"));
    m_searchContactAction = createAction(i18n("Find Contact"), this, SLOT(toggleSearchWidget(bool)),
                                         guiConfigGroup.readEntry("pin_filterbar", true), KIcon("edit-find-user"));
    m_searchContactAction->setShortcut(KStandardShortcut::find());

    // Dual actions
    m_groupContactsAction = new KDualAction(i18n("Show Contacts by Groups"),
                                            i18n("Show Contacts by Accounts"),
                                            this);
    m_groupContactsAction->setActiveIcon(KIcon("user-group-properties"));
    m_groupContactsAction->setInactiveIcon(KIcon("user-group-properties"));
    m_groupContactsAction->setCheckable(true);
    m_groupContactsAction->setChecked(true);

    m_showOfflineAction = new KDualAction(i18n("Show Offline Contacts"),
                                          i18n("Hide Offline Contacts"),
                                          this);
    m_showOfflineAction->setActiveIcon(KIcon("meeting-attending-tentative"));
    m_showOfflineAction->setInactiveIcon(KIcon("meeting-attending-tentative"));
    m_showOfflineAction->setCheckable(true);
    m_showOfflineAction->setChecked(false);
    m_showOfflineAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_S));

    m_sortByPresenceAction = new KDualAction(i18n("Sort by Presence"),
                                             i18n("Sort by Name"),
                                             this);
    m_sortByPresenceAction->setActiveIcon(KIcon("sort-presence"));
    m_sortByPresenceAction->setInactiveIcon(KIcon("sort-name"));

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

#include "main-widget.moc"
