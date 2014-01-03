/*
    Contact list context menu
    Copyright (C) 2011  Martin Klapetek <martin.klapetek@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "context-menu.h"

#include <KDebug>

#include <KMenu>
#include <KLocalizedString>
#include <KIcon>
#include <KToolInvocation>
#include <KInputDialog>
#include <KMessageBox>
#include <KAction>


#include <KTp/text-parser.h>
#include <KTp/Widgets/notification-config-dialog.h>
#include <KTp/contact-info-dialog.h>
#include <KTp/types.h>
#include <KTp/Models/contacts-model.h>
#include <KTp/Logger/log-manager.h>
#include <KTp/Logger/log-entity.h>

#include <TelepathyQt/ContactManager>
#include <TelepathyQt/Account>
#include <TelepathyQt/PendingOperation>

#ifdef HAVE_KPEOPLE
#include <kpeople/widgets/persondetailsdialog.h>
#include <kpeople/global.h>
#include <kpeople/personsmodel.h>
#include <kpeople/persondata.h>
#endif

#include "dialogs/remove-contact-dialog.h"

#include "contact-list-widget_p.h"

ContextMenu::ContextMenu(ContactListWidget *mainWidget)
    : QObject(mainWidget)
{
    m_mainWidget = mainWidget;
}


ContextMenu::~ContextMenu()
{

}

void ContextMenu::setAccountManager(const Tp::AccountManagerPtr &accountManager)
{
    m_accountManager = accountManager;
    KTp::LogManager::instance()->setAccountManager(accountManager);
}

KMenu* ContextMenu::contactContextMenu(const QModelIndex &index)
{
    if (!index.isValid()) {
        return 0;
    }

    if (m_accountManager.isNull()) {
        return 0;
    }

    m_currentIndex = index;

    KTp::ContactPtr contact = index.data(KTp::ContactRole).value<KTp::ContactPtr>();

    if (contact.isNull()) {
        kDebug() << "Contact is nulled";
        return 0;
    }

    Tp::AccountPtr account = index.data(KTp::AccountRole).value<Tp::AccountPtr>();

    if (account.isNull()) {
        kDebug() << "Account is nulled";
        return 0;
    }

    KMenu *menu = new KMenu();
    menu->addTitle(contact->alias());

    QAction *action;

    if (KTp::kpeopleEnabled()) {
    #ifdef HAVE_KPEOPLE
        if (index.parent().isValid()) {
            menu->addActions(KPeople::actionsForPerson(index.data(KTp::ContactVCardRole).value<KABC::Addressee>(), KABC::AddresseeList(), menu));
        } else {
            KPeople::PersonData p(index.data(KTp::PersonIdRole).toString());
            menu->addActions(KPeople::actionsForPerson(p.person(), p.contacts(), menu));
        }
    #endif
    } else {
        //must be a QAction because menu->addAction returns QAction, breaks compilation otherwise
        action = menu->addAction(i18n("Start Chat..."));
        action->setIcon(KIcon("text-x-generic"));
        action->setDisabled(true);
        connect(action, SIGNAL(triggered(bool)),
                SLOT(onStartTextChatTriggered()));

        if (index.data(KTp::ContactCanTextChatRole).toBool()) {
            action->setEnabled(true);
        }

        action = menu->addAction(i18n("Start Audio Call..."));
        action->setIcon(KIcon("audio-headset"));
        action->setDisabled(true);
        connect(action, SIGNAL(triggered(bool)),
                SLOT(onStartAudioChatTriggered()));

        if (index.data(KTp::ContactCanAudioCallRole).toBool()) {
            action->setEnabled(true);
        }

        action = menu->addAction(i18n("Start Video Call..."));
        action->setIcon(KIcon("camera-web"));
        action->setDisabled(true);
        connect(action, SIGNAL(triggered(bool)),
                SLOT(onStartVideoChatTriggered()));

        if (index.data(KTp::ContactCanVideoCallRole).toBool()) {
            action->setEnabled(true);
        }

        action = menu->addAction(i18n("Send File..."));
        action->setIcon(KIcon("mail-attachment"));
        action->setDisabled(true);
        connect(action, SIGNAL(triggered(bool)),
                SLOT(onStartFileTransferTriggered()));

        if (index.data(KTp::ContactCanFileTransferRole).toBool()) {
            action->setEnabled(true);
        }

        action = menu->addAction(i18n("Share my desktop..."));
        action->setIcon(KIcon("krfb"));
        action->setDisabled(true);
        connect(action, SIGNAL(triggered(bool)),
                SLOT(onStartDesktopSharingTriggered()));

        if (index.data(KTp::ContactTubesRole).toStringList().contains(QLatin1String("rfb"))) {
            action->setEnabled(true);
        }

        action = menu->addAction(i18n("Open Log Viewer..."));
        action->setIcon(KIcon("documentation"));
        action->setDisabled(true);
        connect(action, SIGNAL(triggered(bool)),
                SLOT(onOpenLogViewerTriggered()));

        KTp::LogEntity entity(Tp::HandleTypeContact, contact->id());
        if (KTp::LogManager::instance()->logsExist(account, entity)) {
            action->setEnabled(true);
        }
    }

    menu->addSeparator();
    action = menu->addAction(KIcon("dialog-information"), i18n("Configure Notifications..."));
    action->setEnabled(true);
    connect(action, SIGNAL(triggered()),
                           SLOT(onNotificationConfigureTriggered()));

    // add "goto" submenu for navigating to links the contact has in presence message
    // first check to see if there are any links in the contact's presence message
    QStringList contactLinks;
    QString presenceMsg = index.data(KTp::ContactPresenceMessageRole).toString();
    if (presenceMsg.isEmpty()) {
        contactLinks = QStringList();
    } else {
        KTp::TextUrlData urls = KTp::TextParser::instance()->extractUrlData(presenceMsg);
        contactLinks = urls.fixedUrls;
    }

    if (!contactLinks.empty()) {
        KMenu *subMenu = new KMenu(i18np("Presence message link", "Presence message links", contactLinks.count()));

        foreach(const QString &link, contactLinks) {
            action = subMenu->addAction(link);
            action->setData(link);
        }
        connect(subMenu, SIGNAL(triggered(QAction*)), this, SLOT(onOpenLinkTriggered(QAction*)));
        menu->addMenu(subMenu);
    }

    menu->addSeparator();

    Tp::ConnectionPtr accountConnection = account->connection();
    if (accountConnection.isNull()) {
        kDebug() << "Account connection is nulled.";
        return 0;
    }

    if (m_mainWidget->d_ptr->model->groupMode() == KTp::ContactsModel::GroupGrouping) {
        // remove contact from group action, must be QAction because menu->addAction returns QAction
        QAction *groupRemoveAction = menu->addAction(KIcon(), i18n("Remove Contact From This Group"));
        connect(groupRemoveAction, SIGNAL(triggered(bool)), this, SLOT(onRemoveContactFromGroupTriggered()));

        if (accountConnection->actualFeatures().contains(Tp::Connection::FeatureRosterGroups)) {
            QMenu* groupAddMenu = menu->addMenu(i18n("Move to Group"));

            QStringList groupList;
            QList<Tp::AccountPtr> accounts = m_accountManager->allAccounts();
            foreach (const Tp::AccountPtr &account, accounts) {
                if (!account->connection().isNull()) {
                    groupList.append(account->connection()->contactManager()->allKnownGroups());
                }
            }

            groupList.removeDuplicates();

            QStringList currentGroups = contact->groups();

            foreach (const QString &group, currentGroups) {
                groupList.removeAll(group);
            }

            connect(groupAddMenu->addAction(i18n("Create New Group...")), SIGNAL(triggered(bool)),
                    this, SLOT(onCreateNewGroupTriggered()));

            groupAddMenu->addSeparator();

            foreach (const QString &group, groupList) {
                connect(groupAddMenu->addAction(group), SIGNAL(triggered(bool)),
                        SLOT(onAddContactToGroupTriggered()));
            }
        } else {
            kDebug() << "Unable to support Groups";
        }
    }

    menu->addSeparator();


    if (contact->manager()->canRequestPresenceSubscription()) {
        if (contact->subscriptionState() != Tp::Contact::PresenceStateYes) {
            action = menu->addAction(i18n("Re-request Contact Authorization"));
            connect(action, SIGNAL(triggered(bool)), SLOT(onRerequestAuthorization()));
        }
    }
    if (contact->manager()->canAuthorizePresencePublication()) {
        if (contact->publishState() != Tp::Contact::PresenceStateYes) {
            action = menu->addAction(i18n("Resend Contact Authorization"));
            connect(action, SIGNAL(triggered(bool)), SLOT(onResendAuthorization()));
        }
    }

    action = menu->addSeparator(); //prevent two seperators in a row

    if (contact->isBlocked()) {
        action = menu->addAction(i18n("Unblock Contact"));
        connect(action, SIGNAL(triggered(bool)), SLOT(onUnblockContactTriggered()));
        action->setEnabled(contact->manager()->canBlockContacts());
    } else {
        action = menu->addAction(i18n("Block Contact"));
        connect(action, SIGNAL(triggered(bool)), SLOT(onBlockContactTriggered()));
        action->setEnabled(contact->manager()->canBlockContacts());
    }

    // remove contact action, must be QAction because that's what menu->addAction returns

    //TODO find an "if canRemove"
    QAction *removeAction = menu->addAction(KIcon("list-remove-user"), i18n("Remove Contact"));
    connect(removeAction, SIGNAL(triggered(bool)), this, SLOT(onDeleteContactTriggered()));

    menu->addSeparator();

    action = menu->addAction(i18n("Show Info..."));
    action->setIcon(KIcon(""));
    connect(action, SIGNAL(triggered()), SLOT(onShowInfoTriggered()));

    return menu;
}

KMenu* ContextMenu::groupContextMenu(const QModelIndex &index)
{
    if (!index.isValid()) {
        return 0;
    }

    m_currentIndex = index;

    const QString groupName = index.data(Qt::DisplayRole).toString();

    KMenu *menu = new KMenu();
    menu->addTitle(groupName);

    //must be QAction, because menu->addAction returns QAction, otherwise compilation dies horribly
    QAction *action = menu->addAction(i18n("Rename Group..."));
    action->setIcon(KIcon("edit-rename"));

    connect(action, SIGNAL(triggered(bool)),
            this, SLOT(onRenameGroupTriggered()));

    action = menu->addAction(i18n("Delete Group"));
    action->setIcon(KIcon("edit-delete"));

    connect(action, SIGNAL(triggered(bool)),
            this, SLOT(onDeleteGroupTriggered()));

    return menu;
}

void ContextMenu::onRemoveContactFromGroupTriggered()
{
    if (m_currentIndex.parent().data(KTp::RowTypeRole).toUInt() != KTp::GroupRowType) {
        return;
    }

    const QString groupName = m_currentIndex.parent().data(Qt::DisplayRole).toString();
    Tp::ContactPtr contact =  m_currentIndex.data(KTp::ContactRole).value<KTp::ContactPtr>();

    Tp::PendingOperation *operation = contact->removeFromGroup(groupName);

    if (operation) {
        connect(operation, SIGNAL(finished(Tp::PendingOperation*)),
                m_mainWidget, SIGNAL(genericOperationFinished(Tp::PendingOperation*)));
    }
}

void ContextMenu::onOpenLinkTriggered(QAction *action)
{
    KToolInvocation::invokeBrowser(action->data().toString());
}

void ContextMenu::onShowInfoTriggered()
{
    if (!m_currentIndex.isValid()) {
        kDebug() << "Invalid index provided.";
        return;
    }

    if (KTp::kpeopleEnabled()) {
    #ifdef HAVE_KPEOPLE
        const QString &personId = m_currentIndex.data(KTp::PersonIdRole).toString();
        if (!personId.isEmpty()) {
            KPeople::PersonDetailsDialog *view = new KPeople::PersonDetailsDialog(m_mainWidget);
            KPeople::PersonData *person = new KPeople::PersonData(personId, view);
            view->setPerson(person);
            view->setAttribute(Qt::WA_DeleteOnClose);
            view->show();
        }
    #endif
    } else {
        const Tp::AccountPtr &account = m_currentIndex.data(KTp::AccountRole).value<Tp::AccountPtr>();
        const Tp::ContactPtr &contact = m_currentIndex.data(KTp::ContactRole).value<KTp::ContactPtr>();
        if (account && contact) {
            KTp::ContactInfoDialog* contactInfoDialog = new KTp::ContactInfoDialog(account, contact, m_mainWidget);
            contactInfoDialog->setAttribute(Qt::WA_DeleteOnClose);
            contactInfoDialog->show();
        }
    }
}

void ContextMenu::onStartTextChatTriggered()
{
    if (!m_currentIndex.isValid()) {
        kDebug() << "Invalid index provided.";
        return;
    }

    Tp::ContactPtr contact = m_currentIndex.data(KTp::ContactRole).value<KTp::ContactPtr>();
    Tp::AccountPtr account = m_currentIndex.data(KTp::AccountRole).value<Tp::AccountPtr>();

    if (contact && account) {
        m_mainWidget->startTextChannel(account, contact);
    }
}

void ContextMenu::onStartAudioChatTriggered()
{
    Tp::ContactPtr contact = m_currentIndex.data(KTp::ContactRole).value<KTp::ContactPtr>();
    Tp::AccountPtr account = m_currentIndex.data(KTp::AccountRole).value<Tp::AccountPtr>();

    if (contact && account) {
        m_mainWidget->startAudioChannel(account, contact);
    }
}

void ContextMenu::onStartVideoChatTriggered()
{
    if (!m_currentIndex.isValid()) {
        kDebug() << "Invalid index provided.";
        return;
    }

    Tp::ContactPtr contact = m_currentIndex.data(KTp::ContactRole).value<KTp::ContactPtr>();
    Tp::AccountPtr account = m_currentIndex.data(KTp::AccountRole).value<Tp::AccountPtr>();

    if (contact && account) {
        m_mainWidget->startVideoChannel(account, contact);
    }
}

void ContextMenu::onStartFileTransferTriggered()
{
    if (!m_currentIndex.isValid()) {
        kDebug() << "Invalid index provided.";
        return;
    }

    Tp::ContactPtr contact = m_currentIndex.data(KTp::ContactRole).value<KTp::ContactPtr>();
    Tp::AccountPtr account = m_currentIndex.data(KTp::AccountRole).value<Tp::AccountPtr>();

    if (contact && account) {
        m_mainWidget->startFileTransferChannel(account, contact);
    }
}

void ContextMenu::onStartDesktopSharingTriggered()
{
    if (!m_currentIndex.isValid()) {
        kDebug() << "Invalid index provided.";
        return;
    }

    Tp::ContactPtr contact = m_currentIndex.data(KTp::ContactRole).value<KTp::ContactPtr>();
    Tp::AccountPtr account = m_currentIndex.data(KTp::AccountRole).value<Tp::AccountPtr>();

    if (contact && account) {
        m_mainWidget->startDesktopSharing(account, contact);
    }
}

void ContextMenu::onOpenLogViewerTriggered()
{
    if (!m_currentIndex.isValid()) {
      kDebug() << "Invalid index provided.";
      return;
    }

    Tp::ContactPtr contact = m_currentIndex.data(KTp::ContactRole).value<KTp::ContactPtr>();
    Tp::AccountPtr account = m_currentIndex.data(KTp::AccountRole).value<Tp::AccountPtr>();

    if (contact && account) {
        m_mainWidget->startLogViewer(account, contact);
    }
}

void ContextMenu::onUnblockContactTriggered()
{
    Tp::ContactPtr contact = m_currentIndex.data(KTp::ContactRole).value<KTp::ContactPtr>();

    Tp::PendingOperation *operation = contact->unblock(); //FIXME
    connect(operation, SIGNAL(finished(Tp::PendingOperation*)),
            m_mainWidget, SIGNAL(genericOperationFinished(Tp::PendingOperation*)));
}

void ContextMenu::onAddContactToGroupTriggered()
{
    Tp::ContactPtr contact = m_currentIndex.data(KTp::ContactRole).value<KTp::ContactPtr>();

    QAction *action = qobject_cast<QAction*>(sender());
    if (!action) {
        kDebug() << "Invalid action";
        return;
    }

    const QStringList currentGroups = contact->groups();

    Tp::PendingOperation* operation = contact->addToGroup(action->text().remove('&'));

    if (operation) {
        connect(operation, SIGNAL(finished(Tp::PendingOperation*)),
                m_mainWidget, SIGNAL(genericOperationFinished(Tp::PendingOperation*)));

        foreach (const QString &group, currentGroups) {
            Tp::PendingOperation* operation = contact->removeFromGroup(group);
            connect(operation, SIGNAL(finished(Tp::PendingOperation*)),
                    m_mainWidget, SIGNAL(genericOperationFinished(Tp::PendingOperation*)));
        }
    }
}

void ContextMenu::onCreateNewGroupTriggered()
{
    bool ok = false;

    QString newGroupName = KInputDialog::getText(i18n("New Group Name"),
                                                 i18n("Please enter the new group name"),
                                                 QString(),
                                                 &ok);

    if (ok) {
        Tp::ContactPtr contact = m_currentIndex.data(KTp::ContactRole).value<KTp::ContactPtr>();
        Tp::PendingOperation *operation = contact->addToGroup(newGroupName);

        connect(operation, SIGNAL(finished(Tp::PendingOperation*)),
                m_mainWidget, SIGNAL(genericOperationFinished(Tp::PendingOperation*)));
    }
}

void ContextMenu::onRenameGroupTriggered()
{
    if (m_currentIndex.data(KTp::RowTypeRole).toUInt() != KTp::GroupRowType) {
        return;
    }

    const QString groupName = m_currentIndex.data(Qt::DisplayRole).toString();
    const QAbstractItemModel *model = m_currentIndex.model();

    bool ok = false;

    QString newGroupName = KInputDialog::getText(i18n("New Group Name"),
                                                 i18n("Please enter the new group name"),
                                                 groupName,
                                                 &ok);

    if (ok && groupName != newGroupName) {
        //loop through all child indexes of m_currentIndex
        for(int i = 0; i < model->rowCount(m_currentIndex); i++) {
            Tp::ContactPtr contact = model->index(i, 0 , m_currentIndex).data(KTp::ContactRole).value<KTp::ContactPtr>();
            Q_ASSERT(contact);

            Tp::PendingOperation *operation = contact->addToGroup(newGroupName);
            connect(operation, SIGNAL(finished(Tp::PendingOperation*)),
                    m_mainWidget, SIGNAL(genericOperationFinished(Tp::PendingOperation*)));

            operation = contact->removeFromGroup(groupName);
            connect(operation, SIGNAL(finished(Tp::PendingOperation*)),
                    m_mainWidget, SIGNAL(genericOperationFinished(Tp::PendingOperation*)));
        }
    }
}

void ContextMenu::onDeleteGroupTriggered()
{
    if (m_accountManager.isNull() ||
        (m_currentIndex.data(KTp::RowTypeRole).toUInt() != KTp::GroupRowType)) {
        return;
    }

    const QString groupName = m_currentIndex.data(Qt::DisplayRole).toString();
    const QAbstractItemModel *model = m_currentIndex.model();


    if (KMessageBox::warningContinueCancel(m_mainWidget,
                                           i18n("Do you really want to remove group %1?\n\n"
                                                "Note that all contacts will be moved to group 'Ungrouped'", groupName),
                                           i18n("Remove Group")) == KMessageBox::Continue) {

        for(int i = 0; i < model->rowCount(m_currentIndex); i++) {
            Tp::ContactPtr contact = model->index(i, 0 , m_currentIndex).data(KTp::ContactRole).value<KTp::ContactPtr>();

            Q_ASSERT(contact);

            Tp::PendingOperation *operation = contact->removeFromGroup(groupName);
            connect(operation, SIGNAL(finished(Tp::PendingOperation*)),
                    m_mainWidget, SIGNAL(genericOperationFinished(Tp::PendingOperation*)));
        }

        foreach (const Tp::AccountPtr &account, m_accountManager->allAccounts()) {
            if (account->connection()) {
                Tp::PendingOperation *operation = account->connection()->contactManager()->removeGroup(groupName);
                connect(operation, SIGNAL(finished(Tp::PendingOperation*)),
                        m_mainWidget, SIGNAL(genericOperationFinished(Tp::PendingOperation*)));
            }
        }
    }
}

void ContextMenu::onBlockContactTriggered()
{
    Tp::ContactPtr contact =  m_currentIndex.data(KTp::ContactRole).value<KTp::ContactPtr>();

    Tp::PendingOperation *operation = contact->block();
    connect(operation, SIGNAL(finished(Tp::PendingOperation*)),
            m_mainWidget, SIGNAL(genericOperationFinished(Tp::PendingOperation*)));
}

void ContextMenu::onDeleteContactTriggered()
{
    Tp::ContactPtr contact = m_currentIndex.data(KTp::ContactRole).value<KTp::ContactPtr>();

    QList<Tp::ContactPtr>contactList;
    contactList.append(contact);

    // ask for confirmation
    QWeakPointer<RemoveContactDialog> removeDialog = new RemoveContactDialog(contact, m_mainWidget);

    if (removeDialog.data()->exec() == QDialog::Accepted) {
        if (!removeDialog.isNull()) {
            // remove from contact list
            Tp::PendingOperation *deleteOp = contact->manager()->removeContacts(contactList);
            connect(deleteOp, SIGNAL(finished(Tp::PendingOperation*)),
                    m_mainWidget, SIGNAL(genericOperationFinished(Tp::PendingOperation*)));

            if (removeDialog.data()->blockContact()) {
                // block contact
                Tp::PendingOperation *blockOp = contact->manager()->blockContacts(contactList);
                connect(blockOp, SIGNAL(finished(Tp::PendingOperation*)),
                        m_mainWidget, SIGNAL(genericOperationFinished(Tp::PendingOperation*)));
            }
        }
    }

    delete removeDialog.data();
}

void ContextMenu::onRerequestAuthorization()
{
    Tp::ContactPtr contact = m_currentIndex.data(KTp::ContactRole).value<KTp::ContactPtr>();
    Tp::PendingOperation *op = contact->manager()->requestPresenceSubscription(QList<Tp::ContactPtr>() << contact);
    connect(op, SIGNAL(finished(Tp::PendingOperation*)),
            m_mainWidget, SIGNAL(genericOperationFinished(Tp::PendingOperation*)));
}

void ContextMenu::onResendAuthorization()
{
    Tp::ContactPtr contact = m_currentIndex.data(KTp::ContactRole).value<KTp::ContactPtr>();
    Tp::PendingOperation *op = contact->manager()->authorizePresencePublication(QList<Tp::ContactPtr>() << contact);
    connect(op, SIGNAL(finished(Tp::PendingOperation*)),
            m_mainWidget, SIGNAL(genericOperationFinished(Tp::PendingOperation*)));
}

void ContextMenu::onNotificationConfigureTriggered()
{
    Tp::ContactPtr contact = m_currentIndex.data(KTp::ContactRole).value<KTp::ContactPtr>();

    KTp::NotificationConfigDialog *notificationDialog = new KTp::NotificationConfigDialog(contact, m_mainWidget);
    notificationDialog->show();
}
