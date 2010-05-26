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

#include "remove-contacts-from-group-job.h"

#include <person.h>
#include <personcontact.h>
#include <imaccount.h>

#include <QTimer>

#include "telepathy-account-proxy.h"
#include "telepathy-bridge_p.h"
#include "telepathy-base-job_p.h"

#include <KLocalizedString>
#include <KDebug>

#include <TelepathyQt4/Account>
#include <TelepathyQt4/AccountManager>
#include <TelepathyQt4/ContactManager>
#include <TelepathyQt4/PendingOperation>

class RemoveContactsFromGroupJobPrivate : public TelepathyBaseJobPrivate
{
    Q_DECLARE_PUBLIC(RemoveContactsFromGroupJob)

public:
    RemoveContactsFromGroupJobPrivate(RemoveContactsFromGroupJob *parent,
                                      RemoveContactsFromGroupJob::ProcessingMode m)
        : TelepathyBaseJobPrivate(parent), mode(m)
    {}
    virtual ~RemoveContactsFromGroupJobPrivate() {}

    RemoveContactsFromGroupJob::ProcessingMode mode;

    QString group;
    Nepomuk::PersonContact contact;
    Nepomuk::Person metacontact;
    QList< Nepomuk::PersonContact > contacts;
    QList< Nepomuk::Person > metacontacts;

    // Operation Q_PRIVATE_SLOTS
    void __k__removeContactFromGroup();
    void __k__removeContactsFromGroup();
    void __k__removeMetaContactFromGroup();
    void __k__removeMetaContactsFromGroup();
};

RemoveContactsFromGroupJob::RemoveContactsFromGroupJob(const QString& group, const Nepomuk::PersonContact& contact,
                                                       QObject *parent)
    : TelepathyBaseJob(*new RemoveContactsFromGroupJobPrivate(this, RemoveContactMode), parent)
{
    Q_D(RemoveContactsFromGroupJob);

    d->group = group;
    d->contact = contact;
}

RemoveContactsFromGroupJob::RemoveContactsFromGroupJob(const QString& group, const Nepomuk::Person& metacontact,
                                                       QObject *parent)
    : TelepathyBaseJob(*new RemoveContactsFromGroupJobPrivate(this, RemoveMetaContactMode), parent)
{
    Q_D(RemoveContactsFromGroupJob);

    d->group = group;
    d->metacontact = metacontact;
}

RemoveContactsFromGroupJob::RemoveContactsFromGroupJob(const QString& group, const QList< Nepomuk::PersonContact >& contacts,
                                                       QObject *parent)
    : TelepathyBaseJob(*new RemoveContactsFromGroupJobPrivate(this, RemoveContactsMode), parent)
{
    Q_D(RemoveContactsFromGroupJob);

    d->group = group;
    d->contacts = contacts;
}

RemoveContactsFromGroupJob::RemoveContactsFromGroupJob(const QString& group, const QList< Nepomuk::Person >& metacontacts,
                                                       QObject *parent)
    : TelepathyBaseJob(*new RemoveContactsFromGroupJobPrivate(this, RemoveMetaContactsMode), parent)
{
    Q_D(RemoveContactsFromGroupJob);

    d->group = group;
    d->metacontacts = metacontacts;
}

RemoveContactsFromGroupJob::~RemoveContactsFromGroupJob()
{
}

void RemoveContactsFromGroupJob::start()
{
    Q_D(RemoveContactsFromGroupJob);
    // What are we supposed to do?
    switch (d->mode) {
        case RemoveContactMode:
            QTimer::singleShot(0, this, SLOT(__k__removeContactFromGroup()));
            break;
        case RemoveContactsMode:
            QTimer::singleShot(0, this, SLOT(__k__removeContactsFromGroup()));
            break;
        case RemoveMetaContactMode:
            QTimer::singleShot(0, this, SLOT(__k__removeMetaContactFromGroup()));
            break;
        case RemoveMetaContactsMode:
            QTimer::singleShot(0, this, SLOT(__k__removeMetaContactsFromGroup()));
            break;
        default:
            // Hmm?
            setError(TelepathyBridge::InvalidOperationError);
            setErrorText(i18n("This is an internal error of KDE-Telepathy"));
            QTimer::singleShot(0, this, SLOT(__k__doEmitResult()));
            break;
    }
}

void RemoveContactsFromGroupJobPrivate::__k__removeContactFromGroup()
{
    // Just to be sure...
    contacts.clear();
    contacts << contact;

    return __k__removeContactsFromGroup();
}

void RemoveContactsFromGroupJobPrivate::__k__removeContactsFromGroup()
{
    Q_Q(RemoveContactsFromGroupJob);

    QHash< TelepathyAccountProxy*, QList< Tp::ContactPtr > > tbd;

    foreach (const Nepomuk::PersonContact &contact, contacts) {
        // Retrieve IM identifiers first
        QStringList imIDs;
        foreach (const Nepomuk::IMAccount &account, contact.iMAccounts()) {
            imIDs << account.imIDs();
        }

        foreach (TelepathyAccountProxy *proxy, TelepathyBridge::instance()->d_func()->accountProxiesForContact(contact)) {
            tbd[proxy] << proxy->contactsForIdentifiers(imIDs);
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
        addOperation(i.key()->account()->connection()->contactManager()->removeContactsFromGroup(group, i.value()));
    }
}

void RemoveContactsFromGroupJobPrivate::__k__removeMetaContactFromGroup()
{
    // Just to be sure...
    metacontacts.clear();
    metacontacts << metacontact;

    __k__removeMetaContactsFromGroup();
}

void RemoveContactsFromGroupJobPrivate::__k__removeMetaContactsFromGroup()
{
    // Just to be sure...
    contacts.clear();

    foreach (const Nepomuk::Person &person, metacontacts) {
        contacts << TelepathyBridge::instance()->d_func()->contactsForMetaContact(person);
    }

    // Have a check: is contacts empty?
    if (contacts.isEmpty()) {
        // Ouch, failure
        Q_Q(RemoveContactsFromGroupJob);
        q->setError(TelepathyBridge::NoContactsFoundError);
        q->setErrorText(i18np("No existing contacts could be mapped to the chosen metacontact",
                              "No existing contacts could be mapped to any of the chosen metacontacts",
                              metacontacts.size()));
        QTimer::singleShot(0, q, SLOT(__k__doEmitResult()));
        return;
    }

    // Be more async
    Q_Q(RemoveContactsFromGroupJob);
    QTimer::singleShot(0, q, SLOT(__k__removeContactsFromGroup()));
}

#include "remove-contacts-from-group-job.moc"
