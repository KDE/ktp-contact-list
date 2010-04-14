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

#ifndef REMOVE_CONTACTS_JOB_H
#define REMOVE_CONTACTS_JOB_H

#include <telepathy-base-job.h>

#include "telepathy-bridge.h"

namespace Nepomuk {
class PersonContact;
class Person;
}

class RemoveContactsJobPrivate;
class RemoveContactsJob : public TelepathyBaseJob
{
    Q_OBJECT
    Q_DISABLE_COPY(RemoveContactsJob)
    Q_DECLARE_PRIVATE(RemoveContactsJob)

    enum ProcessingMode {
        RemoveContactMode,
        RemoveContactsMode,
        RemoveMetaContactMode,
        RemoveMetaContactsMode
    };

    // Our Q_PRIVATE_SLOTS who perform the real job
    Q_PRIVATE_SLOT(d_func(), void __k__removeContact())
    Q_PRIVATE_SLOT(d_func(), void __k__removeContacts())
    Q_PRIVATE_SLOT(d_func(), void __k__removeMetaContact())
    Q_PRIVATE_SLOT(d_func(), void __k__removeMetaContacts())

public:
    RemoveContactsJob(const Nepomuk::PersonContact &contact, TelepathyBridge::RemovalModes modes, QObject *parent = 0);
    RemoveContactsJob(const Nepomuk::Person &contact, TelepathyBridge::RemovalModes modes, QObject *parent = 0);
    RemoveContactsJob(const QList<Nepomuk::PersonContact> &contacts, TelepathyBridge::RemovalModes modes, QObject *parent = 0);
    RemoveContactsJob(const QList<Nepomuk::Person> &contacts, TelepathyBridge::RemovalModes modes, QObject *parent = 0);
    virtual ~RemoveContactsJob();

    virtual void start();
};

#endif // REMOVE_CONTACTS_JOB_H
