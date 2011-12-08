/*
 *
 * Copyright (C) 2011  Martin Klapetek <martin.klapetek@gmail.com>
 * Copyright (C) 2011  David Edmundson <kde@davidedmundson.co.uk>
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

#include "account-buttons-panel.h"

#include <TelepathyQt/AccountManager>
#include <TelepathyQt/Account>
#include <TelepathyQt/AccountSet>

#include <QHBoxLayout>

#include "account-button.h"

AccountButtonsPanel::AccountButtonsPanel(QWidget *parent) :
    QWidget(parent),
    m_layout(new QHBoxLayout(this))
{
    m_layout->insertStretch(-1);
}

void AccountButtonsPanel::setAccountManager(const Tp::AccountManagerPtr &accountManager)
{
    m_enabledAccounts = accountManager->enabledAccounts();
    foreach (const Tp::AccountPtr &account, m_enabledAccounts->accounts()) {
        onAccountAdded(account);
    }

    connect(m_enabledAccounts.data(), SIGNAL(accountAdded(Tp::AccountPtr)), SLOT(onAccountAdded(Tp::AccountPtr)));
    connect(m_enabledAccounts.data(), SIGNAL(accountRemoved(Tp::AccountPtr)), SLOT(onAccountRemoved(Tp::AccountPtr)));
}

void AccountButtonsPanel::onAccountAdded(const Tp::AccountPtr &account)
{
    //add a new button to the layout
    AccountButton *button = new AccountButton(account, this);
    button->setObjectName(account->uniqueIdentifier());
    m_layout->insertWidget(m_layout->count() - 1, button);
}

void AccountButtonsPanel::onAccountRemoved(const Tp::AccountPtr &account)
{
    //try and find relevant account button - and remove it.
    AccountButton *button = qobject_cast<AccountButton*>(findChild<AccountButton *>(account->uniqueIdentifier()));
    if (button) {
        delete button;
    }
}

#include "account-buttons-panel.moc"
