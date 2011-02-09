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


#ifndef CONTACTITEM_H
#define CONTACTITEM_H


#include <QList>

#include <TelepathyQt4/Contact>

class ContactItem
{
public:
    enum ItemType {Contact, Group};
    
    ContactItem(const Tp::ContactPtr &data, ContactItem *parent, ItemType type = ContactItem::Contact);
    ContactItem(ContactItem *parent = 0, ItemType type = ContactItem::Group);
    ~ContactItem();
    
    void appendChildContact(ContactItem *childContact);
    
    ContactItem *childContact(int row);
    int childContactsCount() const;
    int columnCount() const;
    Tp::ContactPtr data() const;
    Tp::AccountPtr parentAccount() const;
    void setParentAccount(const Tp::AccountPtr &account);
    int row() const;
    ContactItem *parent();
    ItemType type() const;
    bool isContact() const;
    bool isGroup() const;
    
    QString groupName() const;
    void setGroupName(const QString &groupName);
    
private:
    QList<ContactItem*>     m_childContacts;
    Tp::ContactPtr          m_contactData;
    Tp::AccountPtr          m_parentAccount;
    ContactItem*            m_parentContact;
    ItemType                m_itemType;
    QString                 m_groupName;
};

#endif // CONTACTITEM_H
