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
#include <nco.h>
#include <nao.h>
#include <telepathy.h>
#include <pimo.h>
#include <Nepomuk/Variant>
#include <Nepomuk/Query/Result>
#include <Nepomuk/Query/QueryServiceClient>
#include <KMessageBox>

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
    NepomukSignalWatcher *watcher = NepomukSignalWatcher::instance();
    watcher->unregisterCallbackOnSubject(m_pimoPerson, this);
}

void MetaContactItem::onStatementAdded(const Soprano::Statement &statement)
{
    Q_UNUSED(statement);

    Q_EMIT dirty();
}

Nepomuk::Person MetaContactItem::pimoPerson() const
{
    return m_pimoPerson;
}

void MetaContactItem::setPimoPerson(const Nepomuk::Person& pimoPerson)
{
    m_pimoPerson = pimoPerson;

    // Subscribe to Nepomuk change signals for our Nepomuk Resources.
    NepomukSignalWatcher *watcher = NepomukSignalWatcher::instance();
    watcher->registerCallbackOnSubject(m_pimoPerson, this);

    // FIXME: Get the Nepomuk Resource for myself in the standardised way, once it is standardised.
    Nepomuk::Resource me(QUrl::fromEncoded("nepomuk:/myself"));

    // Ok, now we need a decent query to find out our children.
    // Get all the Telepathy PersonContacts which belong to this metacontact, and match our criteria
    QString query = QString("select distinct ?r ?account where { %8 %7 ?r . ?r a %1 . ?r %2 ?account . "
                            "?acccount a %3 . ?account %4 ?t . ?account %5 ?t . ?t a %3 . ?s %2 ?t . "
                            "?s a %1 . %6 %7 ?s }")
            .arg(Soprano::Node::resourceToN3(Nepomuk::Vocabulary::NCO::PersonContact()))
            .arg(Soprano::Node::resourceToN3(Nepomuk::Vocabulary::NCO::hasIMAccount()))
            .arg(Soprano::Node::resourceToN3(Nepomuk::Vocabulary::NCO::IMAccount()))
            .arg(Soprano::Node::resourceToN3(Nepomuk::Vocabulary::Telepathy::isBuddyOf()))
            .arg(Soprano::Node::resourceToN3(Nepomuk::Vocabulary::Telepathy::publishesPresenceTo()))
            .arg(Soprano::Node::resourceToN3(me.resourceUri()))
            .arg(Soprano::Node::resourceToN3(Nepomuk::Vocabulary::PIMO::groundingOccurrence()))
            .arg(Soprano::Node::resourceToN3(pimoPerson.resourceUri()));

    m_queryClient = new Nepomuk::Query::QueryServiceClient(this);
    connect(m_queryClient, SIGNAL(newEntries(QList<Nepomuk::Query::Result>)),
            this, SLOT(onNewEntries(QList<Nepomuk::Query::Result>)));
    connect(m_queryClient, SIGNAL(entriesRemoved(QList<QUrl>)),
            this, SLOT(onEntriesRemoved(QList<QUrl>)));

    Nepomuk::Query::RequestPropertyMap rpm;
    rpm.insert("account", Nepomuk::Vocabulary::NCO::IMAccount());
    bool queryResult = m_queryClient->sparqlQuery(query, rpm);
    kDebug() << "Query result for " << pimoPerson << queryResult;
    if (!queryResult) {
        KMessageBox::error(0, i18n("It was not possible to query Nepomuk database. Please check your "
                                   "installation and make sure Nepomuk is running."));
    }
}

void MetaContactItem::onEntriesRemoved(const QList< QUrl > &entries)
{
    kDebug();
    bool isDirty = false;
    // Find and remove
    foreach (const QUrl &url, entries) {
        kDebug() << "Attempting to remove resource " << url;
        // Iterate our children
        foreach (AbstractTreeItem *item, childItems()) {
            ContactItem *contactItem = dynamic_cast<ContactItem*>(item);

            if (contactItem) {
                removeChildItem(contactItem);
                delete contactItem;
                isDirty = true;
            }
        }
    }

    emit dirty();
}

void MetaContactItem::onNewEntries(const QList< Nepomuk::Query::Result > &entries)
{
    kDebug();
    // Iterate over all the IMAccounts/PersonContacts found.
    foreach (const Nepomuk::Query::Result &result, entries) {
        Nepomuk::PersonContact foundPersonContact(result.resource().resourceUri());
        Nepomuk::IMAccount foundIMAccount(result.requestProperty(Nepomuk::Vocabulary::NCO::IMAccount()).uri());
        kDebug() << "New resource added";

        // Create the contact item itself, parenting it to this metacontact.
        ContactItem *item = new ContactItem(foundPersonContact, foundIMAccount, 0);
        item->setParentItem(this);
        appendChildItem(item);
        connect(item, SIGNAL(dirty()), this, SIGNAL(dirty()));
    }

    emit dirty();
}

QString MetaContactItem::displayName() const
{
    switch (m_type) {
        case RealMetaContact:
            // Let's display what PIMOPerson says
            return m_pimoPerson.genericLabel();
            break;
        case FakeMetaContact:
            // In this case, let's just display the name of the first contact
            if (childItems().isEmpty()) {
                return QString();
            }

            ContactItem *item = dynamic_cast<ContactItem*>(childItems().first());

            if (!item) {
                return QString();
            }

            return item->displayName();
            break;
    }
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

const QPixmap& MetaContactItem::avatar() const
{
    // FIXME: What should we actually return here?
    if (childItems().isEmpty()) {
        return m_invalidPixmap;
    }

    ContactItem *item = dynamic_cast<ContactItem*>(childItems().first());

    if (!item) {
        return m_invalidPixmap;
    }

    return item->avatar();
}

MetaContactItem::MetaContactType MetaContactItem::type() const
{
    return m_type;
}

#include "meta-contact-item.moc"

