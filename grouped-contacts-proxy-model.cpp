/*
 * This file is part of telepathy-contactslist-prototype
 *
 * Copyright (C) 2010 Collabora Ltd. <info@collabora.co.uk>
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

#include "grouped-contacts-proxy-model.h"

#include "contact-item.h"
#include "contacts-list-model.h"

#include <KDebug>

#include <QtGui/QItemSelection>

GroupedContactsProxyModel::GroupedContactsProxyModel(QObject* parent)
 : QAbstractProxyModel(parent),
   m_sourceModel(0),
   m_rootItem(0)
{

}

GroupedContactsProxyModel::~GroupedContactsProxyModel()
{

}

Qt::ItemFlags GroupedContactsProxyModel::flags(const QModelIndex &index) const
{
    return QAbstractItemModel::flags(index);
}

QVariant GroupedContactsProxyModel::data(const QModelIndex &index, int role) const
{
    // If the model-index parent is invalid, then we have a group
    if (!index.parent().isValid()) {
        // Data only exists in Column 0.
        if (index.column() != 0) {
            return QVariant();
        }

        // Return the group data.
        QVariant data;
        Item *item = m_rootItem->children.at(index.row());

        Q_ASSERT(item);
        if (!item) {
            kWarning() << "NO ITEM!";
            return QVariant();
        }

        switch(role) {
        case Qt::DisplayRole:
            data.setValue<QString>(item->name);
            break;
        }

        return data;
    }

    // Return the data from the original model.
    return m_sourceModel->data(mapToSource(index), role);
}

QModelIndex GroupedContactsProxyModel::index(int row, int column, const QModelIndex &parent) const
{
//    kWarning() << "index called";
    if (parent.isValid() && parent.parent().isValid()) {
        return QModelIndex();
    }

    // Only 1 column
    if (column != 0) {
        return QModelIndex();
    }

    if (parent.isValid()) {
        if (row >= m_rootItem->children.at(parent.row())->children.length()) {
            return QModelIndex();
        }

        return createIndex(row, column, m_rootItem->children.at(parent.row())->children.at(row));
    }

    // Return the index to the item.
    if (row >= m_rootItem->children.length()) {
        return QModelIndex();
    }

    return createIndex(row, column, m_rootItem->children.at(row));
}

QModelIndex GroupedContactsProxyModel::parent(const QModelIndex &index) const
{
//    kWarning() << "Parent called. Argh";

    Item *item = static_cast<Item*>(index.internalPointer());

    Q_ASSERT(item);
    if (!item) {
        kWarning() << "Not a valid internal pointer. Argh :/";
        return QModelIndex();
    }

    if (item->parent == m_rootItem) {
        return QModelIndex();
    }

    return this->index(m_rootItem->children.lastIndexOf(item->parent), 0, QModelIndex());
}

int GroupedContactsProxyModel::rowCount(const QModelIndex &parent) const
{
    if (parent == QModelIndex()) {
        return m_rootItem->children.length();
    }

    if (parent.parent() == QModelIndex()) {
        Item *item = static_cast<Item*>(parent.internalPointer());

        Q_ASSERT(item);
        if (!item) {
            kWarning() << "Not a valid internal pointer. Argh :/";
            return 0;
        }

        return item->children.length();
    }

    return 0;
}

int GroupedContactsProxyModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return 1;
}

QModelIndex GroupedContactsProxyModel::mapFromSource(const QModelIndex &sourceIndex) const
{
    kWarning() << "Map From Source Called. Argh";

    Q_ASSERT(false);

    return sourceIndex;
}

QItemSelection GroupedContactsProxyModel::mapSelectionFromSource(const QItemSelection &sourceSelection) const
{
    return QAbstractProxyModel::mapSelectionFromSource(sourceSelection);
}

QItemSelection GroupedContactsProxyModel::mapSelectionToSource(const QItemSelection &proxySelection) const
{
    return QAbstractProxyModel::mapSelectionToSource(proxySelection);
}

QModelIndex GroupedContactsProxyModel::mapToSource(const QModelIndex &proxyIndex) const
{
    // Work out the appropriate index on the source model
    Item *item = static_cast<Item*>(proxyIndex.internalPointer());

    Q_ASSERT(item);
    if (!item) {
        kWarning() << "Invalid internal pointer :/";
        return QModelIndex();
    }
 
    return item->sourceIndex;
}

void GroupedContactsProxyModel::setSourceModel(QAbstractItemModel *sourceModel)
{
    m_sourceModel = qobject_cast<ContactsListModel*>(sourceModel);

    Q_ASSERT(m_sourceModel);
    if (!m_sourceModel) {
        kWarning() << "Invalid model supplied";
        return;
    }

    QAbstractProxyModel::setSourceModel(sourceModel);

    connect(sourceModel,
            SIGNAL(rowsInserted(QModelIndex,int,int)),
            SLOT(onSourceRowsInserted(QModelIndex,int,int)));
    connect(sourceModel,
            SIGNAL(rowsRemoved(QModelIndex,int,int)),
            SLOT(onSourceRowsRemoved(QModelIndex,int,int)));
    connect(sourceModel,
            SIGNAL(modelReset()),
            SLOT(onSourceReset()));

    // Synthesise invalidating all mappings and repopulating this proxy model.
    onSourceReset();
}

void GroupedContactsProxyModel::onSourceReset()
{
    kDebug() << "Source Reset";

    // Reset the internal data-layout of this model.
    if (m_rootItem) {
        foreach (Item *item, m_rootItem->children) {
            foreach (Item *i, item->children) {
                delete i;
            }
            delete item;
        }
        delete m_rootItem;
        m_rootItem = 0;
    }

    // Repopulate the proxy model data.
    m_rootItem = new Item;
    for (int i=0; i < m_sourceModel->rowCount(); ++i) {
        QModelIndex index = m_sourceModel->index(i, 0, QModelIndex());

        QStringList groups = m_sourceModel->data(index, ContactsListModel::GroupsRole).toStringList();

        foreach (const QString &group, groups) {
            Item *item = 0;
            foreach (Item *i, m_rootItem->children) {
                if (i->name == group) {
                    item = i;
                    break;
                }
            }

            if (!item) {
                item = new Item;
                item->parent = m_rootItem;
                m_rootItem->children.append(item);
                item->name = group;
            }

            Item *childItem = new Item;
            childItem->sourceIndex = index;
            item->children.append(childItem);
            childItem->parent = item;
        }
    }

    // Invalidate this model, so that the view re-queries it for stuff.
    reset();
}

void GroupedContactsProxyModel::onSourceRowsInserted(const QModelIndex &parent, int start, int end)
{
    kDebug() << "Rows Inserted";
    // TODO: Invalidate All Mappings
}

void GroupedContactsProxyModel::onSourceRowsRemoved(const QModelIndex &parent, int start, int end)
{
    kDebug() << "Rows Removed";
    // TODO: Invalidate All Mappings
}


#include "grouped-contacts-proxy-model.moc"

