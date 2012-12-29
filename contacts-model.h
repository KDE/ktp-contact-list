/*
 * Model of all accounts with inbuilt grouping and filtering
 *
 * Copyright (C) 2011 David Edmundson <kde@davidedmundson.co.uk>
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

#ifndef CONTACTSMODEL2_H
#define CONTACTSMODEL2_H

#include <KTp/Models/contacts-filter-model.h>
#include <KTp/Models/contacts-list-model.h>
#include <KTp/Models/abstract-grouping-proxy-model.h>

#include <TelepathyQt/Types>

class ContactsModel2 : public ContactsFilterModel
{
public:
    enum GroupMode {NoGrouping, AccountGrouping, GroupGrouping};

    ContactsModel2(QObject *parent);
    void setAccountManager (const Tp::AccountManagerPtr &accountManager);

    void setGroupMode(GroupMode mode);
    GroupMode groupMode() const;

private:
    GroupMode m_groupMode;
    QWeakPointer<AbstractGroupingProxyModel> m_proxy;
    KTp::ContactsListModel *m_source;
    Tp::AccountManagerPtr m_accountManager;
};

#endif // CONTACTSMODEL_H
