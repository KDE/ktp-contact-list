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

#include "add-meta-contact-job.h"

#include "telepathy-base-job_p.h"
#include <QTimer>
#include <pimo.h>
#include <person.h>
#include <nco.h>
#include <informationelement.h>
#include <personcontact.h>
#include <nao.h>
#include <Nepomuk/Variant>

class AddMetaContactJobPrivate : public TelepathyBaseJobPrivate
{
    Q_DECLARE_PUBLIC(AddMetaContactJob)
    AddMetaContactJob * const q_ptr;

public:
    AddMetaContactJobPrivate(AddMetaContactJob *parent)
        : TelepathyBaseJobPrivate(parent), q_ptr(parent)
    {}
    virtual ~AddMetaContactJobPrivate() {}

    QString name;
    QList< Nepomuk::PersonContact > contacts;

    // Operation Q_PRIVATE_SLOTS
    void __k__addMetaContact();
};

AddMetaContactJob::AddMetaContactJob(const QString& name, const QList< Nepomuk::PersonContact > contacts,
                                     QObject *parent)
    : TelepathyBaseJob(*new AddMetaContactJobPrivate(this), parent)
{
    Q_D(AddMetaContactJob);

    d->name = name;
    d->contacts = contacts;
}

AddMetaContactJob::~AddMetaContactJob()
{

}

void AddMetaContactJob::start()
{
    // Go for it
    QTimer::singleShot(0, this, SLOT(__k__addMetaContact()));
}

void AddMetaContactJobPrivate::__k__addMetaContact()
{
    Q_Q(AddMetaContactJob);

    // FIXME: Get the Nepomuk Resource for myself in the standardised way, once it is standardised.
    Nepomuk::Resource me(QUrl::fromEncoded("nepomuk:/myself"));

    // Create the metacontact
    Nepomuk::Person person;
    Nepomuk::Thing thing;
    person.setProperty(Nepomuk::Vocabulary::NAO::prefLabel(),
                       name);

    // Ok, now add the contacts to the metacontact
    foreach (const Nepomuk::PersonContact &contact, contacts) {
        person.addGroundingOccurrence(contact);
    }

    q->emitResult();
}

#include "add-meta-contact-job.moc"
