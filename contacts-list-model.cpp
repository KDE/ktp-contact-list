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

#include "contacts-list-model.h"

#include "contact-item.h"
#include "meta-contact-item.h"

// Nepomuk Vocabulary URIs
#include "nepomuk/pimo.h"
#include "nco.h"
#include "telepathy.h"

// Nepomuk Resources
#include "imaccount.h"
#include "person.h"
#include "personcontact.h"

#include <KDebug>

#include <Nepomuk/ResourceManager>
#include <Nepomuk/Variant>

#include <Soprano/Model>
#include <Soprano/QueryResultIterator>

#include <unistd.h>

ContactsListModelItem::ContactsListModelItem(QObject *parent)
 : QObject(parent),
   m_parent(0)
{

}

ContactsListModelItem::~ContactsListModelItem()
{

}

QList<ContactsListModelItem*> ContactsListModelItem::childItems() const
{
    return m_children;
}

ContactsListModelItem *ContactsListModelItem::parentItem() const
{
    return m_parent;
}

void ContactsListModelItem::appendChildItem(ContactsListModelItem *child)
{
    m_children.append(child);
}

void ContactsListModelItem::removeChildItem(ContactsListModelItem *child)
{
    m_children.removeOne(child);
}

void ContactsListModelItem::setParentItem(ContactsListModelItem *parent)
{
    m_parent = parent;
}


// -------------------------------------------------------------------------------------------------


ContactsListModel::ContactsListModel(QObject *parent)
 : QAbstractItemModel(parent),
   m_rootItem(0)
{
    kDebug();

    // Create the root Item.
    m_rootItem = new ContactsListModelItem;

    // FIXME: Get the Nepomuk Resource for myself in the standardised way, once it is standardised.
    Nepomuk::Resource me(QUrl::fromEncoded("nepomuk:/myself"));

    // Get ALL THE METACONTACT!!!!11!!111!!11one111!!!!eleven!!!1!
    QString metaContactquery = QString("select distinct ?a where { ?a a %7 . }")
            .arg(Soprano::Node::resourceToN3(Nepomuk::Vocabulary::PIMO::Person()));

    QString query = QString("select distinct ?a ?b where { ?a a %1 . ?a %2 ?b . ?b a %3 . ?b %4 ?r . ?r a %3 . ?s %2 ?r . ?s a %1 . %5 %6 ?s }")
            .arg(Soprano::Node::resourceToN3(Nepomuk::Vocabulary::NCO::PersonContact()))
            .arg(Soprano::Node::resourceToN3(Nepomuk::Vocabulary::NCO::hasIMAccount()))
            .arg(Soprano::Node::resourceToN3(Nepomuk::Vocabulary::NCO::IMAccount()))
            .arg(Soprano::Node::resourceToN3(Nepomuk::Vocabulary::Telepathy::isBuddyOf()))
            .arg(Soprano::Node::resourceToN3(me.resourceUri()))
            .arg(Soprano::Node::resourceToN3(Nepomuk::Vocabulary::PIMO::groundingOccurrence()));

    Soprano::Model *model = Nepomuk::ResourceManager::instance()->mainModel();

    Soprano::QueryResultIterator metaIt = model->executeQuery(metaContactquery, Soprano::Query::QueryLanguageSparql);

    while(metaIt.next()) {
        Nepomuk::Person foundPimoPerson(metaIt.binding("a").uri());

        kDebug() << "Found a PIMO Person.";
   //     MetaContactItem *item = new MetaContactItem(foundPimoPerson, 0);
   //     item->setParentItem(m_rootItem);
   //     m_rootItem->appendChildItem(item);
   //     connect(item, SIGNAL(dirty()), SLOT(onItemDirty()));
    }

    Soprano::QueryResultIterator it = model->executeQuery(query, Soprano::Query::QueryLanguageSparql);

    // Iterate over all the IMAccounts/PersonContacts found.
    while(it.next()) {
        Nepomuk::PersonContact foundPersonContact(it.binding("a").uri());
        Nepomuk::IMAccount foundIMAccount(it.binding("b").uri());
      //  kDebug() << this << ": Found Contact:" << foundIMAccount.imIDs().first();

        // And create a ContactItem for each one.
        MetaContactItem *metaContactItem = new MetaContactItem(MetaContactItem::FakeMetaContact, 0);
        metaContactItem->setParentItem(m_rootItem);
        m_rootItem->appendChildItem(metaContactItem);
        connect(metaContactItem, SIGNAL(dirty()), SLOT(onItemDirty()));

        ContactItem *item = new ContactItem(foundPersonContact, foundIMAccount, 0);
        item->setParentItem(metaContactItem);
        metaContactItem->appendChildItem(item);
        connect(item, SIGNAL(dirty()), SLOT(onItemDirty()));
    }
}

ContactsListModel::~ContactsListModel()
{
    kDebug();
}

int ContactsListModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    // List view, so all items have the same number of columns
    return 1;
}

int ContactsListModel::rowCount(const QModelIndex &parent) const
{
    kDebug();

    // If the parent is invalid, then this request is for the root item.
    if (!parent.isValid()) {
        return m_rootItem->childItems().length();
    }

    // Get the item from the internal pointer of the ModelIndex.
    ContactsListModelItem *item = static_cast<ContactsListModelItem*>(parent.internalPointer());

    // If the item is valid, return the number of children it has.
    if (item) {
        kDebug() << item->childItems().length();
        return item->childItems().length();
    }

    // Otherwise, return 0
    return 0;
}

QVariant ContactsListModel::data(const QModelIndex &index, int role) const
{
    kDebug() << "index:" << index << "parent:" << index.parent();
    // Only column 0 is valid.
    if (index.column() != 0) {
        return QVariant();
    }

    if (index.internalPointer() == m_rootItem) {
        kDebug() << "Internal root item pointe rwtf?";
    }

    // Check what type of item we have here.
    ContactsListModelItem *clmItem = static_cast<ContactItem*>(index.internalPointer());
    ContactItem *contactItem = qobject_cast<ContactItem*>(clmItem);

    kDebug() << contactItem;

    if (contactItem) {
        kDebug() << "Is a contact Item." << contactItem;
        QVariant data;
        switch(role)
        {
        case Qt::DisplayRole:
            data.setValue<QString>(contactItem->displayName());
            break;
        case Qt::DecorationRole:
            data.setValue<QIcon>(contactItem->presenceIcon());
            break;
        case ContactsListModel::PresenceTypeRole:
            data.setValue<qint64>(contactItem->presenceType());
            break;
        case ContactsListModel::GroupsRole:
            data.setValue<QStringList>(contactItem->groups());
            break;
        default:
            break;
        }

        return data;
    }

    MetaContactItem *metaContactItem = qobject_cast<MetaContactItem*>(clmItem);

    if (metaContactItem) {
        QVariant data;
        switch(role)
        {
        case Qt::DisplayRole:
            data.setValue<QString>(metaContactItem->displayName());
            break;
        case Qt::DecorationRole:
//            data.setValue<QIcon>(contactItem->presenceIcon());
            break;
        case ContactsListModel::PresenceTypeRole:
//            data.setValue<qint64>(contactItem->presenceType());
            break;
        case ContactsListModel::GroupsRole:
//            data.setValue<QStringList>(contactItem->groups());
            break;
        default:
            break;
        }

        return data;
    }

    Q_ASSERT(false);
    kWarning() << "Model Index pointer is of invalid type.";

    return QVariant();
}

Qt::ItemFlags ContactsListModel::flags(const QModelIndex &index) const
{
    return QAbstractItemModel::flags(index);
}

QModelIndex ContactsListModel::parent(const QModelIndex &index) const
{
    // If the index is invalid, return an invalid parent index.
    if (!index.isValid()) {
        return QModelIndex();
    }

    // Get the item we have been passed, and it's parent
    ContactsListModelItem *childItem = item(index);
    ContactsListModelItem *parentItem = childItem->parentItem();

    // If the parent is the root item, then the parent index of the index we were passed is
    // by definition an invalid index.
    if (parentItem == m_rootItem) {
        return QModelIndex();
    }

    // The parent of the item is not the root item, meaning that the parent must have a parent too.
    ContactsListModelItem *parentOfParentItem = parentItem->parentItem();

    // As stated in the previous comment, something is really wrong if it doesn't have a parent.
    Q_ASSERT(parentOfParentItem);
    if (!parentOfParentItem) {
        kWarning() << "Impossible parent situation occurred!";
        return createIndex(0, 0, parentItem);
    }

    // Return the model index of the parent item.
    return createIndex(parentOfParentItem->childItems().lastIndexOf(parentItem), 0, parentItem);
}

QModelIndex ContactsListModel::index(int row, int column, const QModelIndex &parent) const
{
    kDebug();
    // 1 column list, so invalid index if the column is not 1.
    if (parent.isValid() && parent.column() != 0) {
        kDebug() << "Wrong column";
        return QModelIndex();
    }

    // Get the parent item.
    ContactsListModelItem *parentItem = item(parent);

    // Get all the parent's children.
    QList<ContactsListModelItem*> children = parentItem->childItems();

    // Check the row doesn't go beyond the end of the list of children.
    if (row >= children.length()) {
        kDebug() << "Bounds error";
        return QModelIndex();
    }

    // Return the index to the item.
    kDebug() << "AOK:" << row << column << children.at(row);
    return createIndex(row, column, children.at(row));
}

ContactsListModelItem* ContactsListModel::item(const QModelIndex &index) const
{
    if (index.isValid()) {
        ContactsListModelItem *item = static_cast<ContactsListModelItem*>(index.internalPointer());
         if (item) {
             return item;
         }
     }

     return m_rootItem;
}

QVariant ContactsListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(section);

    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return QVariant("Contact Name");
    }

    return QVariant();
}

void ContactsListModel::onItemDirty()
{
    ContactItem *item = qobject_cast<ContactItem*>(sender());

    Q_ASSERT(item);
    if (!item) {
        kWarning() << "Invalid sender.";
    }

    // FIXME: Port this stuff to new tree structure.
   // QModelIndex itemIndex = index(m_contactItems.indexOf(item), 0, QModelIndex());
   // Q_EMIT dataChanged(itemIndex, itemIndex);
}

#include "contacts-list-model.moc"

