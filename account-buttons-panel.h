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

#ifndef ACCOUNTBUTTONS_H
#define ACCOUNTBUTTONS_H

#include <QWidget>

#include <TelepathyQt/AccountSet>

class QHBoxLayout;

/** This class handles a horizontal list of AccountButton widgets which allow a user to control an account presence.
  * Only enabled accounts are shown, newly enabled accounts are added/removed as appropriate.
  */

class AccountButtonsPanel : public QWidget
{
    Q_OBJECT
public:
    explicit AccountButtonsPanel(QWidget *parent = 0);
    void setAccountManager(const Tp::AccountManagerPtr &accountManager);

signals:

public slots:

private slots:
    void onAccountAdded(const Tp::AccountPtr &account);
    void onAccountRemoved(const Tp::AccountPtr &account);

private:
    Tp::AccountSetPtr m_enabledAccounts;
    QHBoxLayout *m_layout;
};

#endif // ACCOUNTBUTTONS_H
