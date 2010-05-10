/*
 * This file is part of telepathy-contactslist-prototype
 *
 * Copyright (C) 2009-2010 Collabora Ltd. <info@collabora.co.uk>
 *   @Author Dario Freddi <dario.freddi@collabora.co.uk>
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

#include "remove-contacts-job.h"

#include <person.h>
#include <personcontact.h>
#include <imaccount.h>
#include <pimo.h>
#include <informationelement.h>

#include <QTimer>

#include "telepathy-account-proxy.h"
#include "telepathy-bridge_p.h"
#include "telepathy-base-job_p.h"

#include <KLocalizedString>
#include <KDebug>

#include <KMessageBox>

#include <TelepathyQt4/Account>
#include <TelepathyQt4/AccountManager>
#include <TelepathyQt4/ContactManager>
#include <TelepathyQt4/PendingOperation>

#include <Nepomuk/Query/Query>
#include <Nepomuk/Query/QueryServiceClient>
#include <Nepomuk/Query/AndTerm>
#include <Nepomuk/Query/ResourceTerm>
#include <Nepomuk/Query/ResourceTypeTerm>
#include <Nepomuk/Query/ComparisonTerm>
#include <Nepomuk/Query/Result>

class RemoveContactsJobPrivate : public TelepathyBaseJobPrivate
{
    Q_DECLARE_PUBLIC(RemoveContactsJob)
    RemoveContactsJob * const q_ptr;

public:
    RemoveContactsJobPrivate(RemoveContactsJob *parent,
                             RemoveContactsJob::ProcessingMode m, TelepathyBridge::RemovalModes rm)
        : TelepathyBaseJobPrivate(parent), q_ptr(parent), mode(m), removalModes(rm)
    {}
    virtual ~RemoveContactsJobPrivate() {}

    RemoveContactsJob::ProcessingMode mode;
    TelepathyBridge::RemovalModes removalModes;

    Nepomuk::PersonContact contact;
    Nepomuk::Person metacontact;
    QList< Nepomuk::PersonContact > contacts;
    QList< Nepomuk::Person > metacontacts;

    // Operation Q_PRIVATE_SLOTS
    void __k__removeContact();
    void __k__removeContacts();
    void __k__removeMetaContact();
    void __k__removeMetaContacts();
};

RemoveContactsJob::RemoveContactsJob(const Nepomuk::PersonContact& contact, TelepathyBridge::RemovalModes modes, QObject *parent)
    : TelepathyBaseJob(*new RemoveContactsJobPrivate(this, RemoveContactMode, modes), parent)
{
    Q_D(RemoveContactsJob);

    d->contact = contact;
}

RemoveContactsJob::RemoveContactsJob(const Nepomuk::Person& metacontact, TelepathyBridge::RemovalModes modes, QObject *parent)
    : TelepathyBaseJob(*new RemoveContactsJobPrivate(this, RemoveMetaContactMode, modes), parent)
{
    Q_D(RemoveContactsJob);

    d->metacontact = metacontact;
}

RemoveContactsJob::RemoveContactsJob(const QList< Nepomuk::PersonContact >& contacts,
                                     TelepathyBridge::RemovalModes modes, QObject *parent)
    : TelepathyBaseJob(*new RemoveContactsJobPrivate(this, RemoveContactsMode, modes), parent)
{
    Q_D(RemoveContactsJob);

    d->contacts = contacts;
}

RemoveContactsJob::RemoveContactsJob(const QList< Nepomuk::Person >& metacontacts,
                                     TelepathyBridge::RemovalModes modes, QObject *parent)
    : TelepathyBaseJob(*new RemoveContactsJobPrivate(this, RemoveMetaContactsMode, modes), parent)
{
    Q_D(RemoveContactsJob);

    d->metacontacts = metacontacts;
}

RemoveContactsJob::~RemoveContactsJob()
{
}

void RemoveContactsJob::start()
{
    Q_D(RemoveContactsJob);
    // What are we supposed to do?
    switch (d->mode) {
        case RemoveContactMode:
            QTimer::singleShot(0, this, SLOT(__k__removeContact()));
            break;
        case RemoveContactsMode:
            QTimer::singleShot(0, this, SLOT(__k__removeContacts()));
            break;
        case RemoveMetaContactMode:
            QTimer::singleShot(0, this, SLOT(__k__removeMetaContact()));
            break;
        case RemoveMetaContactsMode:
            QTimer::singleShot(0, this, SLOT(__k__removeMetaContacts()));
            break;
        default:
            // Hmm?
            setError(TelepathyBridge::InvalidOperationError);
            setErrorText(i18n("This is an internal error of KDE-Telepathy"));
            QTimer::singleShot(0, this, SLOT(__k__doEmitResult()));
            break;
    }
}

void RemoveContactsJobPrivate::__k__removeContact()
{
    // Just to be sure...
    contacts.clear();
    contacts << contact;

    return __k__removeContacts();
}

void RemoveContactsJobPrivate::__k__removeContacts()
{
    Q_Q(RemoveContactsJob);

    QHash< TelepathyAccountProxy*, QList< Tp::ContactPtr > > tbd;
    QHash< TelepathyAccountProxy*, QList< Nepomuk::PersonContact > > proxyToContacts;

    foreach (const Nepomuk::PersonContact &contact, contacts) {
        // Retrieve IM identifiers first
        QStringList imIDs;
        foreach (const Nepomuk::IMAccount &account, contact.iMAccounts()) {
            imIDs << account.imIDs();
        }

        foreach (TelepathyAccountProxy *proxy, TelepathyBridge::instance()->d_func()->accountProxiesForContact(contact)) {
            tbd[proxy] << proxy->contactsForIdentifiers(imIDs);
            proxyToContacts[proxy] << contact;
        }
    }

    if (tbd.isEmpty()) {
        kWarning() << "No contacts retrieved!";
        // Give it up and emit an error
        q->setError(TelepathyBridge::NoContactsFoundError);
        q->setErrorText(i18np("No existing Telepathy contacts could be mapped to the chosen contact",
                              "No existing Telepathy contacts could be mapped to the chosen contacts",
                              contacts.size()));
        QTimer::singleShot(0, q, SLOT(__k__doEmitResult()));
        return;
    }

    // Ok, now perform the removals and watch the pending operations
    operations.clear();

    QHash< TelepathyAccountProxy*, QList< Tp::ContactPtr > >::const_iterator i;
    for (i = tbd.constBegin(); i != tbd.constEnd(); ++i) {
        // Ok, now let's see what we got here.
        if (removalModes & TelepathyBridge::RemovePublicationMode) {
            // Can we do it?
            if (i.key()->account()->connection()->contactManager()->canRemovePresencePublication()) {
                // Cool, let's go
                addOperation(i.key()->account()->connection()->contactManager()->removePresencePublication(i.value()));
            }
        }
        if (removalModes & TelepathyBridge::RemoveSubscriptionMode) {
            // Can we do it?
            if (i.key()->account()->connection()->contactManager()->canRemovePresenceSubscription()) {
                // Cool, let's go
                addOperation(i.key()->account()->connection()->contactManager()->removePresenceSubscription(i.value()));
            }
        }
        if (removalModes & TelepathyBridge::BlockMode) {
            // Can we do it?
            if (i.key()->account()->connection()->contactManager()->canBlockContacts()) {
                // Cool, let's go
                addOperation(i.key()->account()->connection()->contactManager()->blockContacts(i.value()));
            }
        }
        if (removalModes & TelepathyBridge::RemoveFromMetacontactMode) {
            // FIXME: Port to new OSCAF standard for accessing "me" as soon as it
            // becomes available.
            Nepomuk::Thing me(QUrl::fromEncoded("nepomuk:/myself"));

            foreach (const Nepomuk::PersonContact &contact, proxyToContacts[i.key()]) {
                QList< Nepomuk::Query::Result > results;
                // Get metacontacts for a specific contact
                {
                    using namespace Nepomuk::Query;

                    ResourceTypeTerm rterm(Nepomuk::Vocabulary::PIMO::Person());
                    ComparisonTerm cterm(Nepomuk::Vocabulary::PIMO::groundingOccurrence(),
                                         ResourceTerm(contact));
                    Query query(AndTerm(rterm, cterm));

                    bool queryResult = true;
                    results = QueryServiceClient::syncQuery(query, &queryResult);

                    if (!queryResult) {
                        KMessageBox::error(0, i18n("It was not possible to query Nepomuk database. Please check your "
                                                   "installation and make sure Nepomuk is running."));
                    }
                }

                foreach (const Nepomuk::Query::Result &result, results) {
                    Nepomuk::Person foundPerson(result.resource());

                    if (foundPerson.resourceUri() == me.resourceUri()) {
                        kDebug() << "Skipping myself resource for removing from metacontact";
                        continue;
                    }

                    // Ok, let's now remove the grounding occurrence
                    QList< Nepomuk::InformationElement > gOccs = foundPerson.groundingOccurrences();
                    {
                        QList< Nepomuk::InformationElement >::iterator it = gOccs.begin();
                        while (it != gOccs.end()) {
                            if ((*it).resourceUri() == contact.resourceUri()) {
                                kDebug() << "Matched groundingOccurrence metacontact->contact, removing";
                                it = gOccs.erase(it);
                            } else {
                                ++it;
                            }
                        }
                    }

                    // Set the new property
                    foundPerson.setGroundingOccurrences(gOccs);
                }
            }
        }
    }
}

void RemoveContactsJobPrivate::__k__removeMetaContact()
{
    // Just to be sure...
    metacontacts.clear();
    metacontacts << metacontact;

    __k__removeMetaContacts();
}

void RemoveContactsJobPrivate::__k__removeMetaContacts()
{
    // Just to be sure...
    contacts.clear();

    foreach (const Nepomuk::Person &person, metacontacts) {
        contacts << TelepathyBridge::instance()->d_func()->contactsForMetaContact(person);
    }

    // Have a check: is contacts empty?
    if (contacts.isEmpty()) {
        // Ouch, failure
        Q_Q(RemoveContactsJob);
        q->setError(TelepathyBridge::NoContactsFoundError);
        q->setErrorText(i18np("No existing contacts could be mapped to the chosen metacontact",
                              "No existing contacts could be mapped to any of the chosen metacontacts",
                              metacontacts.size()));
        QTimer::singleShot(0, q, SLOT(__k__doEmitResult()));
        return;
    }

    // Be more async
    Q_Q(RemoveContactsJob);
    QTimer::singleShot(0, q, SLOT(__k__removeContacts()));
}

#include "remove-contacts-job.moc"
