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

#ifndef ACCOUNTFILTERMODEL_H
#define ACCOUNTFILTERMODEL_H

#include <QSortFilterProxyModel>

class AccountFilterModel : public QSortFilterProxyModel
{
public:
    AccountFilterModel(QObject *parent=0);

    void filterOfflineUsers(bool filterOfflineUsers);
    bool filterOfflineUsers() const;

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;

private:
    bool m_filterOfflineUsers;
};

#endif // ACCOUNTFILTERMODEL_H
