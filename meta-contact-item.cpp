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

#include <Nepomuk/Query/QueryServiceClient>
#include <Nepomuk/Query/Query>
#include <Nepomuk/Query/ResourceTypeTerm>
#include <Nepomuk/Query/ResourceTerm>
#include <Nepomuk/Query/ComparisonTerm>
#include <Nepomuk/Query/AndTerm>
#include <Nepomuk/Query/NegationTerm>
#include <Nepomuk/Query/Result>

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

    m_queryClient = new Nepomuk::Query::QueryServiceClient(this);
    connect(m_queryClient, SIGNAL(newEntries(QList<Nepomuk::Query::Result>)),
            this, SLOT(onNewEntries(QList<Nepomuk::Query::Result>)));
    connect(m_queryClient, SIGNAL(entriesRemoved(QList<QUrl>)),
            this, SLOT(onEntriesRemoved(QList<QUrl>)));

    // Get all Telepathy PersonContacts which belong to this metacontact, and match our criteria
    {
        using namespace Nepomuk::Query;
        using namespace Nepomuk::Vocabulary;
        // subquery to match grouding occurrences of me
        ComparisonTerm goterm(PIMO::groundingOccurrence(),
                              ResourceTerm(me));
        goterm.setInverted(true);

        // combine that with only nco:PersonContacts
        AndTerm pcgoterm(ResourceTypeTerm(NCO::PersonContact()),
                         goterm);

        // now look for im accounts of those grounding occurrences (pcgoterm will become the subject of this comparison,
        // thus the comparison will match the im accounts)
        ComparisonTerm impcgoterm(NCO::hasIMAccount(),
                                  pcgoterm);
        impcgoterm.setInverted(true);

        // now look for all buddies of the accounts
        ComparisonTerm buddyTerm(Telepathy::isBuddyOf(),
                                 impcgoterm);
        // set the name of the variable (i.e. the buddies) to be able to match it later
        buddyTerm.setVariableName("t");

        // same comparison, other property, but use the same variable name to match them
        ComparisonTerm ppterm(Telepathy::publishesPresenceTo(),
                              ResourceTypeTerm(NCO::IMAccount()));
        ppterm.setVariableName("t");

        // combine both to complete the matching of the im account ?account
        AndTerm accountTerm(ResourceTypeTerm(NCO::IMAccount()),
                            buddyTerm, ppterm);

        // match the account and select it for the results
        ComparisonTerm imaccountTerm(NCO::hasIMAccount(), accountTerm);
        imaccountTerm.setVariableName("account");

        // the result must be a groundingOccurrence of pimoPerson
        ComparisonTerm personTerm(PIMO::groundingOccurrence(),
                                  ResourceTerm(pimoPerson));
        personTerm.setInverted(true);

        // and all combined
        Query query(AndTerm(ResourceTypeTerm(Nepomuk::Vocabulary::NCO::PersonContact()),
                            imaccountTerm, personTerm));

        bool queryResult = m_queryClient->query(query);
        kDebug() << "Query result for " << pimoPerson << queryResult;
        if (!queryResult) {
            KMessageBox::error(0, i18n("It was not possible to query Nepomuk database. Please check your "
                                    "installation and make sure Nepomuk is running."));
        }
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
        Nepomuk::PersonContact foundPersonContact(result.resource());
        Nepomuk::IMAccount foundIMAccount(result.additionalBinding("account").uri());
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

