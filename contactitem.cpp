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

ContactItem::ContactItem(const Person& data, ContactItem* parent)
{
    m_parentContact = parent;
    m_contactData = data;
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
    return m_childContacts.at(row);
}

int ContactItem::childContactsCount() const
{
    return m_childContacts.count();
}

Person ContactItem::data() const
{
    return m_contactData;
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