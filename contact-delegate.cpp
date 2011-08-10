/*
 * Contact Delegate
 *
 * Copyright (C) 2010-2011 Collabora Ltd. <info@collabora.co.uk>
 *   @Author Dario Freddi <dario.freddi@collabora.co.uk>
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

#include "contact-delegate.h"

#include <QtGui/QPainter>
#include <QtGui/QPainterPath>
#include <QApplication>
#include <QStyle>

#include <KIconLoader>
#include <KIcon>
#include <KDebug>
#include <KGlobalSettings>
#include <KDE/KLocale>

#include "models/accounts-model.h"
#include "models/contact-model-item.h"
#include "models/proxy-tree-node.h"
#include "models/groups-model-item.h"
#include "models/groups-model.h"

const int SPACING = 4;
const int AVATAR_SIZE = 32;
const int PRESENCE_ICON_SIZE = 22;
const int ACCOUNT_ICON_SIZE = 13;

ContactDelegate::ContactDelegate(QObject * parent)
    : AbstractContactDelegate(parent)
{

}

ContactDelegate::~ContactDelegate()
{

}

void ContactDelegate::paintContact(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    QStyleOptionViewItemV4 optV4 = option;
    initStyleOption(&optV4, index);

    painter->save();

    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::HighQualityAntialiasing);
    painter->setClipRect(optV4.rect);

    QStyle *style = QApplication::style();
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter);

    QRect iconRect = optV4.rect;
    iconRect.setSize(QSize(AVATAR_SIZE, AVATAR_SIZE));
    iconRect.moveTo(QPoint(iconRect.x() + SPACING, iconRect.y() + SPACING));

    QPixmap avatar;
    avatar.load(index.data(AccountsModel::AvatarRole).toString());

    bool noContactAvatar = avatar.isNull();

    if (noContactAvatar) {
        avatar = SmallIcon("im-user", KIconLoader::SizeMedium);
    }

    QPainterPath roundedPath;
    roundedPath.addRoundedRect(iconRect, 20, 20, Qt::RelativeSize);

    if (!noContactAvatar) {
        painter->save();
        painter->setClipPath(roundedPath);
    }

    painter->drawPixmap(iconRect, avatar);

    if (!noContactAvatar) {
        painter->restore();
        painter->drawPath(roundedPath);
    }

    QPixmap icon;

    switch (index.data(AccountsModel::PresenceTypeRole).toInt()) {
    case Tp::ConnectionPresenceTypeAvailable:
        icon = SmallIcon("user-online", KIconLoader::SizeSmallMedium);
        break;
    case Tp::ConnectionPresenceTypeAway:
        icon = SmallIcon("user-away", KIconLoader::SizeSmallMedium);
        break;
    case Tp::ConnectionPresenceTypeExtendedAway:
        icon = SmallIcon("user-away-extended", KIconLoader::SizeSmallMedium);
        break;
    case Tp::ConnectionPresenceTypeBusy:
        icon = SmallIcon("user-busy", KIconLoader::SizeSmallMedium);
        break;
    case Tp::ConnectionPresenceTypeHidden:
        icon = SmallIcon("user-invisible", KIconLoader::SizeSmallMedium);
        break;
    case Tp::ConnectionPresenceTypeOffline:
        icon = SmallIcon("user-offline", KIconLoader::SizeSmallMedium);
        break;
    default:
        icon = SmallIcon("task-attention", KIconLoader::SizeSmallMedium);
        break;
    }

    QRect statusIconRect = optV4.rect;
    statusIconRect.setSize(QSize(PRESENCE_ICON_SIZE, PRESENCE_ICON_SIZE));
    statusIconRect.moveTo(QPoint(optV4.rect.right() - PRESENCE_ICON_SIZE - SPACING,
                                 optV4.rect.top() + (optV4.rect.height() - PRESENCE_ICON_SIZE) / 2));

    painter->drawPixmap(statusIconRect, icon);

    QRect userNameRect = optV4.rect;
    userNameRect.setX(iconRect.x() + iconRect.width() + SPACING);
    userNameRect.setY(userNameRect.y() + 3);
    userNameRect.setWidth(userNameRect.width() - PRESENCE_ICON_SIZE - SPACING);

    QFont nameFont = KGlobalSettings::smallestReadableFont();
    nameFont.setPointSize(nameFont.pointSize() + 1);
    nameFont.setWeight(QFont::Bold);

    const QFontMetrics nameFontMetrics(nameFont);

    painter->setFont(nameFont);
    painter->drawText(userNameRect,
                      nameFontMetrics.elidedText(optV4.text, Qt::ElideRight, userNameRect.width()));

    QRect statusMsgRect = optV4.rect;
    statusMsgRect.setX(iconRect.x() + iconRect.width() + SPACING);
    statusMsgRect.setY(userNameRect.top() + 16);
    statusMsgRect.setWidth(statusMsgRect.width() - PRESENCE_ICON_SIZE - SPACING);

    QFont statusFont = KGlobalSettings::smallestReadableFont();

    const QFontMetrics statusFontMetrics(statusFont);

    QColor fadingColor(m_palette->color(QPalette::WindowText));

    if (index == m_indexForHiding) {
        fadingColor.setAlpha(m_fadingValue);
        painter->setPen(fadingColor);
    }

    painter->setFont(statusFont);
    painter->drawText(statusMsgRect,
                      statusFontMetrics.elidedText(index.data(AccountsModel::PresenceMessageRole).toString(),
                                                   Qt::ElideRight, statusMsgRect.width()));


    painter->restore();
}

QSize ContactDelegate::sizeHintContact(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    return QSize(0, 32 + 4 * SPACING);
}

void ContactDelegate::hideStatusMessageSlot(const QModelIndex& index)
{
    m_indexForHiding = index;
    fadeOutStatusMessageSlot();
}

void ContactDelegate::reshowStatusMessageSlot()
{
    m_fadingValue = 255;
    m_indexForHiding = QModelIndex();
    emit repaintItem(m_indexForHiding);
}

void ContactDelegate::fadeOutStatusMessageSlot()
{
    QPropertyAnimation *a = new QPropertyAnimation(this, "m_fadingValue");
    a->setParent(this);
    a->setDuration(100);
    a->setEasingCurve(QEasingCurve::OutExpo);
    a->setStartValue(255);
    a->setEndValue(0);
    a->start();

    connect(a, SIGNAL(valueChanged(QVariant)),
            this, SLOT(triggerRepaint()));
}

int ContactDelegate::fadingValue() const
{
    return m_fadingValue;
}

void ContactDelegate::setFadingValue(int value)
{
    m_fadingValue = value;
}

void ContactDelegate::triggerRepaint()
{
    emit repaintItem(m_indexForHiding);
}

#include "contact-delegate.moc"
