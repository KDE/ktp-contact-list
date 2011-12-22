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

#include "avatar-button.h"

#include <QWidgetAction>

#include <KDebug>
#include <KFileDialog>
#include <KMessageBox>
#include <KMenu>
#include <KLocalizedString>
#include <KSharedConfig>

#include <KTp/Models/accounts-model.h>
#include <KTp/Models/accounts-model-item.h>

#include "fetch-avatar-job.h"

AvatarButton::AvatarButton(QWidget *parent)
    : QToolButton(parent),
      //m_accountManager(0),
      m_accountsModel(0)
{
//     m_accountManager = am;
//     m_accountsModel = model;
    m_avatarButtonMenu = new KMenu(this);

    QToolButton *loadFromFileButton = new QToolButton(this);
    loadFromFileButton->setIcon(KIcon("document-open-folder"));
    loadFromFileButton->setIconSize(QSize(48, 48));
    loadFromFileButton->setText(i18n("Load from file..."));
    loadFromFileButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    QWidgetAction *loadFromFileAction = new QWidgetAction(this);
    loadFromFileAction->setDefaultWidget(loadFromFileButton);

    connect(loadFromFileButton, SIGNAL(clicked(bool)),
            this, SLOT(loadAvatarFromFile()));

    m_avatarButtonMenu->addAction(loadFromFileAction);

    setMenu(m_avatarButtonMenu);
}

AvatarButton::~AvatarButton()
{

}

void AvatarButton::initialize(AccountsModel* model, const Tp::AccountManagerPtr& manager)
{
    m_accountsModel = model;
    m_accountManager = manager;
}

void AvatarButton::loadAvatar(const Tp::AccountPtr &account)
{
    if (!account->avatar().avatarData.isEmpty()) {
        QIcon icon;
        Tp::Avatar avatar = account->avatar();
        icon.addPixmap(QPixmap::fromImage(QImage::fromData(avatar.avatarData)).scaled(48, 48));

        QToolButton *avatarMenuItem = new QToolButton(this);
        avatarMenuItem->setIcon(icon);
        avatarMenuItem->setIconSize(QSize(48, 48));
        avatarMenuItem->setText(i18nc("String in menu saying Use avatar from account X",
                                    "Use from %1", account->displayName()));
        avatarMenuItem->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

        QWidgetAction *avatarAction = new QWidgetAction(m_avatarButtonMenu);
        avatarAction->setDefaultWidget(avatarMenuItem);
        avatarAction->setData(account->uniqueIdentifier());

        //this connect is chained to the avatarAction, because avatarAction holds the account id
        //which is then extracted and used
        connect(avatarMenuItem, SIGNAL(clicked(bool)),
                avatarAction, SIGNAL(triggered(bool)));

        connect(avatarAction, SIGNAL(triggered(bool)),
                this, SLOT(selectAvatarFromAccount()));

        m_avatarButtonMenu->addAction(avatarAction);
    }
}

void AvatarButton::selectAvatarFromAccount()
{
    selectAvatarFromAccount(qobject_cast<QWidgetAction*>(sender())->data().toString());
}

void AvatarButton::selectAvatarFromAccount(const QString &accountUID)
{
    if (accountUID.isEmpty()) {
        kDebug() << "Supplied accountUID is empty, aborting...";
        return;
    }

    if (m_accountsModel->accountItemForId(accountUID) == 0) {
        kDebug() << "Chosen account ID does not exist, aborting..";

        //no point of keeping the config if the previously set account ID does not exist
        KSharedConfigPtr config = KGlobal::config();
        KConfigGroup avatarGroup(config, "Avatar");
        avatarGroup.deleteGroup();
        avatarGroup.config()->sync();

        return;
    }

    Tp::Avatar avatar = qobject_cast<AccountsModelItem*>(m_accountsModel->accountItemForId(accountUID))->data(AccountsModel::AvatarRole).value<Tp::Avatar>();

    foreach (const Tp::AccountPtr &account, m_accountManager->allAccounts()) {
        //don't set the avatar for the account from where it was taken
        if (account->uniqueIdentifier() == accountUID) {
            continue;
        }

        account->setAvatar(avatar);
    }

    //add the selected avatar as the icon of avatar button
    QIcon icon;
    icon.addPixmap(QPixmap::fromImage(QImage::fromData(avatar.avatarData)).scaled(48, 48));
    setIcon(icon);

    m_avatarButtonMenu->close();

    //save the selected account into config
    KSharedConfigPtr config = KGlobal::config();
    KConfigGroup avatarGroup(config, "Avatar");
    avatarGroup.writeEntry("method", "account");
    avatarGroup.writeEntry("source", accountUID);
    avatarGroup.config()->sync();
}

void AvatarButton::loadAvatarFromFile()
{
    if (m_accountManager->allAccounts().isEmpty()) {
        int returnCode = KMessageBox::warningYesNo(this,
                                                   i18nc("Dialog text", "You have no accounts set. Would you like to set one now?"),
                                                   i18nc("Dialog caption", "No accounts set"));

        if (returnCode == KMessageBox::Yes) {
            emit openKCMSettings();
            loadAvatarFromFile();
        } else {
            return;
        }
    } else {
        KUrl fileUrl = KFileDialog::getImageOpenUrl(KUrl(), this,
                                                    i18n("Please choose your avatar"));

        if (!fileUrl.isEmpty()) {
            FetchAvatarJob *job = new FetchAvatarJob(fileUrl, this);

            connect(job, SIGNAL(result(KJob*)),
                    this, SLOT(onAvatarFetched(KJob*)));

            job->start();
        } else {
            return;
        }
    }
}

void AvatarButton::onAvatarFetched(KJob *job)
{
    if (job->error()) {
        KMessageBox::error(this, job->errorString());
        return;
    }

    //this should never be true, but better one "if" than a crash
    if (m_accountManager->allAccounts().isEmpty()) {
        int returnCode = KMessageBox::warningYesNo(this,
                                                   i18nc("Dialog text", "You have no accounts set. Would you like to set one now?"),
                                                   i18nc("Dialog caption", "No accounts set"));

        if (returnCode == KMessageBox::Yes) {
            emit openKCMSettings();
        } else {
            return;
        }
    } else {

        FetchAvatarJob *fetchJob = qobject_cast< FetchAvatarJob* >(job);

        Q_ASSERT(fetchJob);

        foreach (const Tp::AccountPtr &account, m_accountManager->allAccounts()) {
            Tp::PendingOperation *op = account->setAvatar(fetchJob->avatar());

            //connect for eventual error displaying
            connect(op, SIGNAL(finished(Tp::PendingOperation*)),
                    this, SIGNAL(operationFinished(Tp::PendingOperation*)));
        }

        //add the selected avatar to the avatar button
        QIcon icon;
        icon.addPixmap(QPixmap::fromImage(QImage::fromData(fetchJob->avatar().avatarData)).scaled(48, 48));
        setIcon(icon);

        //since all the accounts will have the same avatar,
        //we take simply the first in AM and use this in config
        KSharedConfigPtr config = KGlobal::config();
        KConfigGroup avatarGroup(config, "Avatar");
        avatarGroup.writeEntry("method", "account");
        avatarGroup.writeEntry("source", m_accountManager->allAccounts().first()->uniqueIdentifier());
        avatarGroup.config()->sync();
    }
}
