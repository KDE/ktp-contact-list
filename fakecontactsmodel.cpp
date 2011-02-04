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


#include "fakecontactsmodel.h"

FakeContactsModel::FakeContactsModel(QObject* parent): QAbstractItemModel(parent)
{
    Person me;
    m_rootItem = new ContactItem(me);
    initContacts();
}

FakeContactsModel::~FakeContactsModel()
{
    delete m_rootItem;
}

void FakeContactsModel::initContacts()
{
    Person c1;
    c1.contactId = 1;
    c1.parentId = 0;
    c1.contactName = "Mike";
    c1.status = 1;
    c1.statusMessage = "Hello world!";
    c1.avatar = QPixmap("../avatars/dice.jpg");
    c1.protocolsConnected << 1 << 3;
    
    Person c2;
    c2.contactId = 2;
    c2.parentId = 0;
    c2.contactName = "Jack";
    c2.status = 2;
    c2.statusMessage = "I'm away";
    c2.avatar = QPixmap("../avatars/astronaut.jpg");
    c2.protocolsConnected << 2 << 3;
    
    Person c3;
    c3.contactId = 3;
    c3.parentId = 0;
    c3.contactName = "Will";
    c3.avatar = QPixmap("../avatars/fish.jpg");
    c3.status = 0;

    Person c4;
    c4.contactId = 4;
    c4.parentId = 0;
    c4.contactName = "Zach";
    c4.statusMessage = "Working...";
    c4.status = 1;
    c4.avatar = QPixmap("../avatars/coffee.jpg");
    c4.protocolsConnected << 1 << 2;
    
    Person c5;
    c5.contactId = 5;
    c5.parentId = 4;
    c5.contactName = "Zach ICQ";
    //c1.groups << 1 << 3
    c5.avatar = QPixmap("../avatars/lightning.jpg");
    c5.status = 1;
        
    Person c6;
    c6.contactId = 6;
    c6.parentId = 4;
    c6.contactName = "Zach GTalk";
    //c1.groups << 1 << 3
    c6.avatar = QPixmap("../avatars/chess.jpg");
    c6.status = 2;
    
    m_contacts << c1 << c2 << c3 << c4 << c5 << c6;
    
    Person p;
    Person c;
    foreach(p, m_contacts)
    {
        if(p.parentId == 0)
        {
            ContactItem *contact = new ContactItem(p, m_rootItem);
            m_rootItem->appendChildContact(contact);
            foreach(c, m_contacts)
            {
                if(c.parentId == p.contactId)
                {
                    contact->appendChildContact(new ContactItem(c, contact));
                }
            }
        }
    }
}

QVariant FakeContactsModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();
    
    ContactItem *contact = static_cast<ContactItem*>(index.internalPointer());
    
    if(role == ModelRoles::UserNameRole) 
    {
        return contact->data().contactName;
    }
    else if(role == ModelRoles::UserAvatarRole)
    {
        return contact->data().avatar;
    }
    else if(role == ModelRoles::UserStatusRole)
    {
        return contact->data().status;
    }
    else if(role == ModelRoles::UserStatusMsgRole)
    {
        return contact->data().statusMessage;
    }
    else if(role == ModelRoles::UserGroupsRole)
    {
        return contact->data().groups;
    }
    
    return QVariant();
    
}

int FakeContactsModel::columnCount(const QModelIndex& parent) const
{
    return 1;
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
}

QModelIndex FakeContactsModel::parent(const QModelIndex& index) const
{
    if (!index.isValid())
        return QModelIndex();
    
    ContactItem *childItem = static_cast<ContactItem*>(index.internalPointer());
    ContactItem *parentItem = childItem->parent();
    
    if (parentItem == m_rootItem)
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

