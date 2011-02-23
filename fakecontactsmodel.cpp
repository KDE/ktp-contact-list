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

#include <KIcon>
#include <KDebug>

#include <TelepathyQt4/ContactManager>
#include <TelepathyQt4/Contact>
#include <TelepathyQt4/PendingReady>
#include <TelepathyQt4/PendingContacts>
#include <TelepathyQt4/AvatarData>

#include "fakecontactsmodel.h"

FakeContactsModel::FakeContactsModel(QObject* parent): QAbstractItemModel(parent)
{
    Tp::ContactPtr me;
    m_rootItem = new ContactItem();
}

FakeContactsModel::~FakeContactsModel()
{
    delete m_rootItem;
}

Tp::AccountPtr FakeContactsModel::account(const QModelIndex& index) const
{
    ContactItem *contact = static_cast<ContactItem*>(index.internalPointer());
    return contact->parentAccount();
}

void FakeContactsModel::addAccountContacts(Tp::AccountPtr account)
{
    if(!m_accounts.contains(account))
    {
        kDebug() << "Adding account to contact list..";
        
        if (account->connectionStatus() == Tp::ConnectionStatusConnected && account->connection()) 
        {
            m_accounts.insert(account);
            Tp::ContactManagerPtr contactManager = account->connection()->contactManager();
            
            QList<Tp::ContactPtr> newContacts = contactManager->allKnownContacts().toList();
            
            //need to add a connection to each contact to emit updated when applicable.
            
            beginInsertRows(QModelIndex(), 0, newContacts.size());
            kDebug() << newContacts.size() << "contacts from " << account->displayName();
            
            ContactItem *group = new ContactItem(m_rootItem);
            m_rootItem->appendChildContact(group);
            m_groups.append(group);
            
            group->setGroupName(account->displayName());
            group->setParentAccount(account);
            
            Tp::ContactPtr p;
            foreach(p, newContacts)
            {
                ContactItem *contact = new ContactItem(p, group);
                contact->setParentAccount(account);

                group->appendChildContact(contact);  

                connect(p.data(), SIGNAL(presenceChanged(Tp::Presence)),
                        this, SLOT(onContactUpdated()));
            }

            m_contacts.append(newContacts);
            endInsertRows();
        }
        else kDebug() << "No contacts added";
        
        //updateContactList();    
    }
    else kDebug() << "Account already in the contact list, skipping..";
}

void FakeContactsModel::updateContactList()
{
    Tp::AccountPtr account;
    
    foreach(account, m_accounts)
    {

    }
}

void FakeContactsModel::setAllOffline()
{
    
}

void FakeContactsModel::onContactUpdated()
{
    emit dataChanged(createIndex(0,0), createIndex(rowCount()-1,0)); //update the whole list
}

Tp::ContactPtr FakeContactsModel::contact(const QModelIndex& index) const
{
    if (! index.isValid()) {
        return Tp::ContactPtr();
    }
    return m_contacts.at(index.row());
}

void FakeContactsModel::clear()
{
    beginRemoveRows(QModelIndex(), 0, m_contacts.size());
    m_contacts.clear();
    endRemoveRows();
}

QVariant FakeContactsModel::data(const QModelIndex& index, int role) const
{
    if(!index.isValid())
        return QVariant();
    
    ContactItem *contact = static_cast<ContactItem*>(index.internalPointer());
    
    if(contact->isContact())
    {
    
        if(role == ModelRoles::UserNameRole) 
        {
            return contact->data()->alias();
        }
        else if(role == ModelRoles::UserAvatarRole)
        {
            if(!contact->data()->isAvatarTokenKnown()) 
            {
                return QVariant(KIcon("im-user").pixmap(32, 32));
            }
            else 
            {
                return QVariant(QIcon(contact->data()->avatarData().fileName).pixmap(32, 32));
            }
        }
        else if(role == ModelRoles::UserStatusRole)
        {
            return QVariant::fromValue<Tp::ConnectionPresenceType>(contact->data()->presence().type());
        }
        else if(role == ModelRoles::UserStatusMsgRole)
        {
            return contact->data()->presence().statusMessage();
        }
        else if(role == ModelRoles::UserGroupsRole)
        {
            return contact->data()->groups();
        }
//         else if(role == ModelRoles::AccountGroupRole)
//         {
//             return contact->groupName();
//         }
        else if(role == ModelRoles::IsContact)
        {
            return contact->isContact();
        }
    }
    else 
    {
        if(role == ModelRoles::AccountGroupRole)
        {
            return contact->groupName();
        }        
        else if(role == ModelRoles::IsContact)
        {
            return contact->isContact();
        }
        else if(role == ModelRoles::AccountAllContactsCountRole)
        {
            return contact->childContactsCount();
        }
        else if(role == ModelRoles::AccountAvailContactsCountRole)
        {
            return 0;//contact->parentAccount()->
        }
        else if(role == ModelRoles::AccountIconRole)
        {
            QString iconPath = contact->parentAccount()->iconName();
            
            //if the icon has not been set, we use the protocol icon    
            if(iconPath.isEmpty()) {
                iconPath = QString("im-%1").arg(contact->parentAccount()->protocolName());
            }
            
            return iconPath;
        }
    }

    return QVariant();
}

int FakeContactsModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return static_cast<ContactItem*>(parent.internalPointer())->columnCount();
    else
        return m_rootItem->columnCount();
}

int FakeContactsModel::rowCount(const QModelIndex& parent) const
{
    ContactItem *parentItem;
    
    if (parent.column() > 0)
        return 0;
    
    if (!parent.isValid())
        parentItem = m_rootItem;
    else
        parentItem = static_cast<ContactItem*>(parent.internalPointer());
    
    return parentItem->childContactsCount();

//     if (parent == QModelIndex()) {
//         return m_contacts.size();
//     }
//     return 0;

}

QModelIndex FakeContactsModel::parent(const QModelIndex& index) const
{
    if (!index.isValid())
        return QModelIndex();
    
    ContactItem *childItem = static_cast<ContactItem*>(index.internalPointer());
    
    // if in any case the childItem is m_rootItem->parent(), return empty index
    if (childItem == m_rootItem || !childItem)
        return QModelIndex();
    
    ContactItem *parentItem = childItem->parent();
    
    if (parentItem == m_rootItem || !parentItem)
        return QModelIndex();
    
    return createIndex(parentItem->row(), 0, parentItem);
}

QModelIndex FakeContactsModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();
    
    ContactItem *parentItem;
    
    if (!parent.isValid())
        parentItem = m_rootItem;
    else
        parentItem = static_cast<ContactItem*>(parent.internalPointer());
    
    ContactItem *childItem = parentItem->childContact(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

#include "fakecontactsmodel.moc"

