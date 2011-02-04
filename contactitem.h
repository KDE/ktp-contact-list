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

class Person
{
    
public:
    int           contactId;          //just simple id to reference it from parentId
    int           parentId;           //id of the metacontact, that is parent to this contact
    QString       contactName;        //this is the account username/number
    QPixmap       avatar;             //user's avatar
    int           status;             //status - online, away, offline etc.
    QString       statusMessage;      //additional status message 
    QStringList   groups;             //groups the current contact is in
    QList<int>    protocolsConnected; //makes sense only for metacontact..I guess
    QSet<QString> capabilities;       //capabilities - send file etc - makes sense when not metacontact
};

class ContactItem
{
public:
    ContactItem(const Person &data, ContactItem *parent = 0);
    ~ContactItem();
    
    void appendChildContact(ContactItem *childContact);
    
    ContactItem *childContact(int row);
    int childContactsCount() const;
    int columnCount() const;
    Person data() const;
    int row() const;
    ContactItem *parent();
    
private:
    QList<ContactItem*> m_childContacts;
    Person m_contactData;
    ContactItem *m_parentContact;
};

#endif // CONTACTITEM_H
