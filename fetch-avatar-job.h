/*
 * This file is part of telepathy-contactslist-prototype
 *
 * Copyright (C) 2011 Collabora Ltd. <info@collabora.co.uk>
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

#ifndef FETCH_AVATAR_JOB_H
#define FETCH_AVATAR_JOB_H

#include <KJob>

#include <TelepathyQt/Types>

class KUrl;
class FetchAvatarJob : public KJob
{
    Q_OBJECT

public:
    explicit FetchAvatarJob(const KUrl &url, QObject *parent = 0);
    virtual ~FetchAvatarJob();

    void start();

    Tp::Avatar avatar() const;

private:
    class Private;
    Private * const d;

    Q_PRIVATE_SLOT(d, void _k_onMimeTypeDetected(KIO::Job*,QString))
    Q_PRIVATE_SLOT(d, void _k_onDataFromJob(KIO::Job*,QByteArray))
    Q_PRIVATE_SLOT(d, void _k_onJobFinished(KJob*))
};

#endif // FETCH_AVATAR_JOB_H
