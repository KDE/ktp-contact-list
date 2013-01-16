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

#include "dialogs/remove-contact-dialog.h"
#include "dialogs/contact-info.h"

#include "contact-list-widget_p.h"

#include <kpeople/persons-model.h>

ContextMenu::ContextMenu(ContactListWidget *mainWidget)
    : QObject(mainWidget)
{
    m_mainWidget = mainWidget;

//     Tpl::init();
//     m_logManager = Tpl::LogManager::instance();
}


ContextMenu::~ContextMenu()
{

}

KMenu* ContextMenu::contactContextMenu(const QModelIndex &index)
{
    if (!index.isValid()) {
        return 0;
    }

    KMenu *menu = new KMenu();
    menu->addTitle(index.data(Qt::DisplayRole).toString());

    menu->addActions(index.data(PersonsModel::ContactActionsRole).value<QList<QAction*> >());

    return menu;
}

KMenu* ContextMenu::groupContextMenu(const QModelIndex &index)
{
//     if (!index.isValid()) {
//         return 0;
//     }
//
//     m_currentIndex = index;
//
//     GroupsModelItem *groupItem = index.data(AccountsModel::ItemRole).value<GroupsModelItem*>();
//
//     Q_ASSERT(groupItem);
//
    KMenu *menu = new KMenu();
//     menu->addTitle(groupItem->groupName());
//
//     //must be QAction, because menu->addAction returns QAction, otherwise compilation dies horribly
//     QAction *action = menu->addAction(i18n("Rename Group..."));
//     action->setIcon(KIcon("edit-rename"));
//
//     connect(action, SIGNAL(triggered(bool)),
//             this, SLOT(onRenameGroupTriggered()));
//
//     action = menu->addAction(i18n("Delete Group"));
//     action->setIcon(KIcon("edit-delete"));
//
//     connect(action, SIGNAL(triggered(bool)),
//             this, SLOT(onDeleteGroupTriggered()));
//
    return menu;
}

void ContextMenu::onRemoveContactFromGroupTriggered()
{
//     QString groupName = m_currentIndex.parent().data(GroupsModel::GroupNameRole).toString();
//     ContactModelItem *contactItem = m_currentIndex.data(AccountsModel::ItemRole).value<ContactModelItem*>();
//
//     Q_ASSERT(contactItem);
//     Tp::ContactPtr contact =  contactItem->contact();
//
//     Tp::PendingOperation* operation = contact->removeFromGroup(groupName);
//
//     if (operation) {
//         connect(operation, SIGNAL(finished(Tp::PendingOperation*)),
//                 m_mainWidget, SIGNAL(genericOperationFinished(Tp::PendingOperation*)));
//     }
}

void ContextMenu::onOpenLinkTriggered(QAction *action)
{
    KToolInvocation::invokeBrowser(action->data().toString());
}

void ContextMenu::onShowInfoTriggered()
{
//     if (!m_currentIndex.isValid()) {
//         kDebug() << "Invalid index provided.";
//         return;
//     }
//
//     ContactModelItem* item = m_currentIndex.data(AccountsModel::ItemRole).value<ContactModelItem*>();
//     if (item) {
//         QWeakPointer<ContactInfo> contactInfoDialog = new ContactInfo(item->contact(), m_mainWidget);
//         contactInfoDialog.data()->setAttribute(Qt::WA_DeleteOnClose);
//         contactInfoDialog.data()->show();
//     }
}

void ContextMenu::onStartTextChatTriggered()
{
//     if (!m_currentIndex.isValid()) {
//         kDebug() << "Invalid index provided.";
//         return;
//     }
//
//     ContactModelItem* item = m_currentIndex.data(AccountsModel::ItemRole).value<ContactModelItem*>();
//     if (item) {
//         m_mainWidget->startTextChannel(item);
//     }
}

void ContextMenu::onStartAudioChatTriggered()
{
//     if (!m_currentIndex.isValid()) {
//         kDebug() << "Invalid index provided.";
//         return;
//     }
//
//     ContactModelItem* item = m_currentIndex.data(AccountsModel::ItemRole).value<ContactModelItem*>();
//     if (item) {
//         m_mainWidget->startAudioChannel(item);
//     }
}

void ContextMenu::onStartVideoChatTriggered()
{
//     if (!m_currentIndex.isValid()) {
//         kDebug() << "Invalid index provided.";
//         return;
//     }
//
//     ContactModelItem* item = m_currentIndex.data(AccountsModel::ItemRole).value<ContactModelItem*>();
//     if (item) {
//         m_mainWidget->startVideoChannel(item);
//     }
}

void ContextMenu::onStartFileTransferTriggered()
{
//     if (!m_currentIndex.isValid()) {
//         kDebug() << "Invalid index provided.";
//         return;
//     }
//
//     ContactModelItem* item = m_currentIndex.data(AccountsModel::ItemRole).value<ContactModelItem*>();
//     if (item) {
//         m_mainWidget->startFileTransferChannel(item);
//     }
}

void ContextMenu::onStartDesktopSharingTriggered()
{
//     if (!m_currentIndex.isValid()) {
//         kDebug() << "Invalid index provided.";
//         return;
//     }
//
//     ContactModelItem* item = m_currentIndex.data(AccountsModel::ItemRole).value<ContactModelItem*>();
//     if (item) {
//         m_mainWidget->startDesktopSharing(item);
//     }
}

void ContextMenu::onOpenLogViewerTriggered()
{

}

void ContextMenu::onUnblockContactTriggered()
{
//     ContactModelItem* item = m_currentIndex.data(AccountsModel::ItemRole).value<ContactModelItem*>();
//     Q_ASSERT(item);
//
//     Tp::ContactPtr contact = item->contact();
//
//     Tp::PendingOperation *operation = contact->unblock(); //FIXME
//     connect(operation, SIGNAL(finished(Tp::PendingOperation*)),
//             m_mainWidget, SIGNAL(genericOperationFinished(Tp::PendingOperation*)));
}

void ContextMenu::onAddContactToGroupTriggered()
{
//     ContactModelItem* contactItem = m_currentIndex.data(AccountsModel::ItemRole).value<ContactModelItem*>();
//
//     Q_ASSERT(contactItem);
//     Tp::ContactPtr contact =  contactItem->contact();
//
//     QAction *action = qobject_cast<QAction*>(sender());
//     if (!action) {
//         kDebug() << "Invalid action";
//         return;
//     }
//
//     const QStringList currentGroups = contact->groups();
//
//     Tp::PendingOperation* operation = contact->addToGroup(action->text().remove('&'));
//
//     if (operation) {
//         connect(operation, SIGNAL(finished(Tp::PendingOperation*)),
//                 m_mainWidget, SIGNAL(genericOperationFinished(Tp::PendingOperation*)));
//
//         foreach (const QString &group, currentGroups) {
//             Tp::PendingOperation* operation = contact->removeFromGroup(group);
//             connect(operation, SIGNAL(finished(Tp::PendingOperation*)),
//                     m_mainWidget, SIGNAL(genericOperationFinished(Tp::PendingOperation*)));
//         }
//     }
}

void ContextMenu::onCreateNewGroupTriggered()
{
//     bool ok = false;
//
//     QString newGroupName = KInputDialog::getText(i18n("New Group Name"),
//                                                  i18n("Please enter the new group name"),
//                                                  QString(),
//                                                  &ok);
//
//     if (ok) {
//         ContactModelItem *contactItem = m_currentIndex.data(AccountsModel::ItemRole).value<ContactModelItem*>();
//
//         Q_ASSERT(contactItem);
//         Tp::ContactPtr contact =  contactItem->contact();
//         Tp::PendingOperation *operation = contact->addToGroup(newGroupName);
//
//         connect(operation, SIGNAL(finished(Tp::PendingOperation*)),
//                 m_mainWidget, SIGNAL(genericOperationFinished(Tp::PendingOperation*)));
//     }
}

void ContextMenu::onRenameGroupTriggered()
{
//     GroupsModelItem *groupItem = m_currentIndex.data(AccountsModel::ItemRole).value<GroupsModelItem*>();
//
//     Q_ASSERT(groupItem);
//
//     bool ok = false;
//
//     QString newGroupName = KInputDialog::getText(i18n("New Group Name"),
//                                                  i18n("Please enter the new group name"),
//                                                  groupItem->groupName(),
//                                                  &ok);
//
//     if (ok) {
//         for(int i = 0; i < groupItem->size(); i++) {
//             Tp::ContactPtr contact = qobject_cast<ProxyTreeNode*>(groupItem->childAt(i))->data(AccountsModel::ItemRole).value<ContactModelItem*>()->contact();
//             Q_ASSERT(contact);
//
//             Tp::PendingOperation *operation = contact->addToGroup(newGroupName);
//             connect(operation, SIGNAL(finished(Tp::PendingOperation*)),
//                     m_mainWidget, SIGNAL(genericOperationFinished(Tp::PendingOperation*)));
//
//             operation = contact->removeFromGroup(groupItem->groupName());
//             connect(operation, SIGNAL(finished(Tp::PendingOperation*)),
//                     m_mainWidget, SIGNAL(genericOperationFinished(Tp::PendingOperation*)));
//         }
//     }
}

void ContextMenu::onDeleteGroupTriggered()
{
//     if (m_accountManager.isNull()) {
//         return;
//     }
//
//     GroupsModelItem *groupItem = m_currentIndex.data(AccountsModel::ItemRole).value<GroupsModelItem*>();
//
//     if (KMessageBox::warningContinueCancel(m_mainWidget,
//                                            i18n("Do you really want to remove group %1?\n\n"
//                                                 "Note that all contacts will be moved to group 'Ungrouped'", groupItem->groupName()),
//                                            i18n("Remove Group")) == KMessageBox::Continue) {
//
//         for(int i = 0; i < groupItem->size(); i++) {
//             Tp::ContactPtr contact = qobject_cast<ProxyTreeNode*>(groupItem->childAt(i))->data(AccountsModel::ItemRole).value<ContactModelItem*>()->contact();
//
//             Q_ASSERT(contact);
//
//             Tp::PendingOperation *operation = contact->removeFromGroup(groupItem->groupName());
//             connect(operation, SIGNAL(finished(Tp::PendingOperation*)),
//                     m_mainWidget, SIGNAL(genericOperationFinished(Tp::PendingOperation*)));
//         }
//
//         foreach (const Tp::AccountPtr &account, m_accountManager->allAccounts()) {
//             if (account->connection()) {
//                 Tp::PendingOperation *operation = account->connection()->contactManager()->removeGroup(groupItem->groupName());
//                 connect(operation, SIGNAL(finished(Tp::PendingOperation*)),
//                         m_mainWidget, SIGNAL(genericOperationFinished(Tp::PendingOperation*)));
//             }
//         }
//     }
}

void ContextMenu::onBlockContactTriggered()
{
//     ContactModelItem *contactItem = m_currentIndex.data(AccountsModel::ItemRole).value<ContactModelItem*>();
//
//     Q_ASSERT(contactItem);
//     Tp::ContactPtr contact =  contactItem->contact();
//
//     Tp::PendingOperation *operation = contact->block();
//     connect(operation, SIGNAL(finished(Tp::PendingOperation*)),
//             m_mainWidget, SIGNAL(genericOperationFinished(Tp::PendingOperation*)));
}

void ContextMenu::onDeleteContactTriggered()
{
//     ContactModelItem* contactItem = m_currentIndex.data(AccountsModel::ItemRole).value<ContactModelItem*>();
//
//     Q_ASSERT(contactItem);
//     Tp::ContactPtr contact =  contactItem->contact();
//
//     QList<Tp::ContactPtr>contactList;
//     contactList.append(contact);
//
//     // ask for confirmation
//     QWeakPointer<RemoveContactDialog> removeDialog = new RemoveContactDialog(contact, m_mainWidget);
//
//     if (removeDialog.data()->exec() == QDialog::Accepted) {
//         if (!removeDialog.isNull()) {
//             // remove from contact list
//             Tp::PendingOperation *deleteOp = contact->manager()->removeContacts(contactList);
//             connect(deleteOp, SIGNAL(finished(Tp::PendingOperation*)),
//                     m_mainWidget, SIGNAL(genericOperationFinished(Tp::PendingOperation*)));
//
//             if (removeDialog.data()->blockContact()) {
//                 // block contact
//                 Tp::PendingOperation *blockOp = contact->manager()->blockContacts(contactList);
//                 connect(blockOp, SIGNAL(finished(Tp::PendingOperation*)),
//                         m_mainWidget, SIGNAL(genericOperationFinished(Tp::PendingOperation*)));
//             }
//         }
//     }
//
//     delete removeDialog.data();
}

void ContextMenu::onRerequestAuthorization()
{
//     ContactModelItem* contactItem = m_currentIndex.data(AccountsModel::ItemRole).value<ContactModelItem*>();
//     Tp::PendingOperation *op = contactItem->contact()->manager()->requestPresenceSubscription(QList<Tp::ContactPtr>() << contactItem->contact());
//     connect(op, SIGNAL(finished(Tp::PendingOperation*)),
//             m_mainWidget, SIGNAL(genericOperationFinished(Tp::PendingOperation*)));
}

void ContextMenu::onResendAuthorization()
{
//     ContactModelItem* contactItem = m_currentIndex.data(AccountsModel::ItemRole).value<ContactModelItem*>();
//     Tp::PendingOperation *op = contactItem->contact()->manager()->authorizePresencePublication(QList<Tp::ContactPtr>() << contactItem->contact());
//     connect(op, SIGNAL(finished(Tp::PendingOperation*)),
//             m_mainWidget, SIGNAL(genericOperationFinished(Tp::PendingOperation*)));
}

void ContextMenu::onNotificationConfigureTriggered()
{
//     ContactModelItem *contactItem = m_currentIndex.data(ContactsModel::ItemRole).value<ContactModelItem*>();
// 
//     Q_ASSERT(contactItem);
//     Tp::ContactPtr contact = contactItem->contact();
// 
//     KTp::NotificationConfigDialog *notificationDialog = new KTp::NotificationConfigDialog(contact, m_mainWidget);
//     notificationDialog->show();
}
