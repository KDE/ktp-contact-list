/*
 * Provide some filters on the account model
 *
 * Copyright (C) 2011 David Edmundson <kde@davidedmundson.co.uk>
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

#include "accountfiltermodel.h"
#include "accounts-model.h"

AccountFilterModel::AccountFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent),
      m_filterOfflineUsers(false)
{

}

void AccountFilterModel::filterOfflineUsers(bool filterOfflineUsers)
{
    m_filterOfflineUsers = filterOfflineUsers;
    invalidateFilter();
}

bool AccountFilterModel::filterOfflineUsers() const
{
    return m_filterOfflineUsers;
}

bool AccountFilterModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    //if we're looking at filtering an account or not
    if (source_parent != QModelIndex()) {
        if (m_filterOfflineUsers &&
            (source_parent.child(source_row, 0).data(AccountsModel::PresenceTypeRole).toUInt()
                == Tp::ConnectionPresenceTypeOffline) ||
            (source_parent.child(source_row, 0).data(AccountsModel::PresenceTypeRole).toUInt()
                == Tp::ConnectionPresenceTypeUnknown)) {
            
                return false;
        }
    }

    return true;
}
