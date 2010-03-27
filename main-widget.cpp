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
#include "grouped-contacts-proxy-model.h"

#include <KDebug>
#include <QtGui/QSortFilterProxyModel>
#include <QtGui/QPainter>

const int SPACING = 4;
const int AVATAR_SIZE = 32;

ContactDelegate::ContactDelegate(QObject * parent)
  : QStyledItemDelegate(parent)
{
}

ContactDelegate::~ContactDelegate()
{
}

void ContactDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & idx) const
{
    QStyleOptionViewItemV4 optV4 = option;
    initStyleOption(&optV4, idx);

    painter->save();

    painter->setClipRect(optV4.rect);

    QStyle *style = QApplication::style();
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter);

    QRect iconRect = optV4.rect;
    iconRect.setSize(QSize(32, 32));
    iconRect.moveTo(QPoint(iconRect.x() + SPACING, iconRect.y() + SPACING));

    const QPixmap pixmap = idx.data(ContactsListModel::AvatarRole).value<QPixmap>();
    if (!pixmap.isNull()) {
        painter->drawPixmap(iconRect, idx.data(ContactsListModel::AvatarRole).value<QPixmap>());
    }

    painter->save();

    QFont nameFont = painter->font();
    nameFont.setWeight(QFont::Bold);
    painter->setFont(nameFont);

    QRect textRect = optV4.rect;
    textRect.setX(iconRect.x() + iconRect.width() + SPACING);
    textRect = painter->boundingRect(textRect, Qt::AlignLeft | Qt::AlignTop, optV4.text);
    //textRect.setWidth(optV4.rect.width() / 2);

    painter->drawText(textRect, optV4.text);

    painter->restore();

//     QRect typeRect;
// 
//     typeRect = painter->boundingRect(optV4.rect, Qt::AlignLeft | Qt::AlignBottom, idx.data(51).toString());
//     typeRect.moveTo(QPoint(typeRect.x() + iconRect.x() + iconRect.width() + SPACING, typeRect.y() - SPACING));
//     painter->drawText(typeRect, idx.data(51).toString());
// 
//     QRect sizeRect = painter->boundingRect(optV4.rect, Qt::AlignRight | Qt::AlignTop, idx.data(50).toString());
//     sizeRect.moveTo(QPoint(sizeRect.x() - SPACING, sizeRect.y() + SPACING));
//     painter->drawText(sizeRect, idx.data(50).toString());

    painter->restore();
}

QSize ContactDelegate::sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    return QSize(0, 32 + 2 * SPACING);
}

MainWidget::MainWidget(QWidget *parent)
 : QWidget(parent),
   m_model(0),
   m_groupedContactsProxyModel(0),
   m_sortFilterProxyModel(0)
{
    kDebug();

    setupUi(this);

    m_model = new ContactsListModel(this);
    m_sortFilterProxyModel = new QSortFilterProxyModel(this);
    m_sortFilterProxyModel->setSourceModel(m_model);

    m_groupedContactsProxyModel = new GroupedContactsProxyModel(this);
    m_groupedContactsProxyModel->setSourceModel(m_model);

    m_contactsListView->setSortingEnabled(true);
    m_contactsListView->setModel(m_groupedContactsProxyModel);
    m_contactsListView->setItemDelegate(new ContactDelegate(this));
}

MainWidget::~MainWidget()
{
    kDebug();
}


#include "main-widget.moc"

