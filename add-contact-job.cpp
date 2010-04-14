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

#include "add-contact-job.h"

#include "telepathy-base-job_p.h"
#include "telepathy-bridge_p.h"
#include "telepathy-account-proxy.h"

#include <imaccount.h>
#include <person.h>

#include <QTimer>

#include <KLocalizedString>

#include <TelepathyQt4/Account>
#include <TelepathyQt4/ContactManager>
#include <TelepathyQt4/PendingContacts>

class AddContactJobPrivate : public TelepathyBaseJobPrivate
{
    Q_DECLARE_PUBLIC(AddContactJob)
    AddContactJob * const q_ptr;

public:
    AddContactJobPrivate(AddContactJob *parent)
        : TelepathyBaseJobPrivate(parent), q_ptr(parent)
    {}
    virtual ~AddContactJobPrivate() {}

    Nepomuk::IMAccount account;
    QString contactId;
    QString petname;
    Nepomuk::Person metacontact;

    // Operation Q_PRIVATE_SLOTS
    void __k__addContact();
    void __k__onContactsRetrieved(Tp::PendingOperation* op);
};

AddContactJob::AddContactJob(const Nepomuk::IMAccount& account, const QString& contactId,
                             const QString& petName, const Nepomuk::Person& metacontact, QObject *parent)
    : TelepathyBaseJob(*new AddContactJobPrivate(this), parent)
{
    Q_D(AddContactJob);

    d->account = account;
    d->contactId = contactId;
    d->petname = petName;
    d->metacontact = metacontact;
}

AddContactJob::~AddContactJob()
{

}

void AddContactJob::start()
{
    // Go for it
    QTimer::singleShot(0, this, SLOT(__k__addContact()));
}

void AddContactJobPrivate::__k__addContact()
{
    Q_Q(AddContactJob);
    // Let's go. First things first, the basic stuff.
    TelepathyAccountProxy *proxy = TelepathyBridge::instance()->d_func()->accountProxyForAccount(account);
    if (!proxy) {
        q->setError(TelepathyBridge::NoAccountsFoundError);
        q->setErrorText(i18n("Could not find a matching Telepathy account for the provided resource"));
        QTimer::singleShot(0, q, SLOT(__k__doEmitResult()));
        return;
    }

    if (!proxy->account()->connection()->contactManager()->canRequestPresenceSubscription()) {
        q->setError(TelepathyBridge::ProtocolNotCapableError);
        q->setErrorText(i18n("The protocol of the requested account is not capable of adding contacts to the buddy list"));
        QTimer::singleShot(0, q, SLOT(__k__doEmitResult()));
        return;
    }

    QStringList contactIds = QStringList() << contactId;
    Tp::PendingContacts *contacts = proxy->account()->connection()->contactManager()->contactsForIdentifiers(contactIds);
    q->connect (contacts, SIGNAL(finished(Tp::PendingOperation*)), q, SLOT(__k__onContactsRetrieved(Tp::PendingOperation*)));
}

void AddContactJobPrivate::__k__onContactsRetrieved(Tp::PendingOperation* op)
{
    Q_Q(AddContactJob);
    // Retrieve the PendingContacts operation first
    Tp::PendingContacts *pending = qobject_cast< Tp::PendingContacts* >(op);

    if (!pending) {
        q->setError(TelepathyBridge::TelepathyErrorError);
        q->setErrorText(i18n("Telepathy has returned an unexpected value while adding contacts"));
        QTimer::singleShot(0, q, SLOT(__k__doEmitResult()));
        return;
    }

    QList< Tp::ContactPtr > contacts = pending->contacts();

    if (contacts.isEmpty()) {
        q->setError(TelepathyBridge::NoContactsFoundError);
        q->setErrorText(i18n("Could not retrieve a valid contact for the specified identifier"));
        QTimer::singleShot(0, q, SLOT(__k__doEmitResult()));
        return;
    }

    TelepathyAccountProxy *proxy = TelepathyBridge::instance()->d_func()->accountProxyForAccount(account);
    // Fair enough. Now for the real stuff.
    // TODO: How to handle pet names here?
    addOperation(proxy->account()->connection()->contactManager()->requestPresenceSubscription(contacts));

    // TODO: How to handle metacontacts?
}

#include "add-contact-job.moc"
