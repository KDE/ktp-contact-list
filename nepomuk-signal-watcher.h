/*
 * This file is part of telepathy-contactslist-prototype
 *
 * Copyright (C) 2010 Collabora Ltd. <info@collabora.co.uk>
 *   @author George Goldberg <george.goldberg@collabora.co.uk>
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

#ifndef NEPOMUK_SIGNAL_WATCHER_H
#define NEPOMUK_SIGNAL_WATCHER_H

#include <Nepomuk/Resource>

#include <QtCore/QHash>
#include <QtCore/QObject>
#include <QtCore/QPair>

#include <Soprano/Util/SignalCacheModel>

/**
 * This class allows you to register an interest in a specific Nepomuk resource
 * and be notified whenever that resource is either the subject or object of a statement
 * added or removed from the Nepomuk database. It calls the appropriate callback that is
 * registered with it, when something happens concerning a specific resource.
 *
 * The purpose of all this is to improve performance and code readability (hopefully) by
 * not needing to have Soprano::SignalCachedModels doing stuff all over the place in our
 * source code.
 *
 * Really, the purpose of this class should be fulfilled by Nepomuk or Soprano in some better
 * way, but until that day arrives, we need this temporary solution.
 *
 * To use this class, the object must inherit from NepomukSignalWatcher::Watcher, and reimplement
 * onStatementAdded() and/or onStatementRemoved(). It must then registerCallbackOnSubject()
 * etc for all the resource it is interested in.
 */
class NepomukSignalWatcher : public QObject
{
    Q_OBJECT

public:
    class Watcher {
    public:
        Watcher() { }
        virtual ~Watcher() { }
        virtual void onStatementAdded(const Soprano::Statement &statement) { Q_UNUSED(statement); }
        virtual void onStatementRemoved(const Soprano::Statement &statement) { Q_UNUSED(statement); }
    };

    static NepomukSignalWatcher *instance();

    virtual ~NepomukSignalWatcher();

    void registerCallbackOnSubject(const Nepomuk::Resource &resource,
                                   NepomukSignalWatcher::Watcher *callback);

private Q_SLOTS:
    void onStatementAdded(const Soprano::Statement &statement);

private:
    Q_DISABLE_COPY(NepomukSignalWatcher);

    NepomukSignalWatcher();
    static NepomukSignalWatcher *s_self;

    Soprano::Util::SignalCacheModel *m_sopranoModel;

    QHash<QString, NepomukSignalWatcher::Watcher*> m_subjectCallbacks;
};


#endif  // Header guard

