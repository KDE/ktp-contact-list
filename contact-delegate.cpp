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
#include <QtGui/QToolTip>

#include <KIconLoader>
#include <KIcon>
#include <KDebug>
#include <KGlobalSettings>
#include <KDE/KLocale>

#include "accounts-model.h"
#include "contact-model-item.h"
#include <QHelpEvent>

const int SPACING = 4;
const int AVATAR_SIZE = 32;
const int PRESENCE_ICON_SIZE = 22;
const int ACCOUNT_ICON_SIZE = 13;

ContactDelegate::ContactDelegate(QObject * parent)
    : QStyledItemDelegate(parent), ContactDelegateOverlayContainer(), m_palette(0)
{
    m_palette = new QPalette(QApplication::palette());
}

ContactDelegate::~ContactDelegate()
{
    delete m_palette;
}

void ContactDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    QStyleOptionViewItemV4 optV4 = option;
    initStyleOption(&optV4, index);

    painter->save();

    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::HighQualityAntialiasing);
    painter->setClipRect(optV4.rect);

    QStyle *style = QApplication::style();
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter);

    bool isContact = !index.data(AccountsModel::AliasRole).toString().isEmpty();

    if (isContact) {
        QRect iconRect = optV4.rect;
        iconRect.setSize(QSize(AVATAR_SIZE, AVATAR_SIZE));
        iconRect.moveTo(QPoint(iconRect.x() + SPACING, iconRect.y() + SPACING));

        QPixmap avatar = QPixmap::fromImage(QImage(index.data(AccountsModel::AvatarRole).toString()));

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

    } else {
        QRect groupRect = optV4.rect;

        QRect accountGroupRect = groupRect;
        accountGroupRect.setSize(QSize(ACCOUNT_ICON_SIZE, ACCOUNT_ICON_SIZE));
        accountGroupRect.moveTo(QPoint(groupRect.left() + 2, groupRect.top() + 2));

        QRect groupLabelRect = groupRect;
        groupLabelRect.setRight(groupLabelRect.right() - SPACING);

        QRect expandSignRect = groupLabelRect;
        expandSignRect.setLeft(ACCOUNT_ICON_SIZE + SPACING + SPACING);
        expandSignRect.setRight(groupLabelRect.left() + 20); //keep it by the left side

        QFont groupFont = KGlobalSettings::smallestReadableFont();

        QString counts;// = QString(" (%1/%2)").arg(index.data(AccountsModel::).toString(),
                        //               index.data(ModelRoles::AccountAllContactsCountRole).toString());

        painter->fillRect(groupRect, m_palette->color(QPalette::AlternateBase));

        painter->drawPixmap(accountGroupRect, KIcon(index.data(AccountsModel::IconRole).toString())
                                                   .pixmap(ACCOUNT_ICON_SIZE, ACCOUNT_ICON_SIZE));

        painter->setPen(m_palette->color(QPalette::WindowText));
        painter->setFont(groupFont);
        painter->drawText(groupLabelRect, Qt::AlignVCenter | Qt::AlignRight,
                          index.data(AccountsModel::DisplayNameRole).toString().append(counts));

        QPen thinLinePen;
        thinLinePen.setWidth(0);
        thinLinePen.setCosmetic(true);
        thinLinePen.setColor(m_palette->color(QPalette::ButtonText));

        painter->setPen(thinLinePen);

        painter->drawLine(groupRect.x(), groupRect.y(), groupRect.width(), groupRect.y());
        painter->drawLine(groupRect.x(), groupRect.bottom(), groupRect.width(), groupRect.bottom());

        QStyleOption expandSignOption = option;
        expandSignOption.rect = expandSignRect;

        if (option.state & QStyle::State_Open) {
            style->drawPrimitive(QStyle::PE_IndicatorArrowDown, &expandSignOption, painter);
        } else {
            style->drawPrimitive(QStyle::PE_IndicatorArrowRight, &expandSignOption, painter);
        }
    }

    painter->restore();
}

QSize ContactDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
//     if(option.state & QStyle::State_Selected)
//         kDebug() << index.data(ModelRoles::UserNameRole).toString();

    if (!index.data(AccountsModel::AliasRole).toString().isEmpty()) {
        return QSize(0, 32 + 4 * SPACING);
    } else return QSize(0, 20);
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

bool ContactDelegate::helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    Q_UNUSED(option);

    // Check and make sure that we only want it to work on contacts and nothing else.
    if (index.data(AccountsModel::ItemRole).userType() != qMetaTypeId<ContactModelItem*>()) {
        return false;
    }

    if (event->type() != QEvent::ToolTip) {
        return false;
    }

    const QString contactAvatar = index.data(AccountsModel::AvatarRole).toString();
    const QString displayName = index.parent().data(AccountsModel::DisplayNameRole).toString();
    const QString cmIconPath = KIconLoader::global()->iconPath(index.parent().data(AccountsModel::IconRole).toString(), 1);
    const QString alias = index.data(AccountsModel::AliasRole).toString();
    const QString presenceStatus = index.data(AccountsModel::PresenceMessageRole).toString();
    QString presenseIconPath;
    QString presenseText;

    switch (index.data(AccountsModel::PresenceTypeRole).toUInt()) {
    case Tp::ConnectionPresenceTypeAvailable:
        presenseIconPath = KIconLoader::global()->iconPath("user-online", 1);
        presenseText = i18nc("This is an IM user status", "Online");
        break;
    case Tp::ConnectionPresenceTypeAway:
        presenseIconPath = KIconLoader::global()->iconPath("user-away", 1);
        presenseText = i18nc("This is an IM user status", "Away");
        break;
    case Tp::ConnectionPresenceTypeExtendedAway:
        presenseIconPath = KIconLoader::global()->iconPath("user-away-extended", 1);
        presenseText = i18nc("This is an IM user status", "Away");
        break;
    case Tp::ConnectionPresenceTypeBusy:
        presenseIconPath = KIconLoader::global()->iconPath("user-busy", 1);
        presenseText = i18nc("This is an IM user status", "Busy");
        break;
    case Tp::ConnectionPresenceTypeOffline:
        presenseIconPath = KIconLoader::global()->iconPath("user-offline", 1);
        presenseText = i18nc("This is an IM user status", "Offline");
        break;
    default:
        presenseIconPath = KIconLoader::global()->iconPath("task-attention", 1);
        // What presense Text should be here??
        break;
    }

    /* The tooltip is composed of a HTML table to display the items in it of the contact.
     * -------------------------
     * | account it belongs to |
     * -------------------------
     * | Avatar | Con's Alias  |
     * -------------------------
     * |        | Con's Status*|
     * -------------------------
     * |  Contact is blocked*  |
     * -------------------------
     * * Display actual status name if contact has no custom status message.
     * * Contact is blocked will only show if the contact is blocked, else no display.
     */

    kDebug() << contactAvatar;

    QString table;
    table += QString("<table><th colspan='2' align='center'><img src='%2' height='16' width='16' /> %3</th>").arg(cmIconPath, displayName);
    if (contactAvatar.isEmpty() || QPixmap(contactAvatar).isNull()) {
        table += QString("<tr><td><img src='%1' width='96' /></td>").arg(KIconLoader::global()->iconPath("im-user", -1));
    } else {
        table += QString("<tr><td><img src='%1' width='96' /></td>").arg(contactAvatar);
    }

    table += "<td><table><tr>";
    table += QString("<td align='right'><b>%1</b></td>").arg(i18n("Alias:"));
    table += QString("<td>%1</td></tr>").arg(alias);
    table += QString("<tr><td align='right'><b>%1</b></td>").arg(i18n("Status:"));
    if (presenceStatus.isEmpty()) {
        table += QString("<td><img src='%1' height='16' width='16' /> %2</td></tr>").arg(presenseIconPath, presenseText);
    } else {
        table += QString("<td><img src='%1' height='16' width='16' /> %2</td></tr>").arg(presenseIconPath, presenceStatus);
    }
    if (index.data(AccountsModel::BlockedRole).toBool()) {
        table += QString("<td colspan='2'>%1</td></tr>").arg(i18n("User is blocked"));
    }
    table += "</table></td><tr></table>";
    QToolTip::showText(QCursor::pos(), table, view);

    return true;
}

#include "contact-delegate.moc"
