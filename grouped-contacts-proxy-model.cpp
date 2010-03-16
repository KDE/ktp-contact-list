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
        Item *item = dynamic_cast<Item*>(m_rootItem->childItems().at(index.row()));

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
    // 1 column list, so invalid index if the column is not 1.
    if (parent.isValid() && parent.column() != 0) {
        return QModelIndex();
    }

    // Get the parent item.
    Item *parentItem = item(parent);

    // Get all the parent's children.
    QList<AbstractTreeItem*> children = parentItem->childItems();

    // Check the row doesn't go beyond the end of the list of children.
    if (row >= children.length()) {
        return QModelIndex();
    }

    // Return the index to the item.
    return createIndex(row, column, children.at(row));
}

QModelIndex GroupedContactsProxyModel::parent(const QModelIndex &index) const
{
    // If the index is invalid, return an invalid parent index.
    if (!index.isValid()) {
        return QModelIndex();
    }

    // Get the item we have been passed, and it's parent
    Item *childItem = item(index);
    Item *parentItem = dynamic_cast<Item*>(childItem->parentItem());

    // If the parent is the root item, then the parent index of the index we were passed is
    // by definition an invalid index.
    if (parentItem == m_rootItem) {
        return QModelIndex();
    }

    // The parent of the item is not the root item, meaning that the parent must have a parent too.
    Item *parentOfParentItem = dynamic_cast<Item*>(parentItem->parentItem());

    // As stated in the previous comment, something is really wrong if it doesn't have a parent.
    Q_ASSERT(parentOfParentItem);
    if (!parentOfParentItem) {
        kWarning() << "Impossible parent situation occurred!";
        return createIndex(0, 0, parentItem);
    }

    // Return the model index of the parent item.
    return createIndex(parentOfParentItem->childItems().lastIndexOf(parentItem), 0, parentItem);
}

int GroupedContactsProxyModel::rowCount(const QModelIndex &parent) const
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
        foreach (AbstractTreeItem *item, m_rootItem->childItems()) {
            foreach (AbstractTreeItem *i, item->childItems()) {
                foreach (AbstractTreeItem *i3, i->childItems()) {
                    delete i3;
                }
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
            foreach (AbstractTreeItem *ii, (m_rootItem->childItems())) {
                Item *i = dynamic_cast<Item*>(ii);
                if (!i) {
                    continue;
                }

                if (i->name == group) {
                    item = i;
                    break;
                }
            }

            if (!item) {
                item = new Item;
                item->setParentItem(m_rootItem);
                m_rootItem->appendChildItem(item);
                item->name = group;
            }

            Item *childItem = new Item;
            childItem->sourceIndex = index;
            item->appendChildItem(childItem);
            childItem->setParentItem(item);

            for (int j=0; j < m_sourceModel->rowCount(index); ++j) {
                QModelIndex childIndex = m_sourceModel->index(j, 0, index);
                Item *childChildItem = new Item;
                childChildItem->sourceIndex = childIndex;
                childChildItem->setParentItem(childItem);
                childItem->appendChildItem(childChildItem);
            }
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

GroupedContactsProxyModel::Item* GroupedContactsProxyModel::item(const QModelIndex &index) const
{
    if (index.isValid()) {
        Item *item = static_cast<Item*>(index.internalPointer());
         if (item) {
             return item;
         }
     }

     return m_rootItem;
}


#include "grouped-contacts-proxy-model.moc"

