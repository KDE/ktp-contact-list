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

#ifndef ROOMSMODEL_H
#define ROOMSMODEL_H

#include <QtCore/QAbstractListModel>
#include <TelepathyQt/Types>

class RoomsModel : public QAbstractListModel
{
    Q_OBJECT

public:
    // TODO: find a suitable icon and add an invitation column
    enum Column {
        PasswordColumn=0,
        MembersColumn,
        NameColumn,
        DescriptionColumn
    };

    enum Roles {
        HandleNameRole = Qt::UserRole
    };

    explicit RoomsModel(QObject *parent = 0);
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    /**
     * \brief Add new rooms to the list.
     *
     * \param newRoomList The list with the new rooms to add.
     */
    void addRooms(const Tp::RoomInfoList newRoomList);

    /**
     * \brief Clear the room list.
     */
    void clearRoomInfoList();

private:
    Tp::RoomInfoList m_roomInfoList;
};

#endif // ROOMSMODEL_H
