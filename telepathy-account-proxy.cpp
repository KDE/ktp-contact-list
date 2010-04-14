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

#include "telepathy-account-proxy.h"

#include <TelepathyQt4/AccountManager>
#include <TelepathyQt4/ContactManager>
#include <TelepathyQt4/PendingOperation>
#include <TelepathyQt4/PendingReady>

#include <KDebug>

class TelepathyAccountProxyPrivate
{
    Q_DECLARE_PUBLIC(TelepathyAccountProxy)
    TelepathyAccountProxy * const q_ptr;

public:
    TelepathyAccountProxyPrivate(const QString &p, const Tp::AccountManagerPtr &aM, TelepathyAccountProxy *parent)
        : q_ptr(parent)
        , path(p)
        , accountManager(aM)
        , ready(false)
    {
        // We need to get the Tp::Account ready before we do any other stuff.
        account = accountManager->accountForPath(path);
    }
    ~TelepathyAccountProxyPrivate() {}

    QString path;
    Tp::AccountManagerPtr accountManager;
    Tp::AccountPtr account;
    Tp::ConnectionPtr connection;
    bool ready;

    void setReady(bool ready);

    void __k__onAccountReady(Tp::PendingOperation*);
    void __k__onHaveConnectionChanged(bool);
    void __k__onConnectionReady(Tp::PendingOperation*);
};

void TelepathyAccountProxyPrivate::__k__onAccountReady(Tp::PendingOperation* op)
{
    if (op->isError()) {
        kWarning() << "Account"
                   << path
                   << "cannot become ready:"
                   << op->errorName()
                   << "-"
                   << op->errorMessage();
        setReady(false);
        return;
    }

    Q_Q(TelepathyAccountProxy);

    // Connect to signals that indicate the account is online.
    q->connect(account.data(),
               SIGNAL(haveConnectionChanged(bool)),
               SLOT(__k__onHaveConnectionChanged(bool)));

    if (account.data()->haveConnection() && connection.isNull()) {
        __k__onHaveConnectionChanged(true);
    }
}

void TelepathyAccountProxyPrivate::__k__onHaveConnectionChanged(bool haveConnection)
{
    if (haveConnection) {
        // We now have a connection to the account. Get the connection ready to use.
        if (!connection.isNull()) {
            kWarning() << "Connection should be null, but is not :/ Do nowt.";
            setReady(false);
            return;
        }

        connection = account->connection();

        Tp::Features features;
        features << Tp::Connection::FeatureCore
                 << Tp::Connection::FeatureSimplePresence
                 << Tp::Connection::FeatureSelfContact
                 << Tp::Connection::FeatureRoster
                 << Tp::Connection::FeatureRosterGroups;

        Q_Q(TelepathyAccountProxy);

        q->connect(connection->becomeReady(features),
                   SIGNAL(finished(Tp::PendingOperation*)),
                   SLOT(__k__onConnectionReady(Tp::PendingOperation*)));
    } else {
        // Connection has gone down. Delete our pointer to it.
        kDebug() << "connection down";
        setReady(false);
        connection.reset();
    }
}

void TelepathyAccountProxyPrivate::__k__onConnectionReady(Tp::PendingOperation* op)
{
    if (op->isError()) {
        kWarning() << "Getting connection ready failed."
                   << op->errorName()
                   << op->errorMessage();
        connection.reset();
        setReady(false);
        return;
    }

    if (!connection->contactManager()) {
        kWarning() << "ContactManager is Null. Abort getting contacts.";
        setReady(false);
        return;
    }

    // Now we're ready
    setReady(true);
}

void TelepathyAccountProxyPrivate::setReady(bool r)
{
    if (ready != r) {
        ready = r;

        Q_Q(TelepathyAccountProxy);
        Q_EMIT q->readyChanged(ready);
    }
}

TelepathyAccountProxy::TelepathyAccountProxy(const QString& path, const Tp::AccountManagerPtr& accountManager, QObject* parent)
    : QObject(parent)
    , d_ptr(new TelepathyAccountProxyPrivate(path, accountManager, this))
{
    Tp::Features features;
    features << Tp::Account::FeatureCore
             << Tp::Account::FeatureProtocolInfo;

    Q_D(TelepathyAccountProxy);

    connect(d->account->becomeReady(features),
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(__k__onAccountReady(Tp::PendingOperation*)));
}

TelepathyAccountProxy::~TelepathyAccountProxy()
{
    delete d_ptr;
}

bool TelepathyAccountProxy::isReady() const
{
    Q_D(const TelepathyAccountProxy);
    return d->ready;
}

Tp::AccountPtr TelepathyAccountProxy::account() const
{
    Q_D(const TelepathyAccountProxy);
    return d->account;
}

QList< Tp::ContactPtr > TelepathyAccountProxy::contactsForIdentifiers(const QStringList& identifiers) const
{
    Q_D(const TelepathyAccountProxy);

    QList< Tp::ContactPtr > retlist;
    // Iterate over the contacts
    foreach (Tp::ContactPtr contact, d->connection.data()->contactManager()->allKnownContacts()) {
        if (identifiers.contains(contact.data()->id())) {
            retlist << contact;
        }
    }

    return retlist;
}

#include "telepathy-account-proxy.moc"