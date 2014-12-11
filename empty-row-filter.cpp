/*
 * Copyright (C) 2014  David Edmundson <david@davidedmundson.co.uk>
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "empty-row-filter.h"

#include <KTp/types.h>

EmptyRowFilter::EmptyRowFilter(QObject *parent):
    QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(true);
}

bool EmptyRowFilter::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    const QModelIndex &index = sourceModel()->index(sourceRow, 0, sourceParent);
    if (index.data(KTp::RowTypeRole).toInt() == KTp::GroupRowType) {
        return sourceModel()->rowCount(index) > 0;
    }

    //hide the expanded view for single contacts
    if (sourceParent.parent().isValid() && sourceParent.data(KTp::RowTypeRole).toInt() == KTp::ContactRowType) {
        return false;
    }

    return true;
}
