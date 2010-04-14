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

#include "contact-item.h"

#include "contactgroup.h"

#include <KDebug>
#include <dataobject.h>
#include <QPainter>
#include <kiconloader.h>

ContactItem::ContactItem(Nepomuk::PersonContact personContact,
                         Nepomuk::IMAccount imAccount,
                         QObject *parent)
  : QObject(parent),
    AbstractTreeItem(),
    m_personContact(personContact),
    m_imAccount(imAccount),
    m_presenceIcon(new KIcon)
{
    // Subscribe to Nepomuk change signals for our Nepomuk Resources.
    NepomukSignalWatcher *watcher = NepomukSignalWatcher::instance();
    watcher->registerCallbackOnSubject(m_imAccount, this);
    watcher->registerCallbackOnSubject(m_personContact, this);

    updatePresenceIcon();
}

ContactItem::~ContactItem()
{
    kDebug();
}

Nepomuk::PersonContact ContactItem::personContact() const
{
    return m_personContact;
}

QString ContactItem::displayName() const
{
    // Use the IM Account Nick Name for now.
    return m_imAccount.imNicknames().first();
}

QString ContactItem::accountIdentifier() const
{
    // Return the IM Account Identifier.
    return m_imAccount.imIDs().first();
}

QUrl ContactItem::isBuddyOf() const
{
    return m_imAccount.isBuddyOfs().first().resourceUri();
}

void ContactItem::updatePresenceIcon()
{
    // First, delete the old Icon.
    delete m_presenceIcon;

    // Now find out the current status.
    QList<qint64> statusTypes = m_imAccount.statusTypes();

    // If no presenceType set, then null KIcon.
    if (statusTypes.size() == 0) {
        m_presenceIcon = new KIcon();
        return;
    }

    // Get the presence type and set the icon appropriately from it.
    QString iconName;

    switch (statusTypes.first()) {
    case 2:
        iconName = "user-online";
        break;
    case 3:
        iconName = "user-away";
        break;
    case 4:
        iconName = "user-away-extended";
        break;
    case 5:
        iconName = "user-invisible";
        break;
    case 6:
        iconName = "user-busy";
        break;
    default:
        iconName = "user-offline";
        break;
    }

    m_presenceIcon = new KIcon(iconName);

    kDebug() << "Attempt to build the avatar" << m_personContact.resourceUri();
    // Ok, now build the avatar
    m_pixmap = QPixmap();
    if (!m_personContact.avatarTokens().isEmpty()) {
        // Load the image then
        if (!m_personContact.photos().isEmpty()) {
            if (!m_personContact.photos().first().interpretedAses().isEmpty()) {
                QByteArray imgdata =
                QByteArray::fromBase64(
                                m_personContact.photos().first().interpretedAses().first().plainTextContents().first().toUtf8());
                QImage image = QImage::fromData(imgdata);
                m_pixmap = QPixmap::fromImage(image);
                m_pixmap = m_pixmap.scaled(32,32);
            }
        }
    }

    if (m_pixmap.isNull()) {
        // try to load the action icon
        m_pixmap = KIconLoader::global()->loadIcon("im-user",
                                                   KIconLoader::NoGroup,
                                                   32,
                                                   KIconLoader::DefaultState,
                                                   QStringList(),
                                                   0,
                                                   true);
    }

    // create a painter to paint the action icon over the key icon
    QPainter painter(&m_pixmap);
    // the the emblem icon to size 12
    int overlaySize = 12;
    // try to load the action icon
    const QPixmap iconPixmap = m_presenceIcon->pixmap(overlaySize);
    // if we're able to load the action icon paint it over
    if (!m_pixmap.isNull()) {
        QPoint startPoint;
        // bottom right corner
        startPoint = QPoint(32 - overlaySize - 1,
                            32 - overlaySize - 1);
        painter.drawPixmap(startPoint, iconPixmap);
    }
}

const KIcon& ContactItem::presenceIcon() const
{
    Q_ASSERT(m_presenceIcon != 0);

    return *m_presenceIcon;
}

qint64 ContactItem::presenceType() const
{
    QList<qint64> statusTypes = m_imAccount.statusTypes();

    if (statusTypes.size() == 0) {
        return 1;
    }

    return statusTypes.first();
}

QStringList ContactItem::groups() const
{
    QList<Nepomuk::ContactGroup> groups = m_personContact.belongsToGroups();

    QStringList groupNames;

    if (groups.isEmpty()) {
        // FIXME: What do we do if there is no group?
        return QStringList() << "No Group";
    }

    foreach (const Nepomuk::ContactGroup &group, groups) {
        groupNames << group.contactGroupName();
    }

    return groupNames;
}

void ContactItem::onStatementAdded(const Soprano::Statement &statement)
{
    Q_UNUSED(statement);

    updatePresenceIcon();
    Q_EMIT dirty();
}
const QPixmap& ContactItem::avatar() const
{
    return m_pixmap;
}

#include "contact-item.moc"

