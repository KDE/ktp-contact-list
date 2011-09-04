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

#ifndef JOINCHATROOMDIALOG_H
#define JOINCHATROOMDIALOG_H

#include <KDialog>
#include <TelepathyQt4/AccountManager>

namespace Ui {
    class JoinChatRoomDialog;
}

class JoinChatRoomDialog : public KDialog
{
    Q_OBJECT

public:
    explicit JoinChatRoomDialog(Tp::AccountManagerPtr accountManager, QWidget *parent = 0);
    ~JoinChatRoomDialog();

    Tp::AccountPtr selectedAccount() const;     /** returns selected account */
    QString selectedChatRoom() const;           /** returns selected chat room */

private slots:
    void onTextChanged(QString newText);

private:
    QList<Tp::AccountPtr> m_accounts;
    Ui::JoinChatRoomDialog *ui;
};


#endif  // JOINCHATROOMDIALOG_H
