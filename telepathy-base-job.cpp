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

#include "telepathy-base-job_p.h"

#include "telepathy-bridge.h"

#include <TelepathyQt4/PendingOperation>

#include <KLocalizedString>

TelepathyBaseJobPrivate::TelepathyBaseJobPrivate(TelepathyBaseJob *parent)
    : q_ptr(parent)
{
}

TelepathyBaseJobPrivate::~TelepathyBaseJobPrivate()
{
}

void TelepathyBaseJobPrivate::addOperation(Tp::PendingOperation *op)
{
    Q_Q(TelepathyBaseJob);

    // Add the operation to the list
    operations << op;

    // Attach the operation to our listener
    q->connect(op, SIGNAL(finished(Tp::PendingOperation*)), q, SLOT(__k__tpOperationFinished(Tp::PendingOperation*)));
}

TelepathyBaseJob::TelepathyBaseJob(TelepathyBaseJobPrivate& dd, QObject* parent)
    : KJob(parent)
    , d_ptr(&dd)
{
}

TelepathyBaseJob::~TelepathyBaseJob()
{
    delete d_ptr;
}

void TelepathyBaseJobPrivate::__k__tpOperationFinished(Tp::PendingOperation* op)
{
    // First of all check if the operation is in our list
    if (!operations.contains(op)) {
        // WTF?
        // TODO: This should never happen, should we do something?
        return;
    }

    if (op->isError()) {
        // Ouch. Add it to the error roster
        telepathyErrors << qMakePair(op->errorName(), op->errorMessage());
    }

    // Remove it from the list
    operations.removeOne(op);

    // Ok, are we done yet?
    if (operations.isEmpty()) {
        // It looks like we are. Let's pass the ball to doEmitResult.
        __k__doEmitResult();
    }
}

void TelepathyBaseJobPrivate::__k__doEmitResult()
{
    Q_Q(TelepathyBaseJob);

    // Before streaming out: are there any telepathy errors?
    if (!telepathyErrors.isEmpty()) {
        // Hmm, bad stuff. Let's handle them here.
        // FIXME: Maybe there's a better formatting for this specific error string?

        QString errorMessage = i18np("Telepathy reported an error while performing the requested operation:",
                                     "Telepathy reported %1 errors while performing the requested operation:",
                                     telepathyErrors.size());

        QList< QPair< QString, QString > >::const_iterator i;
        for (i = telepathyErrors.constBegin(); i != telepathyErrors.constEnd(); ++i) {
            errorMessage.append('\n');
            errorMessage.append(i18nc("The following format is: ' - <error name>: <error message>'", " - %1: %2",
                                      (*i).first, (*i).second));
        }

        // Ok, let's set the errors now
        q->setError(TelepathyBridge::TelepathyErrorError);
        q->setErrorText(errorMessage);
    }

    // The job has been finished
    q->emitResult();
}

#include "telepathy-base-job.moc"
