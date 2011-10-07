/*
 * This file is part of telepathy-contact-list
 *
 * Copyright (C) 2009-2010 Collabora Ltd. <info@collabora.co.uk>
 *   @Author George Goldberg <george.goldberg@collabora.co.uk>
 * Copyright (C) 2011 Martin Klapetek <martin.klapetek@gmail.com>
 * Copyright (C) 2011 Keith Rusler <xzekecomax@gmail.com>
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

#ifndef TELEPATHY_CONTACTSLIST_MAIN_WIDGET_H
#define TELEPATHY_CONTACTSLIST_MAIN_WIDGET_H

#include <QtGui/QWidget>
#include <QtGui/QStyledItemDelegate>

#include <TelepathyQt4/AccountManager>

#include <KXmlGuiWindow>
#include <KAction>
#include <KDualAction>
#include "ui_main-widget.h"

class ContactDelegateCompact;
class GroupsModel;
class KMenu;
class KSelectAction;
class AccountsModel;
class AccountsFilterModel;
class ContactDelegate;
class FilterBar;
class KJob;
class ContactModelItem;

class MainWidget : public KMainWindow, Ui::MainWindow
{
    Q_OBJECT
public:
    MainWidget(QWidget *parent = 0);
    ~MainWidget();

    bool isPresencePlasmoidPresent() const;
    bool isAnyAccountOnline() const;
    void addOverlayButtons();

    enum SystemMessageType {
        /*
         * this will show a system message to the user
         * but it will fade after short timout,
         * thus it should be used for non-important messages
         * like "Connecting..." etc.
         */
        SystemMessageInfo,

        /*
         * message with this class will stay visible until user
         * closes it and will have light-red background
         */
        SystemMessageError
    };

public Q_SLOTS:
    void onAddContactRequest();
    void onAddContactRequestFoundContacts(Tp::PendingOperation *operation);
    void onNewAccountAdded(const Tp::AccountPtr &account);
    void toggleSearchWidget(bool show);
    void setCustomPresenceMessage(const QString &message);
    void showSettingsKCM();
    void showMessageToUser(const QString &text, const SystemMessageType type);
    void showInfo(ContactModelItem *contactItem);
    void startTextChannel(ContactModelItem *contactItem);
    void startFileTransferChannel(ContactModelItem *contactItem);
    void startAudioChannel(ContactModelItem *contactItem);
    void startVideoChannel(ContactModelItem *contactItem);
    void startDesktopSharing(ContactModelItem *contactItem);

    void goOffline();

private Q_SLOTS:
    void onAddContactToGroupTriggered();
    void onBlockContactTriggered();
    void onStartTextChatTriggered();
    void onStartAudioChatTriggered();
    void onStartVideoChatTriggered();
    void onStartFileTransferTriggered();
    void onStartDesktopSharingTriggered();
    void onUnblockContactTriggered();
    void onCreateNewGroupTriggered();
    void onRenameGroupTriggered();
    void onDeleteGroupTriggered();
    void onShowInfoTriggered();
    void onDeleteContactTriggered();
    void onJoinChatRoomRequested();         /** join chat room action is triggered */

    void onAccountManagerReady(Tp::PendingOperation *op);
    void onContactListClicked(const QModelIndex &index);
    void onContactListDoubleClicked(const QModelIndex &index);
    void onConnectionChanged(const Tp::ConnectionPtr &connection);
    void onAccountConnectionStatusChanged(Tp::ConnectionStatus status);
    void onCustomContextMenuRequested(const QPoint &point);

    void onAccountsPresenceStatusFiltered();
    void onPresencePublicationRequested(const Tp::Contacts &contacts);
    void onContactManagerStateChanged(Tp::ContactListState state);
    void onContactManagerStateChanged(const Tp::ContactManagerPtr &contactManager, Tp::ContactListState state);
    void onSwitchToFullView();
    void onSwitchToCompactView();
    void onNewGroupModelItemsInserted(const QModelIndex &index, int start, int end);

    void onGenericOperationFinished(Tp::PendingOperation *operation);   /** called when a Tp::PendingOperation finishes. Used to check for errors */
    void onOpenLinkTriggered(QAction *action);                          /** triggered from custom contact menu when user clicks contact link */
    void groupContacts(bool enabled);
    void monitorPresence(const Tp::ConnectionPtr &connection);

Q_SIGNALS:
    void enableOverlays(bool);

private:
    QStringList extractLinksFromIndex(const QModelIndex &index);    /** extract links from a QModelIndex pointing to a contact */
    ///Was moved to telepathy-kded-module
    //void handleConnectionError(const Tp::AccountPtr &account);      /** handle connection errors for given account. This method provides visual notification */
    void closeEvent(QCloseEvent *e);

    KMenu* contactContextMenu(const QModelIndex &index);
    KMenu* groupContextMenu(const QModelIndex &index);

    AccountsModel          *m_model;
    GroupsModel            *m_groupsModel;
    AccountsFilterModel    *m_modelFilter;
    Tp::AccountManagerPtr   m_accountManager;
    KMenu                  *m_accountMenu;
    KSelectAction          *m_setStatusAction;
    ContactDelegate        *m_delegate;
    ContactDelegateCompact *m_compactDelegate;
    KAction                *m_addContactAction;
    KAction                *m_groupContactsAction;
    KAction                *m_showOfflineAction;
    KAction                *m_searchContactAction;
    KDualAction                *m_sortByPresenceAction;
};


#endif // Header guard
