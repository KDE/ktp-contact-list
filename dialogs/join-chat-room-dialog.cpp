/*
 * This file is part of telepathy-contactslist-prototype
 *
 * Copyright (C) 2011 Francesco Nwokeka <francesco.nwokeka@gmail.com>
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

#include "join-chat-room-dialog.h"
#include "ui_join-chat-room-dialog.h"
#include "../models/accounts-model.h"

#include <KDE/KPushButton>
#include <TelepathyQt4/AccountManager>

JoinChatRoomDialog::JoinChatRoomDialog(Tp::AccountManagerPtr accountManager, QWidget* parent)
    : KDialog(parent, Qt::Dialog)
    , ui(new Ui::JoinChatRoomDialog)
{
    QWidget *joinChatRoomDialog = new QWidget(this);
    m_accounts = accountManager->allAccounts();
    ui->setupUi(joinChatRoomDialog);
    setMainWidget(joinChatRoomDialog);

    // disable OK button on start
    button(Ok)->setEnabled(false);


    // populate combobox with accounts that support chat rooms
    for (int i = 0; i < m_accounts.count(); ++i) {
        Tp::AccountPtr acc = m_accounts.at(i);

        if (acc->capabilities().textChatrooms() && acc->isOnline()) {
            // add unique data to identify the correct account later on
            ui->comboBox->addItem(KIcon(acc->iconName()), acc->displayName(), acc->uniqueIdentifier());
        }
    }

    // connects
    connect(ui->lineEdit, SIGNAL(textChanged(QString)), this, SLOT(onTextChanged(QString)));
}

JoinChatRoomDialog::~JoinChatRoomDialog()
{
    delete ui;
}

Tp::AccountPtr JoinChatRoomDialog::selectedAccount() const
{
    Tp::AccountPtr account;
    bool found = false;

    for (int i = 0; i < m_accounts.count() && !found; ++i) {
        if (m_accounts.at(i)->uniqueIdentifier() == ui->comboBox->itemData(ui->comboBox->currentIndex()).toString()) {
            account = m_accounts.at(i);
            found = true;
        }
    }

    // account should never be empty
    return account;
}

QString JoinChatRoomDialog::selectedChatRoom() const
{
    return ui->lineEdit->text();
}

void JoinChatRoomDialog::onTextChanged(QString newText)
{
    if (button(Ok)->isEnabled() == newText.isEmpty()) {
        button(Ok)->setEnabled(!newText.isEmpty());
    }
}

