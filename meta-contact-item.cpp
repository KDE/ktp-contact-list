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

MetaContactItem::MetaContactItem(MetaContactType type, QObject *parent)
  : QObject(parent),
    AbstractTreeItem(),
    m_type(type)
{
    if (type == RealMetaContact) {
        // A Real metacontact. Wait for the setPimoPerson to be called before
        // setting everything up.
        kDebug() << "Constructing new Real MetaContact.";
    } else if (type == FakeMetaContact) {
        // A fake Meta-Contact. There is no PIMO:Person, so wait for setPersonContact()
        // to be called before doing the setup.
        kDebug() << "Constructing new Fake MetaContact.";
    }
}

MetaContactItem::~MetaContactItem()
{
    kDebug();
}

void MetaContactItem::onStatementAdded(const Soprano::Statement &statement)
{
    Q_UNUSED(statement);

    kDebug() << "Statement added called.";

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
    if (childItems().isEmpty()) {
        return QString();
    }

    ContactItem *item = dynamic_cast<ContactItem*>(childItems().first());

    if (!item) {
        return QString();
    }

    return item->displayName();
}
/*
void MetaContactItem::setPersonContact(const Nepomuk::PersonContact &personContact)
{
    // This should only be called if the meta contact is fake.
    Q_ASSERT(m_type == FakeMetaContact);
    if (m_type != FakeMetaContact) {
        kWarning() << "setPersonContact called on real meta contact. This should not happen.";
        return;
    }

    // The list of personContacts should be empty when this is called.
    Q_ASSERT(!m_personContacts.isEmpty());
    if (!m_personContacts.isEmpty()) {
        kWarning() << "We already have a person contact set for this fake meta contact.";
        return;
    }

    m_personContacts.insert(personContact);
}
*/

#include "meta-contact-item.moc"

