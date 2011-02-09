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

#include <QList>
#include <QVariant>

#include "fakecontactsmodel.h"
#include "contactitem.h"

ContactItem::ContactItem(const Tp::ContactPtr& data, ContactItem* parent, ItemType type)
{
    m_parentContact = parent;
    m_contactData = data;
    m_itemType = type;
}

ContactItem::ContactItem(ContactItem* parent, ContactItem::ItemType type)
{
    m_parentContact = parent;
    m_itemType = type;
}

ContactItem::~ContactItem()
{
    qDeleteAll(m_childContacts);
}

void ContactItem::appendChildContact(ContactItem* childContact)
{
    m_childContacts.append(childContact);
}

ContactItem* ContactItem::childContact(int row)
{
    return m_childContacts.value(row);
}

int ContactItem::childContactsCount() const
{
    return m_childContacts.count();
}

Tp::ContactPtr ContactItem::data() const
{
    return m_contactData;
}

Tp::AccountPtr ContactItem::parentAccount() const
{
    return m_parentAccount;
}

void ContactItem::setParentAccount(const Tp::AccountPtr &account)
{
    m_parentAccount = account;
}

ContactItem* ContactItem::parent()
{
    return m_parentContact;
}

int ContactItem::columnCount() const
{
    return 1;
}

int ContactItem::row() const
{
    if (m_parentContact)
        return m_parentContact->m_childContacts.indexOf(const_cast<ContactItem*>(this));
    
    return 0;
}

ContactItem::ItemType ContactItem::type() const
{
    return m_itemType;
}

bool ContactItem::isContact() const
{
    if(m_itemType == ContactItem::Contact)
        return true;
    return false;
}

bool ContactItem::isGroup() const
{
    if(m_itemType == ContactItem::Group)
        return true;
    return false;
}

QString ContactItem::groupName() const
{
    return m_groupName;
}

void ContactItem::setGroupName(const QString& groupName)
{
    m_groupName = groupName;
}