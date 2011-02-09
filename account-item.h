/*
 * This file is part of telepathy-accounts-kcm
 *
 * Copyright (C) 2009 Collabora Ltd. <http://www.collabora.co.uk/>
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

#ifndef TELEPATHY_ACCOUNTS_KCM_ACCOUNT_ITEM_H
#define TELEPATHY_ACCOUNTS_KCM_ACCOUNT_ITEM_H

#include <QtCore/QObject>

#include <TelepathyQt4/Account>

class KIcon;

class AccountsListModel;

namespace Tp {
    class PendingOperation;
}

class AccountItem : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(AccountItem);

public:
    explicit AccountItem(const Tp::AccountPtr &account, AccountsListModel *parent = 0);
    virtual ~AccountItem();
    Tp::AccountPtr account() const;
    void remove();
    const KIcon& icon() const;
    const QString connectionStateString() const;
    const KIcon connectionStateIcon() const;
    const QString connectionStatusReason() const;

public Q_SLOTS:
    void onTitleForCustomPages(QString, QList<QString>);

Q_SIGNALS:
    void ready();
    void removed();
    void updated();
    void protocolSelected(QString, QString);
    void setTitleForCustomPages(QString, QList<QString>);

private Q_SLOTS:
    void generateIcon();
    void onAccountRemoved(Tp::PendingOperation *op);

private:
    Tp::AccountPtr m_account;
    KIcon* m_icon;
};


#endif // header guard

