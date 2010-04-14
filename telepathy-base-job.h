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

#ifndef TELEPATHY_BASE_JOB_H
#define TELEPATHY_BASE_JOB_H

#include <KJob>

class TelepathyBaseJobPrivate;
class TelepathyBaseJob : public KJob
{
    Q_OBJECT
    Q_DISABLE_COPY(TelepathyBaseJob)
    Q_DECLARE_PRIVATE(TelepathyBaseJob)

    Q_PRIVATE_SLOT(d_func(), void __k__tpOperationFinished(Tp::PendingOperation*))
    Q_PRIVATE_SLOT(d_func(), void __k__doEmitResult())

protected:
    explicit TelepathyBaseJob(TelepathyBaseJobPrivate &dd, QObject *parent = 0);
    virtual ~TelepathyBaseJob();

    TelepathyBaseJobPrivate * const d_ptr;
};

#endif // TELEPATHY_BASE_JOB_H
