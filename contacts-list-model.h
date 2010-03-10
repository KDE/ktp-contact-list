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

#ifndef TELEPATHY_CONTACTSLIST_PROTOTYPE_CONTACTS_LIST_MODEL_H
#define TELEPATHY_CONTACTSLIST_PROTOTYPE_CONTACTS_LIST_MODEL_H

#include <QtCore/QAbstractItemModel>

class ContactItem;

class ContactsListModelItem : public QObject
{
    Q_OBJECT

public:
    ContactsListModelItem(QObject *parent = 0);
    virtual ~ContactsListModelItem();

    virtual void appendChildItem(ContactsListModelItem *child);
    virtual void removeChildItem(ContactsListModelItem *child);

    virtual void setParentItem(ContactsListModelItem *parent);

    virtual QList<ContactsListModelItem*> childItems() const;
    virtual ContactsListModelItem *parentItem() const;

private:
    QList<ContactsListModelItem*> m_children;
    ContactsListModelItem *m_parent;
};

class ContactsListModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    enum {
        PresenceTypeRole = Qt::UserRole,
        GroupsRole
    };

    explicit ContactsListModel(QObject *parent = 0);
    virtual ~ContactsListModel();

     virtual QVariant data(const QModelIndex &index, int role) const;
     virtual Qt::ItemFlags flags(const QModelIndex &index) const;
     virtual QVariant headerData(int section,
                                 Qt::Orientation orientation,
                                 int role = Qt::DisplayRole) const;
     virtual QModelIndex index(int row,
                               int column,
                               const QModelIndex &parent = QModelIndex()) const;
     virtual QModelIndex parent(const QModelIndex &index) const;
     virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
     virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;

private Q_SLOTS:
    void onItemDirty();

private:
    Q_DISABLE_COPY(ContactsListModel);

    ContactsListModelItem *item(const QModelIndex &index) const;

    ContactsListModelItem *m_rootItem;
};


#endif // header guard

