/*
 * This file is part of telepathy-contactslist-prototype
 *
 * Copyright (C) 2010 Collabora Ltd. <info@collabora.co.uk>
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

#include "meta-contact-item.h"

#include "contact-item.h"

#include <KDebug>
#include <KIcon>

MetaContactItem::MetaContactItem(MetaContactType type, QObject *parent)
  : QObject(parent),
    AbstractTreeItem(),
    m_type(type),
    m_invalidPresenceIcon(new KIcon())
{
    if (type == RealMetaContact) {
        // A Real metacontact. Wait for the setPimoPerson to be called before
        // setting everything up.
 
    } else if (type == FakeMetaContact) {
        // A fake Meta-Contact. There is no PIMO:Person, so wait for setPersonContact()
        // to be called before doing the setup.

    }
}

MetaContactItem::~MetaContactItem()
{
    kDebug();
}

void MetaContactItem::onStatementAdded(const Soprano::Statement &statement)
{
    Q_UNUSED(statement);

    Q_EMIT dirty();
}

void MetaContactItem::setPimoPerson(const Nepomuk::Person& pimoPerson)
{
    m_pimoPerson = pimoPerson;

    // Subscribe to Nepomuk change signals for our Nepomuk Resources.
    NepomukSignalWatcher *watcher = NepomukSignalWatcher::instance();
    watcher->registerCallbackOnSubject(m_pimoPerson, this);
}

QString MetaContactItem::displayName() const
{
    // FIXME: What should we actually return here?
    if (childItems().isEmpty()) {
        return QString();
    }

    ContactItem *item = dynamic_cast<ContactItem*>(childItems().first());

    if (!item) {
        return QString();
    }

    return item->displayName();
}

const KIcon &MetaContactItem::presenceIcon() const
{
    // FIXME: What should we actually return here?
    if (childItems().isEmpty()) {
        return *m_invalidPresenceIcon;
    }

    ContactItem *item = dynamic_cast<ContactItem*>(childItems().first());

    if (!item) {
        return *m_invalidPresenceIcon;
    }

    return item->presenceIcon();
}

QStringList MetaContactItem::groups() const
{
    // FIXME: What should we actually return here?
    if (childItems().isEmpty()) {
        return QStringList();
    }

    ContactItem *item = dynamic_cast<ContactItem*>(childItems().first());

    if (!item) {
        return QStringList();
    }

    return item->groups();
}


#include "meta-contact-item.moc"

