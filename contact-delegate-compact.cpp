/*
 * Contact Delegate - compact version
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

#include "contact-delegate-compact.h"

#include <QtGui/QPainter>
#include <QtGui/QPainterPath>
#include <QtGui/QToolTip>
#include <QApplication>
#include <QStyle>
#include <QHelpEvent>

#include <KIconLoader>
#include <KIcon>
#include <KDebug>
#include <KGlobalSettings>
#include <KDE/KLocale>

#include <KTp/Models/accounts-model.h>
#include <KTp/Models/contact-model-item.h>
#include <KTp/Models/proxy-tree-node.h>
#include <KTp/Models/groups-model-item.h>
#include <KTp/Models/groups-model.h>
#include <KTp/presence.h>

const int SPACING = 4;
const int AVATAR_SIZE = 22;
const int PRESENCE_ICON_SIZE = 16;
const int ACCOUNT_ICON_SIZE = 13;

ContactDelegateCompact::ContactDelegateCompact(QObject * parent)
    : AbstractContactDelegate(parent)
{
}

ContactDelegateCompact::~ContactDelegateCompact()
{

}

void ContactDelegateCompact::paintContact(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
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

    style->drawItemPixmap(painter, iconRect, Qt::AlignCenter, avatar.scaled(iconRect.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));

    KTp::Presence presence = index.data(AccountsModel::PresenceRole).value<KTp::Presence>();

    QPixmap icon = presence.icon().pixmap(KIconLoader::SizeSmallMedium);

    QRect statusIconRect = optV4.rect;
    statusIconRect.setSize(QSize(PRESENCE_ICON_SIZE, PRESENCE_ICON_SIZE));
    statusIconRect.moveTo(QPoint(optV4.rect.right() - PRESENCE_ICON_SIZE - SPACING,
                                 optV4.rect.top() + (optV4.rect.height() - PRESENCE_ICON_SIZE) / 2));

    painter->drawPixmap(statusIconRect, icon);

    QFont nameFont = KGlobalSettings::generalFont();

    const QFontMetrics nameFontMetrics(nameFont);

    painter->setFont(nameFont);

    QRect userNameRect = optV4.rect;
    userNameRect.setX(iconRect.x() + iconRect.width() + SPACING * 2);
    userNameRect.setY(userNameRect.y() + (userNameRect.height()/2 - nameFontMetrics.height()/2));
    userNameRect.setWidth(userNameRect.width() - PRESENCE_ICON_SIZE - SPACING);

    painter->drawText(userNameRect,
                      nameFontMetrics.elidedText(optV4.text, Qt::ElideRight, userNameRect.width()));

    QRect presenceMessageRect = optV4.rect;
    presenceMessageRect.setX(userNameRect.x() + nameFontMetrics.boundingRect(optV4.text).width() + SPACING * 2);
    presenceMessageRect.setWidth(optV4.rect.width() - presenceMessageRect.x() - PRESENCE_ICON_SIZE - SPACING);
    presenceMessageRect.setY(presenceMessageRect.y() + (presenceMessageRect.height()/2 - nameFontMetrics.height()/2));

    QPen presenceMessagePen = painter->pen();
    presenceMessagePen.setColor(m_palette->color(QPalette::Disabled, QPalette::Text));

    painter->setPen(presenceMessagePen);

    painter->drawText(presenceMessageRect,
                      nameFontMetrics.elidedText(presence.statusMessage().simplified(),
                                                 Qt::ElideRight, presenceMessageRect.width()));

    painter->restore();
}

QSize ContactDelegateCompact::sizeHintContact(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    return QSize(0, AVATAR_SIZE + 2 * SPACING);
}

#include "contact-delegate-compact.moc"
