/*
    <one line to give the library's name and an idea of what it does.>
    Copyright (C) 2012  Martin Klapetek <email>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


#include "kpeople-proxy.h"
#include <kpeople/persons-model.h>
#include <KDebug>

KPeopleProxy::KPeopleProxy(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

KPeopleProxy::~KPeopleProxy()
{

}

bool KPeopleProxy::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    QModelIndex i = sourceModel()->index(source_row, 0, source_parent);
    if ((PersonsModel::ResourceType)sourceModel()->data(i, PersonsModel::ResourceTypeRole).toInt() == PersonsModel::Contact) {
        if (!sourceModel()->data(i, PersonsModel::IMRole).isNull()) {
            return true;
        }
    } else {
        QVariantList l = sourceModel()->data(i, PersonsModel::IMRole).toList();
        if (!l.isEmpty()) {
            return true;
        }
    }

    return false;
}

QVariant KPeopleProxy::data(const QModelIndex &index, int role) const
{
    if (role == PersonsModel::ContactsCountRole) {
        if (index.parent().isValid() || (!index.parent().isValid() && rowCount(index) == 0)) {
            return 1;
        } else {
            return rowCount(index);
        }
    }

    return QSortFilterProxyModel::data(index, role);
}
