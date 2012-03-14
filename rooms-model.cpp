/*
 * Rooms Model - A model of chatrooms.
 * Copyright (C) 2012  Dominik Cermak <d.cermak@arcor.de>
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
 */

#include "rooms-model.h"
#include <KIcon>
#include <KLocale>

// RoomsModel
RoomsModel::RoomsModel(QObject *parent): QAbstractListModel(parent)
{
}

int RoomsModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    } else {
        return m_roomInfoList.size();
    }
}

int RoomsModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    } else {
        return 4;
    }
}

QVariant RoomsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (index.row() >= m_roomInfoList.count()) {
        return QVariant();
    }

    const int row = index.row();
    const Tp::RoomInfo &roomInfo = m_roomInfoList.at(row);

    // this is handled here because when putting it in the switch below
    // all columns get an empty space for the decoration
    if (index.column() == PasswordColumn) {
        switch (role) {
        case Qt::DecorationRole:
            if (roomInfo.info.value("password").toBool()) {
                return KIcon("object-locked");
            } else {
                return QVariant();
            }
        case Qt::ToolTipRole:
            if (roomInfo.info.value("password").toBool()) {
                return i18n("Password required");
            } else {
                return i18n("No password required");
            }
        }
    }

    switch(role) {
    case Qt::DisplayRole:
        switch (index.column()) {
        case PasswordColumn:
            return QVariant();
        case NameColumn:
            return roomInfo.info.value(QString("name"));
        case DescriptionColumn:
            return roomInfo.info.value(QString("description"));
        case MembersColumn:
            return roomInfo.info.value(QString("members"));
        }
    case Qt::ToolTipRole:
        switch (index.column()) {
        case MembersColumn:
            return i18n("Member count");
        }
    case RoomsModel::HandleNameRole:
        return roomInfo.info.value(QString("handle-name"));
    }

    return QVariant();
}

QVariant RoomsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole && role != Qt::DecorationRole) {
        return QVariant();
    }

    if (orientation == Qt::Horizontal) {
        switch (role) {
        case Qt::DisplayRole:
            switch (section) {
            case NameColumn:
                return i18nc("Chatrooms name", "Name");
            case DescriptionColumn:
                return i18nc("Chatrooms description", "Description");
            }
        case Qt::DecorationRole:
            switch (section) {
            case PasswordColumn:
                return KIcon("object-locked");
            case MembersColumn:
                return KIcon("meeting-participant");
            }
        }
    }

    return QVariant();
}

void RoomsModel::addRooms(const Tp::RoomInfoList newRoomList)
{
    if (newRoomList.size() > 0) {
        beginInsertRows(QModelIndex(), m_roomInfoList.size(), m_roomInfoList.size() + newRoomList.size() - 1);
        m_roomInfoList.append(newRoomList);
        endInsertRows();
    }
}

void RoomsModel::clearRoomInfoList()
{
    if (m_roomInfoList.size() > 0) {
        beginRemoveRows(QModelIndex(), 0, m_roomInfoList.size() - 1);
        m_roomInfoList.clear();
        endRemoveRows();
    }
}

// FavoriteRoomsModel
FavoriteRoomsModel::FavoriteRoomsModel(QObject *parent): QAbstractListModel(parent)
{
}

int FavoriteRoomsModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    } else {
        return m_favoriteRoomsList.size();
    }
}

int FavoriteRoomsModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    } else {
        return 3;
    }
}

QVariant FavoriteRoomsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (index.row() >= m_favoriteRoomsList.size()) {
        return QVariant();
    }

    const int row = index.row();
    const QVariantMap &room = m_favoriteRoomsList.at(row);

    switch(role) {
    case Qt::DisplayRole:
        switch (index.column()) {
        case NameColumn:
            return room.value("name");
        case HandleNameColumn:
            return room.value("handle-name");
        case AccountIdentifierColumn:
            return room.value("account-identifier");
        }
    case Qt::ToolTipRole:
        return room.value("handle-name");
    case FavoriteRoomsModel::HandleNameRole:
        return room.value("handle-name");
    case FavoriteRoomsModel::FavoriteRoomRole:
        return QVariant::fromValue<QVariantMap>(room);
    }

    return QVariant();
}

void FavoriteRoomsModel::addRooms(const QList<QVariantMap> newRoomList)
{
    if (newRoomList.size() > 0) {
        beginInsertRows(QModelIndex(), m_favoriteRoomsList.size(), m_favoriteRoomsList.size() + newRoomList.size() - 1);
        m_favoriteRoomsList.append(newRoomList);
        endInsertRows();
    }
}

void FavoriteRoomsModel::addRoom(const QVariantMap &room)
{
    beginInsertRows(QModelIndex(), m_favoriteRoomsList.size(), m_favoriteRoomsList.size());
    m_favoriteRoomsList.append(room);
    endInsertRows();
}

void FavoriteRoomsModel::removeRoom(const QVariantMap &room)
{
    int row = m_favoriteRoomsList.indexOf(room);
    beginRemoveRows(QModelIndex(), row, row);
    m_favoriteRoomsList.removeOne(room);
    endRemoveRows();
}

bool FavoriteRoomsModel::containsRoom(const QString &handle, const QString &account) const
{
    bool contains = false;

    Q_FOREACH(const QVariantMap &room, m_favoriteRoomsList) {
        if ((room.value("handle-name") == handle) && (room.value("account-identifier") == account)) {
            contains = true;
        }
    }

    return contains;
}

int FavoriteRoomsModel::countForAccount(const QString &account) const
{
    int count = 0;

    Q_FOREACH (const QVariantMap &room, m_favoriteRoomsList) {
        if (room.value("account-identifier") == account) {
            count++;
        }
    }

    return count;
}
