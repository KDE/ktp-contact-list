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

#include "fetch-avatar-job.h"

#include <KUrl>
#include <KLocalizedString>

#include <KIO/Job>

class FetchAvatarJob::Private
{
public:
    Private(FetchAvatarJob *q) : q(q) {}
    ~Private() {}

    void _k_onMimeTypeDetected(KIO::Job *job, const QString &mimetype);
    void _k_onDataFromJob(KIO::Job *job, const QByteArray &data);
    void _k_onJobFinished(KJob *job);

    Tp::Avatar avatar;
    KUrl url;

    FetchAvatarJob *q;
};

FetchAvatarJob::FetchAvatarJob(const KUrl& url, QObject* parent)
    : KJob(parent)
    , d(new Private(this))
{
    d->url = url;
}

FetchAvatarJob::~FetchAvatarJob()
{
    delete d;
}

Tp::Avatar FetchAvatarJob::avatar() const
{
    return d->avatar;
}

void FetchAvatarJob::start()
{
    if (d->url.isEmpty() || !d->url.isValid()) {
        setError(1);
        emitResult();
        return;
    }

    KIO::TransferJob *job = KIO::get(d->url);

    connect(job, SIGNAL(mimetype(KIO::Job*,QString)),
            this, SLOT(_k_onMimeTypeDetected(KIO::Job*,QString)));
    connect(job, SIGNAL(data(KIO::Job*,QByteArray)),
            this, SLOT(_k_onDataFromJob(KIO::Job*,QByteArray)));
    connect(job, SIGNAL(result(KJob*)),
            this, SLOT(_k_onJobFinished(KJob*)));
}

void FetchAvatarJob::Private::_k_onMimeTypeDetected(KIO::Job *job, const QString &mimetype)
{
    if (!mimetype.contains("image/")) {
        q->setErrorText(i18n("The file you have selected does not seem to be an image.\n"
                             "Please select an image file."));
        q->setError(1);
        q->emitResult();

        disconnect(job, SIGNAL(result(KJob*)),
                   q, SLOT(_k_onJobFinished(KJob*)));
        disconnect(job, SIGNAL(data(KIO::Job*,QByteArray)),
                   q, SLOT(_k_onDataFromJob(KIO::Job*,QByteArray)));

        job->kill();

        return;
    }

    avatar.MIMEType = mimetype;
}

void FetchAvatarJob::Private::_k_onDataFromJob(KIO::Job *job, const QByteArray &data)
{
    Q_UNUSED(job)
    avatar.avatarData.append(data);
}

void FetchAvatarJob::Private::_k_onJobFinished(KJob *job)
{
    q->setError(job->error());
    q->emitResult();
}

#include "fetch-avatar-job.moc"
