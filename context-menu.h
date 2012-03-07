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

#ifndef CONTEXT_MENU_H
#define CONTEXT_MENU_H

#include <QObject>
#include <QModelIndex>
#include <QStringList>

#include <TelepathyQt/Types>

#include "contact-list-widget.h"

class AccountsModel;
class ContactModelItem;
class KMenu;
class QAction;

class ContextMenu : public QObject
{
    Q_OBJECT
public:
    explicit ContextMenu(ContactListWidget *mainWidget);
    virtual ~ContextMenu();

    KMenu* groupContextMenu(const QModelIndex &index);
    KMenu* contactContextMenu(const QModelIndex &index);
    void setAccountManager(const Tp::AccountManagerPtr &accountManager);

private Q_SLOTS:
    void onAddContactToGroupTriggered();
    void onBlockContactTriggered();
    void onStartTextChatTriggered();
    void onStartAudioChatTriggered();
    void onStartVideoChatTriggered();
    void onStartFileTransferTriggered();
    void onStartDesktopSharingTriggered();
    void onUnblockContactTriggered();
    void onRemoveContactFromGroupTriggered();
    void onCreateNewGroupTriggered();
    void onRenameGroupTriggered();
    void onDeleteGroupTriggered();
    void onShowInfoTriggered();
    void onDeleteContactTriggered();
    void onOpenLinkTriggered(QAction *action);      /** triggered from custom contact menu when user clicks contact link */
    void onRerequestAuthorization();
    void onResendAuthorization();

private:
    ContactListWidget     *m_mainWidget;
    QModelIndex            m_currentIndex;
    Tp::AccountManagerPtr  m_accountManager;
};

#endif // CONTEXT_MENU_H

class QTreeView;
