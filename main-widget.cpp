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

#include "main-widget.h"

#include "contacts-list-model.h"

#include <KDebug>
#include <QtGui/QSortFilterProxyModel>

MainWidget::MainWidget(QWidget *parent)
 : QWidget(parent),
   m_model(0)
{
    kDebug();

    setupUi(this);

    m_model = new ContactsListModel(this);
    m_sortFilterProxyModel = new QSortFilterProxyModel(this);
    m_sortFilterProxyModel->setSourceModel(m_model);
    m_contactsListView->setSortingEnabled(true);
    m_contactsListView->setModel(m_sortFilterProxyModel);
}

MainWidget::~MainWidget()
{
    kDebug();
}


#include "main-widget.moc"

