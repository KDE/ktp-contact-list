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

ContactItem::ContactItem(Nepomuk::PersonContact personContact,
                         Nepomuk::IMAccount imAccount,
                         QObject *parent)
  : QObject(parent),
    m_personContact(personContact),
    m_imAccount(imAccount),
    m_presenceIcon(new KIcon)
{
    kDebug() << this << ": New ContactItem: " << personContact.resourceType().toString() << imAccount.resourceType().toString();

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

QString ContactItem::displayName() const
{
    // Use the IM Account Nick Name for now.
    return m_imAccount.imNicknames().first();
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

    foreach (const Nepomuk::ContactGroup &group, groups) {
        groupNames << group.contactGroupName();
    }

    return groupNames;
}

void ContactItem::onStatementAdded(const Soprano::Statement &statement)
{
    kDebug() << "Statement added called.";

    Q_EMIT dirty();
}


#include "contact-item.moc"

