/*
 * This file is part of telepathy-contactslist-prototype
 *
 * Copyright (C) 2010 Collabora Ltd. <info@collabora.co.uk>
 *   @Author George Goldberg <george.goldberg@collabora.co.uk>
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

#ifndef TELEPATHY_CONTACTSLIST_PROTOTYPE_META_CONTACT_ITEM_H
#define TELEPATHY_CONTACTSLIST_PROTOTYPE_META_CONTACT_ITEM_H

#include "abstract-tree-item.h"
#include "nepomuk-signal-watcher.h"

#include "person.h"
#include "personcontact.h"

#include <QObject>

class MetaContactItem : public QObject,
                        public AbstractTreeItem,
                        protected NepomukSignalWatcher::Watcher
{

    Q_OBJECT

public:
    enum MetaContactType {
        RealMetaContact = 0,
        FakeMetaContact = 1
    };

    MetaContactItem(MetaContactType type, QObject *parent = 0);
    ~MetaContactItem();

    QString displayName() const;

    void setPimoPerson(const Nepomuk::Person &pimoPerson);

    virtual void onStatementAdded(const Soprano::Statement &statement);

Q_SIGNALS:
    void dirty();

private Q_SLOTS:
//    void updatePresenceIcon();

private:
    Q_DISABLE_COPY(MetaContactItem);

    MetaContactType m_type;
    Nepomuk::Person m_pimoPerson;
};


#endif // Header guard

