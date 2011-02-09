/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Martin Klapetek <email>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/


#include "contactsmodelfilter.h"
#include "fakecontactsmodel.h"

bool ContactsModelFilter::filterAcceptsRow ( int source_row, const QModelIndex& source_parent ) const
{
    QModelIndex target = sourceModel()->index(source_row, 0, source_parent);
    Tp::ConnectionPresenceType presence = sourceModel()->data(target, ModelRoles::UserStatusRole).value<Tp::ConnectionPresenceType>();
    
    switch (presence) {
        case Tp::ConnectionPresenceTypeUnset:
        case Tp::ConnectionPresenceTypeOffline:
        case Tp::ConnectionPresenceTypeHidden :
        case Tp::ConnectionPresenceTypeUnknown:
            return false;
        default:
            return true;
    }
}

ContactsModelFilter::ContactsModelFilter(QObject *parent) : QSortFilterProxyModel(parent)
{

}

ContactsModelFilter::~ContactsModelFilter()
{

}

