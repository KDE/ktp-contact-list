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

#ifndef TELEPATHY_ACCOUNT_PROXY_H
#define TELEPATHY_ACCOUNT_PROXY_H

#include <QtCore/QObject>

#include <TelepathyQt4/Types>

class TelepathyAccountProxyPrivate;
class TelepathyAccountProxy : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(TelepathyAccountProxy)
    Q_DISABLE_COPY(TelepathyAccountProxy)
public:
    TelepathyAccountProxy(const QString &path, const Tp::AccountManagerPtr &accountManager, QObject* parent = 0);
    virtual ~TelepathyAccountProxy();

    bool isReady() const;

    Tp::AccountPtr account() const;
    QList < Tp::ContactPtr > contactsForIdentifiers(const QStringList &identifiers) const;

Q_SIGNALS:
    void readyChanged(bool);

private:
    TelepathyAccountProxyPrivate * const d_ptr;

    Q_PRIVATE_SLOT(d_func(), void __k__onAccountReady(Tp::PendingOperation*))
    Q_PRIVATE_SLOT(d_func(), void __k__onHaveConnectionChanged(bool))
    Q_PRIVATE_SLOT(d_func(), void __k__onConnectionReady(Tp::PendingOperation*))
};

#endif // TELEPATHY_ACCOUNT_PROXY_H
