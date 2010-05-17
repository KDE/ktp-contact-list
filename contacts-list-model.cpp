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

#include "abstract-tree-item.h"
#include "contact-item.h"
#include "meta-contact-item.h"

// Nepomuk Vocabulary URIs
#include "pimo.h"
#include "nco.h"
#include "telepathy.h"

// Nepomuk Resources
#include "imaccount.h"
#include "person.h"
#include "personcontact.h"

#include <KDebug>

#include <Nepomuk/ResourceManager>
#include <Nepomuk/Variant>

#include <Nepomuk/Query/QueryServiceClient>
#include <Nepomuk/Query/Query>
#include <Nepomuk/Query/ResourceTypeTerm>
#include <Nepomuk/Query/ResourceTerm>
#include <Nepomuk/Query/ComparisonTerm>
#include <Nepomuk/Query/AndTerm>
#include <Nepomuk/Query/NegationTerm>
#include <Nepomuk/Query/Result>

#include <unistd.h>
#include <KMessageBox>

ContactsListModel::ContactsListModel(QObject *parent)
 : QAbstractItemModel(parent),
   m_rootItem(0)
{
    kDebug();

    // Create the root Item.
    m_rootItem = new AbstractTreeItem;

    // FIXME: Get the Nepomuk Resource for myself in the standardised way, once it is standardised.
    Nepomuk::Resource me(QUrl::fromEncoded("nepomuk:/myself"));

    m_metaContactsQuery = new Nepomuk::Query::QueryServiceClient(this);
    connect(m_metaContactsQuery, SIGNAL(newEntries(QList<Nepomuk::Query::Result>)),
            this, SLOT(onMetaContactsQueryNewEntries(QList<Nepomuk::Query::Result>)));
    connect(m_metaContactsQuery, SIGNAL(entriesRemoved(QList<QUrl>)),
            this, SLOT(onMetaContactsEntriesRemoved(QList<QUrl>)));

    // Get all metacontacts
    {
        using namespace Nepomuk::Query;

        Query query(ResourceTypeTerm(Nepomuk::Vocabulary::PIMO::Person()));

        bool queryResult = m_metaContactsQuery->query(query);
        kDebug() << "Metacontact query result " << queryResult;

        if (!queryResult) {
            KMessageBox::error(0, i18n("It was not possible to query Nepomuk database. Please check your "
                                       "installation and make sure Nepomuk is running."));
        }
    }

    m_contactsQuery = new Nepomuk::Query::QueryServiceClient(this);
    connect(m_contactsQuery, SIGNAL(newEntries(QList<Nepomuk::Query::Result>)),
            this, SLOT(onContactsQueryNewEntries(QList<Nepomuk::Query::Result>)));
    connect(m_contactsQuery, SIGNAL(entriesRemoved(QList<QUrl>)),
            this, SLOT(onContactsQueryEntriesRemoved(QList<QUrl>)));

    // Get all Telepathy PersonContacts which do not belong to any metacontact
    {
        using namespace Nepomuk::Query;
        using namespace Nepomuk::Vocabulary;
        // subquery to match grouding occurrences of me
        ComparisonTerm goterm(PIMO::groundingOccurrence(),
                              ResourceTerm(me));
        goterm.setInverted(true);

        // combine that with only nco:PersonContacts
        AndTerm pcgoterm(ResourceTypeTerm(NCO::PersonContact()),
                         goterm);

        // now look for im accounts of those grounding occurrences (pcgoterm will become the subject of this comparison,
        // thus the comparison will match the im accounts)
        ComparisonTerm impcgoterm(NCO::hasIMAccount(),
                                  pcgoterm);
        impcgoterm.setInverted(true);

        // now look for all buddies of the accounts
        ComparisonTerm buddyTerm(Telepathy::isBuddyOf(),
                                 impcgoterm);
        // set the name of the variable (i.e. the buddies) to be able to match it later
        buddyTerm.setVariableName("t");

        // same comparison, other property, but use the same variable name to match them
        ComparisonTerm ppterm(Telepathy::publishesPresenceTo(),
                              ResourceTypeTerm(NCO::IMAccount()));
        ppterm.setVariableName("t");

        // combine both to complete the matching of the im account ?account
        AndTerm accountTerm(ResourceTypeTerm(NCO::IMAccount()),
                            buddyTerm, ppterm);

        // match the account and select it for the results
        ComparisonTerm imaccountTerm(NCO::hasIMAccount(), accountTerm);
        imaccountTerm.setVariableName("account");

        // and finally the exclusion of those person contacts that already have a pimo person attached
        ComparisonTerm personTerm(PIMO::groundingOccurrence(),
                                  ResourceTypeTerm(PIMO::Person()));
        personTerm.setInverted(true);

        // and all combined
        Query query(AndTerm(ResourceTypeTerm(Nepomuk::Vocabulary::NCO::PersonContact()),
                            imaccountTerm, NegationTerm::negateTerm(personTerm)));

        bool queryResult = m_contactsQuery->query(query);

        kDebug() << "Contact query result " << queryResult;
        if (!queryResult) {
            KMessageBox::error(0, i18n("It was not possible to query Nepomuk database. Please check your "
                                       "installation and make sure Nepomuk is running."));
        }
    }
}

void ContactsListModel::onContactsQueryNewEntries(const QList<Nepomuk::Query::Result> &entries)
{
    kDebug();
    // Iterate over all the IMAccounts/PersonContacts found.
    foreach (const Nepomuk::Query::Result &result, entries) {
        Nepomuk::PersonContact foundPersonContact(result.resource());
        Nepomuk::IMAccount foundIMAccount(result.additionalBinding("account").toUrl());
        kDebug() << "New resource added: " << foundPersonContact << foundIMAccount;

        // Create a fake metacontact to hold this item.
        MetaContactItem *metaContactItem = new MetaContactItem(MetaContactItem::FakeMetaContact, 0);
        metaContactItem->setParentItem(m_rootItem);
        m_rootItem->appendChildItem(metaContactItem);
        connect(metaContactItem, SIGNAL(dirty()), SLOT(onItemDirty()));

        // And create the contact item itself, parenting it to the fake meta contact we just created.
        ContactItem *item = new ContactItem(foundPersonContact, foundIMAccount, 0);
        item->setParentItem(metaContactItem);
        metaContactItem->appendChildItem(item);
        connect(item, SIGNAL(dirty()), SLOT(onItemDirty()));
    }

    reset();
}

void ContactsListModel::onContactsQueryEntriesRemoved(const QList< QUrl > &entries)
{
    kDebug();
    // Find and remove
    foreach (const QUrl &url, entries) {
        kDebug() << "Attempting to remove resource " << url;
        foreach (const QModelIndex &index, findContacts(url)) {
            kDebug() << "Found matching item at" << index;
            AbstractTreeItem *abstractItem = static_cast<AbstractTreeItem*>(index.internalPointer());
            ContactItem *contactItem = dynamic_cast<ContactItem*>(abstractItem);

            if (contactItem) {
                // Yeah, we're here. Now let's have a look through metacontacts available
                MetaContactItem *metacontact = dynamic_cast<MetaContactItem*>(contactItem->parentItem());
                if (metacontact->type() != MetaContactItem::FakeMetaContact) {
                    // Not our business
                    kDebug() << "Skipping deletion as the metacontact is real";
                    continue;
                }

                metacontact->removeChildItem(contactItem);
                delete contactItem;
                if (metacontact->childItems().isEmpty()) {
                    // We will have to delete the metacontact as well
                    m_rootItem->removeChildItem(metacontact);
                    delete metacontact;
                }
            }
        }
    }

    reset();
}

void ContactsListModel::onMetaContactsEntriesRemoved(const QList< QUrl > &entries)
{
    Q_UNUSED(entries)
    // Not a big deal here actually, deletion is handled in MetaContactItem.
    // We just keep this slot for the day when it will be useful
}

void ContactsListModel::onMetaContactsQueryNewEntries(const QList< Nepomuk::Query::Result > &entries)
{
    kDebug();
    // Iterate over all the IMAccounts/PersonContacts found.
    foreach (const Nepomuk::Query::Result &result, entries) {
        kDebug() << result.resource();
        Nepomuk::Person foundPimoPerson(result.resource());
        kDebug() << "Found a PIMO Person "<< foundPimoPerson;
        MetaContactItem *item = new MetaContactItem(MetaContactItem::RealMetaContact, 0);
        item->setParentItem(m_rootItem);
        m_rootItem->appendChildItem(item);
        item->setPimoPerson(foundPimoPerson);
        connect(item, SIGNAL(dirty()), SLOT(onItemDirty()));
    }

    reset();
}

ContactsListModel::~ContactsListModel()
{
    kDebug();
}

QModelIndexList ContactsListModel::findContacts(const QUrl& resource)
{
    return findChildrenContacts(resource, QModelIndex());
}

QModelIndexList ContactsListModel::findChildrenContacts(const QUrl& resource, const QModelIndex& parent)
{
    QModelIndexList list;

    kDebug() << "Finding contacts in " << parent << rowCount(parent) << columnCount(parent);

    for (int i = 0; i < rowCount(parent); ++i) {
        for (int j = 0; j < columnCount(parent); ++j) {
            QModelIndex current = index(i, j, parent);

            if (current.data(PersonContactResourceRole) == resource) {
                list << current;
            }

            // Iterate over children, if any
            if (hasChildren(current)) {
                list << findChildrenContacts(resource, current);
            }
        }
    }

    return list;
}

int ContactsListModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    // All items have the same number of columns
    return 1;
}

QVariant ContactsListModel::data(const QModelIndex &index, int role) const
{
    // Only column 0 is valid.
    if (index.column() != 0) {
        return QVariant();
    }

    // Check what type of item we have here.
    AbstractTreeItem *abstractItem = static_cast<AbstractTreeItem*>(index.internalPointer());
    ContactItem *contactItem = dynamic_cast<ContactItem*>(abstractItem);

    if (contactItem) {

        QVariant data;

        switch(role)
        {
        case Qt::DisplayRole:
            data.setValue<QString>(contactItem->accountIdentifier());
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
        case ContactsListModel::AvatarRole:
            data.setValue<QPixmap>(contactItem->avatar());
            break;
        case ContactsListModel::PersonContactResourceRole:
            data.setValue<QUrl>(contactItem->personContact().resourceUri());
            break;
        default:
            break;
        }

        return data;
    }

    MetaContactItem *metaContactItem = dynamic_cast<MetaContactItem*>(abstractItem);

    if (metaContactItem) {
        QVariant data;
        switch(role)
        {
            // FIXME: Implement all the roles properly for meta contacts.
        case Qt::DisplayRole:
            data.setValue<QString>(metaContactItem->displayName());
            break;
        case Qt::DecorationRole:
            data.setValue<QIcon>(metaContactItem->presenceIcon());
            break;
        case ContactsListModel::PresenceTypeRole:
//            data.setValue<qint64>(contactItem->presenceType());
            break;
        case ContactsListModel::GroupsRole:
            data.setValue<QStringList>(metaContactItem->groups());
            break;
        case ContactsListModel::AvatarRole:
            data.setValue<QPixmap>(metaContactItem->avatar());
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

QVariant ContactsListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(section);

    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return QVariant("Contact Name");
    }

    return QVariant();
}

QModelIndex ContactsListModel::index(int row, int column, const QModelIndex &parent) const
{
    // 1 column list, so invalid index if the column is not 1.
    if (parent.isValid() && parent.column() != 0) {
        return QModelIndex();
    }

    // Get the parent item.
    AbstractTreeItem *parentItem = item(parent);

    // Get all the parent's children.
    QList<AbstractTreeItem*> children = parentItem->childItems();

    // Check the row doesn't go beyond the end of the list of children.
    if (row >= children.length()) {
        return QModelIndex();
    }

    // Return the index to the item.
    return createIndex(row, column, children.at(row));
}

QModelIndex ContactsListModel::parent(const QModelIndex &index) const
{
    // If the index is invalid, return an invalid parent index.
    if (!index.isValid()) {
        return QModelIndex();
    }

    // Get the item we have been passed, and it's parent
    AbstractTreeItem *childItem = item(index);
    AbstractTreeItem *parentItem = childItem->parentItem();

    // If the parent is the root item, then the parent index of the index we were passed is
    // by definition an invalid index.
    if (parentItem == m_rootItem) {
        return QModelIndex();
    }

    // The parent of the item is not the root item, meaning that the parent must have a parent too.
    AbstractTreeItem *parentOfParentItem = parentItem->parentItem();

    // As stated in the previous comment, something is really wrong if it doesn't have a parent.
    Q_ASSERT(parentOfParentItem);
    if (!parentOfParentItem) {
        kWarning() << "Impossible parent situation occurred!";
        return createIndex(0, 0, parentItem);
    }

    // Return the model index of the parent item.
    return createIndex(parentOfParentItem->childItems().lastIndexOf(parentItem), 0, parentItem);
}

int ContactsListModel::rowCount(const QModelIndex &parent) const
{
    // If the parent is invalid, then this request is for the root item.
    if (!parent.isValid()) {
        return m_rootItem->childItems().length();
    }

    // Get the item from the internal pointer of the ModelIndex.
    AbstractTreeItem *item = static_cast<AbstractTreeItem*>(parent.internalPointer());

    // If the item is valid, return the number of children it has.
    if (item) {
        return item->childItems().length();
    }

    // Otherwise, return 0
    return 0;
}

void ContactsListModel::onItemDirty()
{
    ContactItem *item = qobject_cast<ContactItem*>(sender());

    Q_ASSERT(item);
    if (!item) {
        kWarning() << "Invalid sender.";
    }

    reset();

    // FIXME: Port this stuff to new tree structure.
   // QModelIndex itemIndex = index(m_contactItems.indexOf(item), 0, QModelIndex());
   // Q_EMIT dataChanged(itemIndex, itemIndex);
}

AbstractTreeItem* ContactsListModel::item(const QModelIndex &index) const
{
    if (index.isValid()) {
        AbstractTreeItem *item = static_cast<AbstractTreeItem*>(index.internalPointer());
         if (item) {
             return item;
         }
     }

     return m_rootItem;
}


#include "contacts-list-model.moc"

