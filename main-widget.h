/*
 * This file is part of telepathy-contactslist-prototype
 *
 * Copyright (C) 2009-2010 Collabora Ltd. <info@collabora.co.uk>
 *   @Author George Goldberg <george.goldberg@collabora.co.uk>
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

#ifndef TELEPATHY_CONTACTSLIST_PROTOTYPE_MAIN_WIDGET_H
#define TELEPATHY_CONTACTSLIST_PROTOTYPE_MAIN_WIDGET_H

#include "ui_main-widget.h"

#include "fakecontactsmodel.h"
#include "accounts-list-model.h"

#include <QtGui/QWidget>
#include <QtGui/QStyledItemDelegate>

#include <TelepathyQt4/AccountManager>

class QSortFilterProxyModel;
class QAbstractProxyModel;
class ContactsModelFilter;
class KMenu;
class KSelectAction;

namespace KTelepathy {
    class ContactsListModel;
    class GroupedContactsProxyModel;
}

namespace Tp {
    class PendingOperation;
}

class ContactDelegate : public QStyledItemDelegate
{
    Q_OBJECT
    public:
        ContactDelegate(QObject * parent = 0);
        ~ContactDelegate();

        virtual void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const;
        virtual QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const;
};

class MainWidget : public QWidget, Ui::MainWidget
{
    Q_OBJECT

public:
    MainWidget(QWidget *parent = 0);
    ~MainWidget();

public Q_SLOTS:
    void onAccountManagerReady(Tp::PendingOperation *op);
    void onChannelJoined(Tp::PendingOperation *op);
    void startTextChannel(const QModelIndex &index);
    void onContactListDoubleClick(const QModelIndex &index);
    void onConnectionChanged(const Tp::ConnectionPtr &connection);
    void onAccountReady(Tp::PendingOperation *op);
    void onAccountConnectionStatusChanged(Tp::ConnectionStatus status);
    void loadContactsFromAccount(const Tp::AccountPtr &account);
    void showMessageToUser(const QString &text);
    //    void startAudioChannel();
    //    void startVideoChannel();
    
    void onCustomContextMenuRequested(const QPoint &point);
    //Menu actions
    void onStartChat(bool);
    void onRequestRemoveFromGroup(bool);
    void onContactRemovalRequest(bool);
    void onContactBlockRequest(bool);
    void onHandlerReady(bool);
    void onRequestAddToGroup(bool);
    void onAddToMetaContact(bool);
    void onRemoveFromMetacontact(bool);
    //Toolbar actions
    void onAddContactRequest(bool);
    void onGroupContacts(bool);
    
private:
    FakeContactsModel*      m_model;
    QSortFilterProxyModel*  m_sortFilterProxyModel;
    QAbstractProxyModel*    m_currentModel;
    Tp::AccountManagerPtr   m_accountManager;
    AccountsListModel*      m_accountsListModel;
    KMenu*                  m_accountMenu;
    KSelectAction*          m_setStatusAction;
    
//     KTelepathy::GroupedContactsProxyModel *m_groupedContactsProxyModel;
//     Nepomuk::PersonContact m_mePersonContact;

};


#endif // Header guard

