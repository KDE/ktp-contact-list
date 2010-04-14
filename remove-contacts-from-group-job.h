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

#ifndef REMOVE_CONTACTS_FROM_GROUP_JOB_H
#define REMOVE_CONTACTS_FROM_GROUP_JOB_H

#include "telepathy-base-job.h"

namespace Nepomuk {
class PersonContact;
class Person;
}

class RemoveContactsFromGroupJobPrivate;
class RemoveContactsFromGroupJob : public TelepathyBaseJob
{
    Q_OBJECT
    Q_DISABLE_COPY(RemoveContactsFromGroupJob)
    Q_DECLARE_PRIVATE(RemoveContactsFromGroupJob)

    enum ProcessingMode {
        RemoveContactMode,
        RemoveContactsMode,
        RemoveMetaContactMode,
        RemoveMetaContactsMode
    };

    // Our Q_PRIVATE_SLOTS who perform the real job
    Q_PRIVATE_SLOT(d_func(), void __k__removeContactFromGroup())
    Q_PRIVATE_SLOT(d_func(), void __k__removeContactsFromGroup())
    Q_PRIVATE_SLOT(d_func(), void __k__removeMetaContactFromGroup())
    Q_PRIVATE_SLOT(d_func(), void __k__removeMetaContactsFromGroup())

public:
    RemoveContactsFromGroupJob(const QString &group, const Nepomuk::PersonContact &contact, QObject *parent = 0);
    RemoveContactsFromGroupJob(const QString &group, const Nepomuk::Person &contact, QObject *parent = 0);
    RemoveContactsFromGroupJob(const QString &group, const QList<Nepomuk::PersonContact> &contacts, QObject *parent = 0);
    RemoveContactsFromGroupJob(const QString &group, const QList<Nepomuk::Person> &contacts, QObject *parent = 0);
    virtual ~RemoveContactsFromGroupJob();

    virtual void start();
};

#endif // REMOVE_CONTACTS_FROM_GROUP_JOB_H
