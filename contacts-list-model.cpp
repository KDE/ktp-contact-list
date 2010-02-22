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

// Nepomuk Vocabulary URIs
#include "nepomuk/pimo.h"
#include "nco.h"
#include "telepathy.h"

// Nepomuk Resources
#include "imaccount.h"
#include "personcontact.h"

#include <KDebug>

#include <Nepomuk/ResourceManager>
#include <Nepomuk/Variant>

#include <Soprano/Model>
#include <Soprano/QueryResultIterator>

ContactsListModel::ContactsListModel(QObject *parent)
 : QAbstractListModel(parent)
{
    kDebug();

    // FIXME: Get the Nepomuk Resource for myself in the standardised way, once it is standardised.
    Nepomuk::Resource me("nepomuk://myself");

    // FIXME: Make it check if the found PersonContacts are grounding occurrences of me.
    QString query = QString("select distinct ?a ?b where { ?a a %1 . ?a %2 ?b . ?b a %3 . ?b %4 ?r . ?r a %3 . ?s %2 ?r . ?s a %1 . }") // %5 %6 ?s }")
            .arg(Soprano::Node::resourceToN3(Nepomuk::Vocabulary::NCO::PersonContact()))
            .arg(Soprano::Node::resourceToN3(Nepomuk::Vocabulary::NCO::hasIMAccount()))
            .arg(Soprano::Node::resourceToN3(Nepomuk::Vocabulary::NCO::IMAccount()))
            .arg(Soprano::Node::resourceToN3(Nepomuk::Vocabulary::Telepathy::isBuddyOf()));
//            .arg(Soprano::Node::resourceToN3(me.resourceUri()))
//            .arg(Soprano::Node::resourceToN3(Nepomuk::Vocabulary::PIMO::groundingOccurrence()));

    Soprano::Model *model = Nepomuk::ResourceManager::instance()->mainModel();

    Soprano::QueryResultIterator it = model->executeQuery(query, Soprano::Query::QueryLanguageSparql);

    // Iterate over all the IMAccounts/PersonContacts found.
    while(it.next()) {
        Nepomuk::PersonContact foundPersonContact(it.binding("a").uri());
        Nepomuk::IMAccount foundIMAccount(it.binding("b").uri());
        kDebug() << this << ": Found Contact:" << foundIMAccount.imIDs().first();

        // And create a ContactItem for each one.
        m_contactItems.append(new ContactItem(foundPersonContact, foundIMAccount, this));
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
    // If the index is the root item, then return the row count.
    if (parent == QModelIndex()) {
       return m_contactItems.size();
    }

    // Otherwise, return 0 (as this is a list model, so all items
    // are children of the root item).
    return 0;
}

QVariant ContactsListModel::data(const QModelIndex &index, int role) const
{
    if (index.column() != 0) {
        return QVariant();
    }

    QVariant data;
    switch(role)
    {
    case Qt::DisplayRole:
        data.setValue<QString>(m_contactItems.at(index.row())->displayName());
        break;
    default:
        break;
    }

    return data;
}

Qt::ItemFlags ContactsListModel::flags(const QModelIndex &index) const
{
    return QAbstractItemModel::flags(index);
}

QModelIndex ContactsListModel::parent(const QModelIndex &index) const
{
    Q_UNUSED(index);

    // This is a list model, so all items are children of the root item.
    return QModelIndex();
}

QModelIndex ContactsListModel::index(int row, int column, const QModelIndex &parent) const
{
    // List view, so all items are children of the root item.
    if (parent.isValid()) {
        return QModelIndex();
    }

    // Only 1 column
    if (column != 0) {
        return QModelIndex();
    }

    // Check the row is within the range of the list.
    if (row >= m_contactItems.size()) {
        return QModelIndex();
    }

    // Return the index to the item.
    return createIndex(row, column, 0);
}

QVariant ContactsListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(section);

    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return QVariant("Contact Name");
    }

    return QVariant();
}


#include "contacts-list-model.moc"

