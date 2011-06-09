/*
 * Provide some filters on the account model
 *
 * Copyright (C) 2011 David Edmundson <kde@davidedmundson.co.uk>
 * Copyright (C) 2011 Martin Klapetek <martin dot klapetek at gmail dot com>
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

#include "account-filter-model.h"
#include "accounts-model.h"
#include "groups-model.h"

AccountFilterModel::AccountFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent),
      m_filterOfflineUsers(false),
      m_filterByName(false)
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
    bool rowAccepted = true;
    //if we're looking at filtering an account or not
    if (source_parent != QModelIndex()) {
        //filter by name in the contact list
        if (m_filterByName &&
                !source_parent.child(source_row, 0).data(AccountsModel::AliasRole).toString()
                .contains(m_filterString, Qt::CaseInsensitive)) {

            rowAccepted = false;
        }

        //filter offline users out
        if (m_filterOfflineUsers &&
                ((source_parent.child(source_row, 0).data(AccountsModel::PresenceTypeRole).toUInt()
                == Tp::ConnectionPresenceTypeOffline) ||
                (source_parent.child(source_row, 0).data(AccountsModel::PresenceTypeRole).toUInt()
                == Tp::ConnectionPresenceTypeUnknown))) {

            rowAccepted = false;
        }
    } else {
        QModelIndex index = sourceModel()->index(source_row, 0);
        if (index.isValid()) {
            if (m_groupsActive) {
                if (m_filterOfflineUsers) {
                    if (index.data(AccountsModel::OnlineUsersCountRole).toInt() > 0) {
                        return true;
                    } else {
                        return false;
                    }
                } else {
                    //if the offline users are shown, display all the groups
                    return true;
                }
            } else {
                if (!index.data(AccountsModel::EnabledRole).toBool()) {
                    rowAccepted = false;
                }
                if (index.data(AccountsModel::ConnectionStatusRole).toUInt()
                    != Tp::ConnectionStatusConnected) {

                    rowAccepted = false;
                }
            }
        }
    }

    return rowAccepted;
}

void AccountFilterModel::setFilterString(const QString &str)
{
    m_filterString = str;
    m_filterByName = true;
    invalidateFilter();
}

void AccountFilterModel::clearFilterString()
{
    m_filterString.clear();
    m_filterByName = false;
    invalidateFilter();
}

bool AccountFilterModel::lessThan( const QModelIndex &left, const QModelIndex &right ) const
{
        uint leftPresence;
        uint rightPresence;

        QString leftDisplayedName = sourceModel()->data(left).toString();
        QString rightDisplayedName = sourceModel()->data(right).toString();

        if (sortRole() == AccountsModel::PresenceTypeRole) {
            leftPresence = sourceModel()->data(left, AccountsModel::PresenceTypeRole).toUInt();
            rightPresence = sourceModel()->data(right, AccountsModel::PresenceTypeRole).toUInt();

            if (leftPresence == rightPresence) {
                return QString::localeAwareCompare(leftDisplayedName, rightDisplayedName) < 0;
            } else {
                if (leftPresence == Tp::ConnectionPresenceTypeAvailable) {
                    return true;
                }
                if (leftPresence == Tp::ConnectionPresenceTypeUnset ||
                        leftPresence == Tp::ConnectionPresenceTypeOffline ||
                        leftPresence == Tp::ConnectionPresenceTypeUnknown ||
                        leftPresence == Tp::ConnectionPresenceTypeError) {
                    return false;
                }

                return leftPresence < rightPresence;
            }
        } else {
            return QString::localeAwareCompare(leftDisplayedName, rightDisplayedName) < 0;
        }
}

void AccountFilterModel::setSortByPresence(bool enabled)
{
    if (enabled) {
        setSortRole(AccountsModel::PresenceTypeRole);
    } else {
        setSortRole(Qt::DisplayRole);
    }
}

bool AccountFilterModel::isSortedByPresence() const
{
    return sortRole() == AccountsModel::PresenceTypeRole;
}

bool AccountFilterModel::groupsActive() const
{
    return m_groupsActive;
}

void AccountFilterModel::setGroupsActive(bool active)
{
    m_groupsActive = active;
}

#include "account-filter-model.moc"
