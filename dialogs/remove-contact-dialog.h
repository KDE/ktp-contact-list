/*
 * This file is part of telepathy-contactslist
 *
 * Copyright (C) 2011 by Francesco Nwokeka <francesco.nwokeka@gmail.com>
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

#ifndef REMOVECONTACTDIALOG_H
#define REMOVECONTACTDIALOG_H

#include <QDialog>

#include <TelepathyQt/Contact>

namespace Ui {
    class RemoveContactDialog;
}

class AccountsModel;

class RemoveContactDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * constructor
     * @param contact Tp::ContactPtr of the contact to remove
     * @param parent parent widget
     */
    explicit RemoveContactDialog(Tp::ContactPtr contact, QWidget *parent = 0);

    /** returns value of "block contact" checkbox */
    bool blockContact() const;


private:
    Ui::RemoveContactDialog *ui;
};


#endif  // REMOVECONTACTDIALOG_H
