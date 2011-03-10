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
    Q_OBJECT
    
public:
    AccountFilterModel(QObject *parent=0);

    bool filterOfflineUsers() const;
    
public slots:
    void filterOfflineUsers(bool filterOfflineUsers);
    void setFilterString(const QString &str);
    void clearFilterString();
    
protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;

private:
    /// Filters out offline users
    bool m_filterOfflineUsers;
    
    /// Used when searching for contact
    bool m_filterByName;
    
    /// Holds the string which is searched in the model
    QString m_filterString;
};

#endif // ACCOUNTFILTERMODEL_H
