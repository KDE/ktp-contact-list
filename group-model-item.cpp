/*
 * Group model item, represents a group in the contactlist tree
 *
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

#include "group-model-item.h"
#include "accounts-model.h"

struct GroupModelItem::Private
{
    Private(const QString &groupName)
    : mGroupName(groupName)
    {
    }

    QString mGroupName;
};

GroupModelItem::GroupModelItem(const QString &groupName)
    : mPriv(new Private(groupName))
{
}

GroupModelItem::~GroupModelItem()
{

}

QVariant GroupModelItem::data(int role) const
{
    switch(role) {
    case AccountsModel::GroupNameRole:
        return mPriv->mGroupName;
    default:
        return QVariant();
    }
}

bool GroupModelItem::setData(int role, const QVariant& value)
{
    switch(role) {
    case AccountsModel::GroupNameRole:
        //mPriv->mGroupName = value.toString();
        return true;
    default:
        return false;
    }
}