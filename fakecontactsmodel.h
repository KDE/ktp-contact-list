/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/


#ifndef FAKECONTACTSMODEL_H
#define FAKECONTACTSMODEL_H

#include <QObject>
#include <QAbstractItemModel>
#include <QString>
#include <QStringList>
#include <QPixmap>
#include <QSet>

//FIXME find out which include I really need for the ptrs
#include <TelepathyQt4/Account>
#include <TelepathyQt4/Contact>
#include <TelepathyQt4/PendingContacts>

#include "contactitem.h"

/* contact status
 * 0 - offline
 * 1 - online
 * 2 - away
 * 3 - not available
 * 4 - do not disturb
 * 5 - free for chat
 * 6 - invisible
 */

Q_DECLARE_METATYPE(Tp::ConnectionPresenceType);

class ModelRoles {
public:
    enum {
        UserNameRole         = Qt::DisplayRole,
        UserAvatarRole       = Qt::DecorationRole,
        UserStatusRole       = Qt::UserRole + 1,
        UserStatusMsgRole    = Qt::UserRole + 2,
        UserGroupsRole       = Qt::UserRole + 3,
        IsContact            = Qt::UserRole + 4,
        AccountGroupRole     = Qt::UserRole + 5,
        AccountAllContactsCountRole = Qt::UserRole + 6,
        AccountAvailContactsCountRole = Qt::UserRole + 7
    };
};

class FakeContactsModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    FakeContactsModel(QObject* parent = 0);
    ~FakeContactsModel();
    
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    int columnCount(const QModelIndex& parent = QModelIndex()) const;
    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex& child) const;
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
    
    Tp::AccountPtr account(const QModelIndex& index) const;
    void addAccountContacts(Tp::AccountPtr account);
    
    Tp::ContactPtr contact(const QModelIndex &index) const;
    
public Q_SLOTS:
    void updateContactList();
    void setAllOffline();
    void onContactUpdated();
    void clear();
    
private:
    ContactItem *m_rootItem; 
    
    QList<Tp::ContactPtr> m_contacts;
    QList<ContactItem*> m_groups;
    QSet<Tp::AccountPtr> m_accounts; //use QSet for auto-preventing of inserting duplicate accounts
};

#endif // FAKECONTACTSMODEL_H
