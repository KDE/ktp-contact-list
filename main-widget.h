/*
 * This file is part of telepathy-contactslist-prototype
 *
 * Copyright (C) 2009-2010 Collabora Ltd. <info@collabora.co.uk>
 *   @Author George Goldberg <george.goldberg@collabora.co.uk>
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

#ifndef TELEPATHY_CONTACTSLIST_PROTOTYPE_MAIN_WIDGET_H
#define TELEPATHY_CONTACTSLIST_PROTOTYPE_MAIN_WIDGET_H

#include "ui_main-widget.h"

#include <personcontact.h>

#include <QtGui/QWidget>
#include <QtGui/QStyledItemDelegate>

class ContactsListModel;
class GroupedContactsProxyModel;
class QSortFilterProxyModel;

class ContactDelegate : public QStyledItemDelegate
{
    Q_OBJECT
    public:
        ContactDelegate(QObject * parent = 0);
        ~ContactDelegate();

        virtual void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const;
        virtual QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const;
};

class MainWidget : public QWidget, Ui::MainWidget
{
    Q_OBJECT

public:
    MainWidget(QWidget *parent = 0);
    ~MainWidget();

private:
    ContactsListModel *m_model;
    GroupedContactsProxyModel *m_groupedContactsProxyModel;
    QSortFilterProxyModel *m_sortFilterProxyModel;
    Nepomuk::PersonContact m_mePersonContact;

public slots:
    void onCustomContextMenuRequested(const QPoint &point);
    void onRequestRemoveFromGroup(bool);
    void onContactRemovalRequest(bool);
    void onContactBlockRequest(bool);
    void onHandlerReady(bool);
    void onRequestAddToGroup(bool);
    void onAddContactRequest(bool);
};


#endif // Header guard

