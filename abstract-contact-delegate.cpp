/*
 * Abstract Contact Delegate - base class for other delegates
 *
 * Copyright (C) 2011 Martin Klapetek <martin.klapetek@gmail.com>
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

#include "abstract-contact-delegate.h"

#include <QtGui/QApplication>
#include <QtGui/QStyle>
#include <QtGui/QPainter>
#include <QtGui/QToolTip>
#include <QtGui/QHelpEvent>
#include <QAbstractItemView>

#include <KDE/KGlobalSettings>
#include <KDE/KLocale>
#include <KDE/KIconLoader>
#include <KDE/KIcon>

#include <KTp/types.h>

const int SPACING = 4;
const int ACCOUNT_ICON_SIZE = 22;
const qreal GROUP_ICON_OPACITY = 0.6;

AbstractContactDelegate::AbstractContactDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

AbstractContactDelegate::~AbstractContactDelegate()
{
}

void AbstractContactDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.data(KTp::RowTypeRole).toInt() == KTp::ContactRowType) {
        paintContact(painter, option, index);
    } else {
        paintHeader(painter, option, index);
    }
}

QSize AbstractContactDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);

    if (index.data(KTp::RowTypeRole).toInt() == KTp::ContactRowType) {
        return sizeHintContact(option, index);
    } else {
        return sizeHintHeader(option, index);
    }
}


void AbstractContactDelegate::paintHeader(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItemV4 optV4 = option;
    initStyleOption(&optV4, index);

    painter->save();

    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::HighQualityAntialiasing);
    painter->setClipRect(optV4.rect);

    QStyle *style = QApplication::style();
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter);

    QRect groupRect = optV4.rect;

    //paint the background
    QBrush bgBrush(option.palette.color(QPalette::Active, QPalette::Button).lighter(105));
    painter->fillRect(groupRect, bgBrush);

    //paint very subtle line at the bottom
    QPen thinLinePen;
    thinLinePen.setWidth(0);
    thinLinePen.setColor(option.palette.color(QPalette::Active, QPalette::Button));
    painter->setPen(thinLinePen);
    //to get nice sharp 1px line we need to turn AA off, otherwise it will be all blurry
    painter->setRenderHint(QPainter::Antialiasing, false);
    painter->drawLine(groupRect.bottomLeft(), groupRect.bottomRight());
    painter->setRenderHint(QPainter::Antialiasing, true);

    //remove spacing from the sides and one point to the bottom for the 1px line
    groupRect.adjust(SPACING, 0, -SPACING, -1);

    //get the proper rect for the expand sign
    int iconSize = IconSize(KIconLoader::Toolbar);

    QStyleOption expandSignOption = option;
    expandSignOption.rect = groupRect;
    expandSignOption.rect.setSize(QSize(iconSize, iconSize));
    expandSignOption.rect.moveLeft(groupRect.left());
    expandSignOption.rect.moveTop(groupRect.top() + groupRect.height()/2 - expandSignOption.rect.height()/2);

    //paint the expand sign
    if (option.state & QStyle::State_Open) {
        style->drawPrimitive(QStyle::PE_IndicatorArrowDown, &expandSignOption, painter);
    } else {
        style->drawPrimitive(QStyle::PE_IndicatorArrowRight, &expandSignOption, painter);
    }

    QFont groupFont = KGlobalSettings::smallestReadableFont();

    //paint the group icon
    QRect groupIconRect;
    groupIconRect.setSize(QSize(ACCOUNT_ICON_SIZE, ACCOUNT_ICON_SIZE));
    groupIconRect.moveRight(groupRect.right());
    groupIconRect.moveTop(groupRect.top() + groupRect.height()/2 - groupIconRect.height()/2);

    if (index.data(KTp::RowTypeRole).toInt() == KTp::AccountRowType) {
        //draw the icon with some opacity
        painter->setOpacity(GROUP_ICON_OPACITY);
        painter->drawPixmap(groupIconRect, KIcon(index.data(Qt::DecorationRole).value<QIcon>()).pixmap(ACCOUNT_ICON_SIZE));
        painter->setOpacity(1.0);
    } else {
        groupIconRect.setWidth(0);
    }

    //paint the header string
    QRect groupLabelRect = groupRect.adjusted(expandSignOption.rect.width() + SPACING * 2, 0, -groupIconRect.width() -SPACING, 0);
    QString countsString = QString("(%1/%2)").arg(index.data(KTp::HeaderOnlineUsersRole).toString(),
                                                  index.data(KTp::HeaderTotalUsersRole).toString());

    QString groupHeaderString =  index.data(Qt::DisplayRole).toString();

    QFontMetrics groupFontMetrics(groupFont);

    painter->setFont(groupFont);
    if (index.data(KTp::HeaderTotalUsersRole).toInt() > 0) {
        painter->setPen(option.palette.color(QPalette::Disabled, QPalette::Text));
        painter->drawText(groupLabelRect, Qt::AlignVCenter | Qt::AlignRight, countsString);
    }

    painter->setPen(option.palette.color(QPalette::Active, QPalette::Text));
    painter->drawText(groupLabelRect, Qt::AlignVCenter | Qt::AlignLeft,
                      groupFontMetrics.elidedText(groupHeaderString, Qt::ElideRight,
                                                  groupLabelRect.width() - groupFontMetrics.width(countsString) - SPACING));

    painter->restore();
}

QSize AbstractContactDelegate::sizeHintHeader(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)

    // Add one point to the bottom for the 1px line
    return QSize(0, qMax(ACCOUNT_ICON_SIZE, KGlobalSettings::smallestReadableFont().pixelSize()) + SPACING + 1);
}

bool AbstractContactDelegate::helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    Q_UNUSED(event)
    Q_UNUSED(view)
    Q_UNUSED(option)
    Q_UNUSED(index)
    return false;
}
