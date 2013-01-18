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
#include <KTp/Widgets/notificationconfigdialog.h>

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
    if (!index.isValid()) {
        return 0;
    }

    KMenu *menu = new KMenu();

//     m_currentIndex = index;
// 
//     const QString groupName = index.data(Qt::DisplayRole).toString();
// 
//     KMenu *menu = new KMenu();
//     menu->addTitle(groupName);
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
    return menu;
}

void ContextMenu::onRemoveContactFromGroupTriggered()
{
//     if (m_currentIndex.parent().data(ContactsModel::TypeRole).toUInt() != ContactsModel::GroupRowType) {
//         return;
//     }
// 
//     const QString groupName = m_currentIndex.parent().data(Qt::DisplayRole).toString();
//     Tp::ContactPtr contact =  m_currentIndex.data(ContactsModel::ContactRole).value<Tp::ContactPtr>();
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
//     Tp::ContactPtr contact  = m_currentIndex.data(ContactsModel::ContactRole).value<Tp::ContactPtr>();
//     if (contact) {
//         QWeakPointer<ContactInfo> contactInfoDialog = new ContactInfo(contact, m_mainWidget);
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
//     Tp::ContactPtr contact = m_currentIndex.data(ContactsModel::ContactRole).value<Tp::ContactPtr>();
//     Tp::AccountPtr account = m_currentIndex.data(ContactsModel::AccountRole).value<Tp::AccountPtr>();
// 
//     if (contact && account) {
//         m_mainWidget->startTextChannel(account, contact);
//     }
}

void ContextMenu::onStartAudioChatTriggered()
{
//     Tp::ContactPtr contact = m_currentIndex.data(ContactsModel::ContactRole).value<Tp::ContactPtr>();
//     Tp::AccountPtr account = m_currentIndex.data(ContactsModel::AccountRole).value<Tp::AccountPtr>();
// 
//     if (contact && account) {
//         m_mainWidget->startAudioChannel(account, contact);
//     }
}

void ContextMenu::onStartVideoChatTriggered()
{
//     if (!m_currentIndex.isValid()) {
//         kDebug() << "Invalid index provided.";
//         return;
//     }
// 
//     Tp::ContactPtr contact = m_currentIndex.data(ContactsModel::ContactRole).value<Tp::ContactPtr>();
//     Tp::AccountPtr account = m_currentIndex.data(ContactsModel::AccountRole).value<Tp::AccountPtr>();
// 
//     if (contact && account) {
//         m_mainWidget->startVideoChannel(account, contact);
//     }
}

void ContextMenu::onStartFileTransferTriggered()
{
//     if (!m_currentIndex.isValid()) {
//         kDebug() << "Invalid index provided.";
//         return;
//     }
// 
//     Tp::ContactPtr contact = m_currentIndex.data(ContactsModel::ContactRole).value<Tp::ContactPtr>();
//     Tp::AccountPtr account = m_currentIndex.data(ContactsModel::AccountRole).value<Tp::AccountPtr>();
// 
//     if (contact && account) {
//         m_mainWidget->startFileTransferChannel(account, contact);
//     }
}

void ContextMenu::onStartDesktopSharingTriggered()
{
//     if (!m_currentIndex.isValid()) {
//         kDebug() << "Invalid index provided.";
//         return;
//     }
// 
//     Tp::ContactPtr contact = m_currentIndex.data(ContactsModel::ContactRole).value<Tp::ContactPtr>();
//     Tp::AccountPtr account = m_currentIndex.data(ContactsModel::AccountRole).value<Tp::AccountPtr>();
// 
//     if (contact && account) {
//         m_mainWidget->startDesktopSharing(account, contact);
//     }
}

void ContextMenu::onOpenLogViewerTriggered()
{
//     if (!m_currentIndex.isValid()) {
//       kDebug() << "Invalid index provided.";
//       return;
//     }
// 
//     Tp::ContactPtr contact = m_currentIndex.data(ContactsModel::ContactRole).value<Tp::ContactPtr>();
//     Tp::AccountPtr account = m_currentIndex.data(ContactsModel::AccountRole).value<Tp::AccountPtr>();
// 
//     if (contact && account) {
//         m_mainWidget->startLogViewer(account, contact);
//     }
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
//     Tp::ContactPtr contact = m_currentIndex.data(ContactsModel::ContactRole).value<Tp::ContactPtr>();
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
//         Tp::ContactPtr contact = m_currentIndex.data(ContactsModel::ContactRole).value<Tp::ContactPtr>();
//         Tp::PendingOperation *operation = contact->addToGroup(newGroupName);
// 
//         connect(operation, SIGNAL(finished(Tp::PendingOperation*)),
//                 m_mainWidget, SIGNAL(genericOperationFinished(Tp::PendingOperation*)));
//     }
}

void ContextMenu::onRenameGroupTriggered()
{
//     if (m_currentIndex.data(ContactsModel::TypeRole).toUInt() != ContactsModel::GroupRowType) {
//         return;
//     }
// 
//     const QString groupName = m_currentIndex.data(Qt::DisplayRole).toString();
//     const QAbstractItemModel *model = m_currentIndex.model();
// 
//     bool ok = false;
// 
//     QString newGroupName = KInputDialog::getText(i18n("New Group Name"),
//                                                  i18n("Please enter the new group name"),
//                                                  groupName,
//                                                  &ok);
// 
//     if (ok && groupName != newGroupName) {
//         //loop through all child indexes of m_currentIndex
//         for(int i = 0; i < model->rowCount(m_currentIndex); i++) {
//             Tp::ContactPtr contact = model->index(i, 0 , m_currentIndex).data(ContactsModel::ContactRole).value<Tp::ContactPtr>();
//             Q_ASSERT(contact);
// 
//             Tp::PendingOperation *operation = contact->addToGroup(newGroupName);
//             connect(operation, SIGNAL(finished(Tp::PendingOperation*)),
//                     m_mainWidget, SIGNAL(genericOperationFinished(Tp::PendingOperation*)));
// 
//             operation = contact->removeFromGroup(groupName);
//             connect(operation, SIGNAL(finished(Tp::PendingOperation*)),
//                     m_mainWidget, SIGNAL(genericOperationFinished(Tp::PendingOperation*)));
//         }
//     }
}

void ContextMenu::onDeleteGroupTriggered()
{
//     if (m_accountManager.isNull() ||
//         (m_currentIndex.data(ContactsModel::TypeRole).toUInt() != ContactsModel::GroupRowType)) {
//         return;
//     }
// 
//     const QString groupName = m_currentIndex.data(Qt::DisplayRole).toString();
//     const QAbstractItemModel *model = m_currentIndex.model();
// 
// 
//     if (KMessageBox::warningContinueCancel(m_mainWidget,
//                                            i18n("Do you really want to remove group %1?\n\n"
//                                                 "Note that all contacts will be moved to group 'Ungrouped'", groupName),
//                                            i18n("Remove Group")) == KMessageBox::Continue) {
// 
//         for(int i = 0; i < model->rowCount(m_currentIndex); i++) {
//             Tp::ContactPtr contact = model->index(i, 0 , m_currentIndex).data(ContactsModel::ContactRole).value<Tp::ContactPtr>();
// 
//             Q_ASSERT(contact);
// 
//             Tp::PendingOperation *operation = contact->removeFromGroup(groupName);
//             connect(operation, SIGNAL(finished(Tp::PendingOperation*)),
//                     m_mainWidget, SIGNAL(genericOperationFinished(Tp::PendingOperation*)));
//         }
// 
//         foreach (const Tp::AccountPtr &account, m_accountManager->allAccounts()) {
//             if (account->connection()) {
//                 Tp::PendingOperation *operation = account->connection()->contactManager()->removeGroup(groupName);
//                 connect(operation, SIGNAL(finished(Tp::PendingOperation*)),
//                         m_mainWidget, SIGNAL(genericOperationFinished(Tp::PendingOperation*)));
//             }
//         }
//     }
}

void ContextMenu::onBlockContactTriggered()
{
//     Tp::ContactPtr contact =  m_currentIndex.data(ContactsModel::ContactRole).value<Tp::ContactPtr>();
// 
//     Tp::PendingOperation *operation = contact->block();
//     connect(operation, SIGNAL(finished(Tp::PendingOperation*)),
//             m_mainWidget, SIGNAL(genericOperationFinished(Tp::PendingOperation*)));
}

void ContextMenu::onDeleteContactTriggered()
{
//     Tp::ContactPtr contact = m_currentIndex.data(ContactsModel::ContactRole).value<Tp::ContactPtr>();
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
//     Tp::ContactPtr contact = m_currentIndex.data(ContactsModel::ContactRole).value<Tp::ContactPtr>();
//     Tp::PendingOperation *op = contact->manager()->requestPresenceSubscription(QList<Tp::ContactPtr>() << contact);
//     connect(op, SIGNAL(finished(Tp::PendingOperation*)),
//             m_mainWidget, SIGNAL(genericOperationFinished(Tp::PendingOperation*)));
}

void ContextMenu::onResendAuthorization()
{
//     Tp::ContactPtr contact = m_currentIndex.data(ContactsModel::ContactRole).value<Tp::ContactPtr>();
//     Tp::PendingOperation *op = contact->manager()->authorizePresencePublication(QList<Tp::ContactPtr>() << contact);
//     connect(op, SIGNAL(finished(Tp::PendingOperation*)),
//             m_mainWidget, SIGNAL(genericOperationFinished(Tp::PendingOperation*)));
}

void ContextMenu::onNotificationConfigureTriggered()
{
//     Tp::ContactPtr contact = m_currentIndex.data(ContactsModel::ContactRole).value<Tp::ContactPtr>();
// 
//     KTp::NotificationConfigDialog *notificationDialog = new KTp::NotificationConfigDialog(contact, m_mainWidget);
//     notificationDialog->show();
}
