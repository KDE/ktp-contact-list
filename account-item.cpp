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

#include "account-item.h"

#include "accounts-list-model.h"


#include <KApplication>
#include <KDebug>
#include <KIcon>
#include <KLocalizedString>

#include <QtCore/QTimer>
#include <QtGui/QPainter>

#include <TelepathyQt4/PendingOperation>
#include <TelepathyQt4/PendingReady>

AccountItem::AccountItem(const Tp::AccountPtr &account, AccountsListModel *parent)
 : QObject(parent),
   m_account(account),
   m_icon(new KIcon())
{
    kDebug();

    //connect AccountPtr signals to AccountItem signals
    connect(m_account.data(),
            SIGNAL(stateChanged(bool)),
            SIGNAL(updated()));
    connect(m_account.data(),
            SIGNAL(displayNameChanged(const QString&)),
            SIGNAL(updated()));
    connect(m_account.data(),
            SIGNAL(connectionStatusChanged(Tp::ConnectionStatus)),
            SIGNAL(updated()));

    generateIcon();
}

AccountItem::~AccountItem()
{
    kDebug();

    delete m_icon;
}

Tp::AccountPtr AccountItem::account() const
{
    return m_account;
}

void AccountItem::remove()
{
    kDebug() << "Account about to be removed";

    Tp::PendingOperation *op = m_account->remove();
    connect(op,
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onAccountRemoved(Tp::PendingOperation*)));
}

const KIcon& AccountItem::icon() const
{
    Q_ASSERT(m_icon != 0);

    return *m_icon;
}

const QString AccountItem::connectionStateString() const
{
    switch (m_account->connectionStatus()) {
    case Tp::ConnectionStatusConnected:
        return i18n("Online");
    case Tp::ConnectionStatusConnecting:
        return i18n("Connecting");
    case Tp::ConnectionStatusDisconnected:
        return i18n("Disconnected");
    default:
        return "Unknown";
    }
}

const KIcon AccountItem::connectionStateIcon() const
{
    switch (m_account->connectionStatus()) {
    case Tp::ConnectionStatusConnected:
        return KIcon("user-online");
    case Tp::ConnectionStatusConnecting:
        return KIcon("user-away"); //FIXME this is bit misleading
    case Tp::ConnectionStatusDisconnected:
        return KIcon("user-offline");
    default:
        return KIcon("user-offline");
    }
}

const QString AccountItem::connectionStatusReason() const
{
    switch (m_account->connectionStatusReason())
    {
    case Tp::ConnectionStatusReasonAuthenticationFailed:
        return i18n("Authentication Failed");
    case Tp::ConnectionStatusReasonNetworkError:
        return i18n("Network Error");
    default:
        return QString();
    }
}

void AccountItem::generateIcon()
{
    kDebug();

    QString iconPath = account()->iconName();

    //if the icon has not been setted, we use the protocol icon    
    if(iconPath.isEmpty()) {
        iconPath = QString("im-%1").arg(account()->protocolName());
    }

    delete m_icon;
    m_icon = new KIcon(iconPath);

    if(!account()->isValid()) {
        //we paint a warning symbol in the right-bottom corner
        QPixmap pixmap = m_icon->pixmap(32, 32);
        QPainter painter(&pixmap);
        KIcon("dialog-error").paint(&painter, 15, 15, 16, 16);

        delete m_icon;
        m_icon = new KIcon(pixmap);
    }

    Q_EMIT(updated());
}

void AccountItem::onAccountRemoved(Tp::PendingOperation *op)
{
    kDebug();

    if (op->isError()) {
        kDebug() << "An error occurred removing the Account."
                 << op->errorName()
                 << op->errorMessage();
        return;
    }

    Q_EMIT removed();
}

void AccountItem::onTitleForCustomPages(QString mandatoryPage, QList<QString> optionalPage)
{
	kDebug();
	emit setTitleForCustomPages(mandatoryPage, optionalPage);
}

#include "account-item.moc"

