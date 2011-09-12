/*
 * Button representing user's Avatar
 *
 * Copyright (C) 2011  Martin Klapetek <martin.klapetek@gmail.com>
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

#ifndef AVATAR_BUTTON_H
#define AVATAR_BUTTON_H

#include <QtGui/QToolButton>
#include <TelepathyQt4/Account>

namespace Tp {
class PendingOperation;
}

class AccountsModel;
class KJob;
class KMenu;

class AvatarButton : public QToolButton
{
    Q_OBJECT

public:
    AvatarButton(QWidget* parent = 0);
    ~AvatarButton();
    void initialize(AccountsModel *model, const Tp::AccountManagerPtr &manager);

Q_SIGNALS:
    void openKCMSettings();
    void operationFinished(Tp::PendingOperation*);

public Q_SLOTS:
    void loadAvatar(const Tp::AccountPtr &account);
    void selectAvatarFromAccount(const QString &accountUID);

private Q_SLOTS:
    void selectAvatarFromAccount();
    void loadAvatarFromFile();

    void onAvatarFetched(KJob*);

private:
    KMenu                 *m_avatarButtonMenu;
    Tp::AccountManagerPtr  m_accountManager;
    AccountsModel         *m_accountsModel;
};
