/*
 * This file is part of telepathy-contactslist-prototype
 *
 * Copyright (C) 2009-2010 Collabora Ltd. <info@collabora.co.uk>
 *   @Author George Goldberg <george.goldberg@collabora.co.uk>
 *   @Author Martin Klapetek <martin.klapetek@gmail.com>
 *   @Author Keith Rusler <xzekecomax@gmail.com>
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
#include "ui_main-widget.h"

class ContactDelegateCompact;
class GroupsModel;
class KMenu;
class KSelectAction;
class AccountsModel;
class AccountFilterModel;
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
    void onAccountManagerReady(Tp::PendingOperation *op);

    void onContactListDoubleClick(const QModelIndex &index);
    void onConnectionChanged(const Tp::ConnectionPtr &connection);
    void onAccountConnectionStatusChanged(Tp::ConnectionStatus status);
    void showMessageToUser(const QString &text, const SystemMessageType type);
    void addOverlayButtons();
    void onNewAccountAdded(const Tp::AccountPtr &account);
    void onAccountStateChanged(bool enabled);
    void onAccountRemoved();
    void toggleSearchWidget(bool show);
    void setCustomPresenceMessage(const QString &message);
    void showSettingsKCM();
    void onAddContactRequest();
    void onAddContactRequestFoundContacts(Tp::PendingOperation *operation);
    void loadAvatar(const Tp::AccountPtr &account);
    void selectAvatarFromAccount(const QString &accountUID);
    void selectAvatarFromAccount();
    void loadAvatarFromFile();
    void startTextChannel(ContactModelItem *contactItem);
    void startFileTransferChannel(ContactModelItem *contactItem);
    void startAudioChannel(ContactModelItem *contactItem);
    void startVideoChannel(ContactModelItem *contactItem);
    void onCustomContextMenuRequested(const QPoint &point);
    void onGroupContacts(bool enabled);

private Q_SLOTS:
    void slotAddContactToGroupTriggered();
    void slotBlockContactTriggered();
    void slotDeleteContact();
    void slotGenericOperationFinished(Tp::PendingOperation *operation); /** called when a Tp::PendingOperation finishes. Used to check for errors */
    void slotStartTextChat();
    void slotStartAudioChat();
    void slotStartVideoChat();
    void slotStartFileTransfer();
    void slotUnblockContactTriggered();
    void onAvatarFetched(KJob*);
    void onAccountsPresenceStatusFiltered();
    void onPresencePublicationRequested(const Tp::Contacts &contacts);
    void monitorPresence(const Tp::ConnectionPtr &connection);
    void onContactManagerStateChanged(Tp::ContactListState state);
    void onContactManagerStateChanged(const Tp::ContactManagerPtr &contactManager, Tp::ContactListState state);
    void onSwitchToFullView();
    void onSwitchToCompactView();

Q_SIGNALS:
    void enableOverlays(bool);

private:
    /** handle connection errors for given account. This method provides visual notification */
    void handleConnectionError(const Tp::AccountPtr &account);

    AccountsModel          *m_model;
    GroupsModel            *m_groupsModel;
    AccountFilterModel     *m_modelFilter;
    Tp::AccountManagerPtr   m_accountManager;
    KMenu                  *m_accountMenu;
    KMenu                  *m_avatarButtonMenu;
    KSelectAction          *m_setStatusAction;
    ContactDelegate        *m_delegate;
    ContactDelegateCompact *m_compactDelegate;
    KAction                *m_addContactAction;
    KAction                *m_groupContactsAction;
    KAction                *m_hideOfflineAction;
    KAction                *m_searchContactAction;
    KAction                *m_sortByPresenceAction;
};


#endif // Header guard
