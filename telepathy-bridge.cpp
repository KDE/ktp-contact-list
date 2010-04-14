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

#include "telepathy-bridge_p.h"

#include "telepathy-account-proxy.h"

// Nepomuk resources
#include <nco.h>
#include <telepathy.h>
#include <personcontact.h>
#include <person.h>
#include <imaccount.h>
#include <pimo.h>
#include <informationelement.h>

// Jobs
#include "remove-contacts-from-group-job.h"
#include "add-contacts-to-group-job.h"
#include "remove-contacts-job.h"
#include "add-contact-job.h"

#include <TelepathyQt4/AccountManager>
#include <TelepathyQt4/PendingReady>
#include <TelepathyQt4/ContactManager>

#include <KDebug>
#include <KGlobal>

#include <Nepomuk/ResourceManager>

#include <Soprano/Node>
#include <Soprano/QueryResultIterator>
#include <Soprano/Model>

class TelepathyBridgeHelper
{
public:
    TelepathyBridgeHelper() : q(0) {}
    ~TelepathyBridgeHelper() {
        delete q;
    }
    TelepathyBridge *q;
};

K_GLOBAL_STATIC(TelepathyBridgeHelper, s_globalTelepathyBridge)

TelepathyBridge *TelepathyBridge::instance()
{
    if (!s_globalTelepathyBridge->q) {
        new TelepathyBridge;
    }

    return s_globalTelepathyBridge->q;
}

TelepathyBridge::TelepathyBridge(QObject *parent)
        : QObject(parent)
        , d_ptr(new TelepathyBridgePrivate(this))
{
    Q_ASSERT(!s_globalTelepathyBridge->q);
    s_globalTelepathyBridge->q = this;
}

TelepathyBridge::~TelepathyBridge()
{
    delete d_ptr;
}

void TelepathyBridge::init()
{
    Q_D(TelepathyBridge);
    // Initialise Telepathy.
    Tp::registerTypes();

    // Create an instance of the AccountManager and start to get it ready.
    d->accountManager = Tp::AccountManager::create();

    connect(d->accountManager->becomeReady(),
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(__k__onAccountManagerReady(Tp::PendingOperation*)));

    // In the meanwhile, get our "me" PIMO person
    // FIXME: Port to new OSCAF standard for accessing "me" as soon as it
    // becomes available.
    Nepomuk::Thing me(QUrl::fromEncoded("nepomuk:/myself"));

    // Loop through all the grounding instances of this person
    foreach (Nepomuk::InformationElement resource, me.groundingOccurrences()) {
        // See if this grounding instance is of type nco:contact.
        if (resource.hasType(Nepomuk::Vocabulary::NCO::PersonContact())) {
            // FIXME: We are going to assume the first NCO::PersonContact is the
            // right one. Can we improve this?
            d->mePimoURI = resource.resourceUri();
            break;
        }
    }
}

void TelepathyBridgePrivate::__k__onAccountManagerReady(Tp::PendingOperation* op)
{
    Q_Q(TelepathyBridge);

    if (op->isError()) {
        kWarning() << "Account manager cannot become ready:"
                   << op->errorName()
                   << op->errorMessage();
        Q_EMIT q->ready(false);
        return;
    }

    // Account Manager is now ready. We should watch for any new accounts being created.
    q->connect(accountManager.data(),
            SIGNAL(accountCreated(QString)),
            SLOT(__k__onAccountCreated(QString)));

    // Take into account (ha ha) the accounts that already existed when the AM object became ready.
    foreach (const QString &path, accountManager.data()->allAccountPaths()) {
        __k__onAccountCreated(path);
    }

    // Hey, initialization finished
    Q_EMIT q->ready(true);
}

void TelepathyBridgePrivate::__k__onAccountCreated(const QString& path)
{
    Q_Q(TelepathyBridge);
    accountProxies.append(new TelepathyAccountProxy(path, accountManager, q));
}

TelepathyAccountProxy* TelepathyBridgePrivate::accountProxyForAccount(const Nepomuk::IMAccount& imAccount) const
{
    if (!imAccount.isValid()) {
        kWarning() << "Given imAccount appears not to be valid!";
        return 0;
    }

    TelepathyAccountProxy *proxy = 0;
    foreach (TelepathyAccountProxy *account, accountProxies) {
        if (account->isReady()) {
            if (account->account().data()->parameters().value("account") == imAccount.imIDs().first()) {
                proxy = account;
            }
        }
    }

    if (!proxy) {
        kWarning() << "Could not find a matching account, or the matching account is not ready yet";
    }
    return proxy;
}

QList< TelepathyAccountProxy* > TelepathyBridgePrivate::accountProxiesForContact(const Nepomuk::PersonContact& contact) const
{
    QList<TelepathyAccountProxy*> retlist;

    foreach (const Nepomuk::IMAccount &account, contact.iMAccounts()) {
        foreach (const Nepomuk::IMAccount &buddyAccount, account.isBuddyOfs()) {
            foreach (TelepathyAccountProxy *proxyAccount, accountProxies) {
                if (proxyAccount->isReady()) {
                    foreach (const QString &imID, buddyAccount.imIDs()) {
                        if (proxyAccount->account().data()->parameters().value("account") == imID) {
                            retlist << proxyAccount;
                        }
                    }
                }
            }
        }
    }

    return retlist;
}

QList< Nepomuk::PersonContact > TelepathyBridgePrivate::contactsForMetaContact(const Nepomuk::Person& metacontact) const
{
    QList< Nepomuk::PersonContact > retlist;

    // We need a query here, go sparql!
    QString query = QString("select distinct ?a where { ?a a %1 . %2 %3 ?a . ?a %4 ?r . ?r a %5 . ?r %6 ?s . "
                            "?s a %5 . ?t a %1 . ?t %4 ?s . %7 %3 ?t }")
            .arg(Soprano::Node::resourceToN3(Nepomuk::Vocabulary::NCO::PersonContact()))
            .arg(Soprano::Node::resourceToN3(metacontact.resourceUri()))
            .arg(Soprano::Node::resourceToN3(Nepomuk::Vocabulary::PIMO::groundingOccurrence()))
            .arg(Soprano::Node::resourceToN3(Nepomuk::Vocabulary::NCO::hasIMAccount()))
            .arg(Soprano::Node::resourceToN3(Nepomuk::Vocabulary::NCO::IMAccount()))
            .arg(Soprano::Node::resourceToN3(Nepomuk::Vocabulary::Telepathy::isBuddyOf()))
            .arg(Soprano::Node::resourceToN3(mePimoURI));

    // Get the Nepomuk model to query.
    Soprano::Model *model = Nepomuk::ResourceManager::instance()->mainModel();
    Soprano::QueryResultIterator it = model->executeQuery(query, Soprano::Query::QueryLanguageSparql);

    // Iterate over all the IMAccounts/PersonContacts found.
    while(it.next()) {
        Nepomuk::PersonContact foundPersonContact(it.binding("a").uri());

        retlist << foundPersonContact;
    }

    return retlist;
}

QStringList TelepathyBridge::knownGroupsFor(const Nepomuk::IMAccount& account) const
{
    Q_D(const TelepathyBridge);

    TelepathyAccountProxy *proxy = d->accountProxyForAccount(account);

    if (!proxy) {
        return QStringList();
    } else {
        return proxy->account().data()->connection().data()->contactManager()->allKnownGroups();
    }
}

QStringList TelepathyBridge::knownGroupsFor(const Nepomuk::Person& metacontact) const
{
    Q_D(const TelepathyBridge);

    QStringList retlist;
    foreach (const Nepomuk::PersonContact &contact, d->contactsForMetaContact(metacontact)) {
        retlist << knownGroupsFor(contact);
    }

    return retlist;
}

QStringList TelepathyBridge::knownGroupsFor(const Nepomuk::PersonContact& contact) const
{
    Q_D(const TelepathyBridge);

    QStringList retlist;
    foreach (TelepathyAccountProxy *proxy, d->accountProxiesForContact(contact)) {
        retlist << proxy->account().data()->connection().data()->contactManager()->allKnownGroups();
    }

    return retlist;
}

KJob *TelepathyBridge::removeContactsFromGroup(const QString& group, const QList< Nepomuk::PersonContact >& contacts)
{
    return new RemoveContactsFromGroupJob(group, contacts, this);
}

KJob *TelepathyBridge::removeContactFromGroup(const QString& group, const Nepomuk::PersonContact& contact)
{
    return new RemoveContactsFromGroupJob(group, contact, this);
}

KJob *TelepathyBridge::removeMetaContactsFromGroup(const QString& group, const QList< Nepomuk::Person >& contacts)
{
    return new RemoveContactsFromGroupJob(group, contacts, this);
}

KJob *TelepathyBridge::removeMetaContactFromGroup(const QString& group, const Nepomuk::Person& contact)
{
    return new RemoveContactsFromGroupJob(group, contact, this);
}

KJob* TelepathyBridge::addContactsToGroup(const QString& group, const QList< Nepomuk::PersonContact >& contacts)
{
    return new AddContactsToGroupJob(group, contacts, this);
}

KJob* TelepathyBridge::addContactToGroup(const QString& group, const Nepomuk::PersonContact& contact)
{
    return new AddContactsToGroupJob(group, contact, this);
}

KJob* TelepathyBridge::addMetaContactsToGroup(const QString& group, const QList< Nepomuk::Person >& contacts)
{
    return new AddContactsToGroupJob(group, contacts, this);
}

KJob* TelepathyBridge::addMetaContactToGroup(const QString& group, const Nepomuk::Person& contact)
{
    return new AddContactsToGroupJob(group, contact, this);
}

TelepathyBridge::RemovalModes TelepathyBridge::supportedRemovalModesFor(const Nepomuk::PersonContact& contact) const
{
    Q_D(const TelepathyBridge);

    QList< TelepathyAccountProxy* > proxies = d->accountProxiesForContact(contact);

    bool canBlock = true;
    bool canUnsubscribe = true;
    bool canUnpublish = true;
    foreach (TelepathyAccountProxy *proxy, proxies) {
        if (!proxy->account().data()->connection().data()->contactManager()->canBlockContacts()) {
            canBlock = false;
        }
        if (!proxy->account().data()->connection().data()->contactManager()->canRemovePresencePublication()) {
            canUnpublish = false;
        }
        if (!proxy->account().data()->connection().data()->contactManager()->canRemovePresenceSubscription()) {
            canUnsubscribe = false;
        }
    }

    RemovalModes retmodes;

    if (canBlock) {
        if (retmodes == 0) {
            retmodes = TelepathyBridge::BlockMode;
        } else {
            retmodes |= TelepathyBridge::BlockMode;
        }
    }
    if (canUnpublish) {
        if (retmodes == 0) {
            retmodes = TelepathyBridge::RemovePublicationMode;
        } else {
            retmodes |= TelepathyBridge::RemovePublicationMode;
        }
    }
    if (canUnsubscribe) {
        if (retmodes == 0) {
            retmodes = TelepathyBridge::RemoveSubscriptionMode;
        } else {
            retmodes |= TelepathyBridge::RemoveSubscriptionMode;
        }
    }

    return retmodes;
}

TelepathyBridge::RemovalModes TelepathyBridge::supportedRemovalModesFor(const Nepomuk::Person& metacontact) const
{
    Q_D(const TelepathyBridge);

    RemovalModes retmodes = RemoveMetaContactMode;
    foreach (const Nepomuk::PersonContact& contact, d->contactsForMetaContact(metacontact)) {
        RemovalModes contactModes = supportedRemovalModesFor(contact);
        if (contactModes & TelepathyBridge::BlockMode && !(retmodes & TelepathyBridge::BlockMode)) {
            retmodes |= TelepathyBridge::BlockMode;
        }
        if (contactModes & TelepathyBridge::RemovePublicationMode && !(retmodes & TelepathyBridge::RemovePublicationMode)) {
            retmodes |= TelepathyBridge::RemovePublicationMode;
        }
        if (contactModes & TelepathyBridge::RemoveSubscriptionMode && !(retmodes & TelepathyBridge::RemoveSubscriptionMode)) {
            retmodes |= TelepathyBridge::RemoveSubscriptionMode;
        }
    }

    return retmodes;
}

KJob* TelepathyBridge::removeContact(const Nepomuk::PersonContact& contact, TelepathyBridge::RemovalModes modes)
{
    return new RemoveContactsJob(contact, modes, this);
}

KJob* TelepathyBridge::removeMetaContact(const Nepomuk::Person& metacontact, TelepathyBridge::RemovalModes modes)
{
    return new RemoveContactsJob(metacontact, modes, this);
}

bool TelepathyBridge::canAddContact(const Nepomuk::IMAccount& account, const QString& contactId)
{
    Q_D(TelepathyBridge);

    TelepathyAccountProxy *proxy = d->accountProxyForAccount(account);

    if (!proxy) {
        // If the account is not valid, it surely is not able to add stuff :D
        return false;
    }

    // First of all, is our contact capable of requesting presence subscription?
    if (!proxy->account().data()->connection().data()->contactManager()->canRequestPresenceSubscription()) {
        return false;
    }

    // Ok, now are we able to retrieve a valid contact pointer?
    // TODO: This function is async and we surely do not want to block using an event loop here. How to handle this?
//     QList< Tp::ContactPtr > contacts =
//         proxy->account().data()->connection().data()->contactManager()->contactsForIdentifiers(QStringList() << contactId);
//
//     return !contacts.isEmpty();
    return true;
}

KJob* TelepathyBridge::addContact(const Nepomuk::IMAccount& account, const QString& contactId)
{
    return new AddContactJob(account, contactId, QString(), Nepomuk::Person(), this);
}

KJob* TelepathyBridge::addContact(const Nepomuk::IMAccount& account, const QString& contactId, const QString& petName)
{
    return new AddContactJob(account, contactId, petName, Nepomuk::Person(), this);
}

KJob* TelepathyBridge::addContact(const Nepomuk::IMAccount& account, const QString& contactId, const QString& petName,
                                  const Nepomuk::Person& metacontact)
{
    return new AddContactJob(account, contactId, petName, metacontact, this);
}


#include "telepathy-bridge.moc"
