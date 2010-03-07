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

#include "nepomuk-signal-watcher.h"

#include <KDebug>

#include <Nepomuk/ResourceManager>

#include <Soprano/Node>
#include <Soprano/Statement>

NepomukSignalWatcher* NepomukSignalWatcher::s_self = 0;

NepomukSignalWatcher::NepomukSignalWatcher()
 : m_sopranoModel(new Soprano::Util::SignalCacheModel(
            Nepomuk::ResourceManager::instance()->mainModel()))
{
    kDebug();

    // Set up the singleton instance
    s_self = this;

    // Connect to the slots we need to monitor from the Soprano Model.
    connect(m_sopranoModel,
            SIGNAL(statementAdded(Soprano::Statement)),
            SLOT(onStatementAdded(Soprano::Statement)));
}

NepomukSignalWatcher::~NepomukSignalWatcher()
{
    kDebug();

    delete m_sopranoModel;

    // Delete the singleton instance of this class
    s_self = 0;
}

NepomukSignalWatcher *NepomukSignalWatcher::instance()
{
    // Construct the singleton if hasn't been already
    if (!s_self) {
        s_self = new NepomukSignalWatcher;
    }

    // Return the singleton instance of this class
    return s_self;
}

void NepomukSignalWatcher::onStatementAdded(const Soprano::Statement &statement)
{
    foreach (NepomukSignalWatcher::Watcher *callback,
             m_subjectCallbacks.values(statement.subject().toString())) {
        // Call the callback
        callback->onStatementAdded(statement);
    }
}

void NepomukSignalWatcher::registerCallbackOnSubject(const Nepomuk::Resource &resource,
                                                     NepomukSignalWatcher::Watcher *callback)
{
    // Add this subject to the list.
    m_subjectCallbacks.insertMulti(resource.resourceUri().toString(), callback);
   // kDebug() << "Registering callback:" << resource.resourceUri() << callback;
}

