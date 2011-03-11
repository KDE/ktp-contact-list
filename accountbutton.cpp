/*
 * Tool button which controls account's presence
 *
 * Copyright (C) 2011 Martin Klapetek <martin dot klapetek at gmail dot com>
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

#include <QAction>
#include <QPainter>
#include <QPixmap>

#include <KIcon>
#include <KLocale>
#include <KPixmapSequenceOverlayPainter>
#include <KPixmapSequence>

#include <TelepathyQt4/PendingOperation>

#include "accountbutton.h"

static Tp::ConnectionPresenceType accountPresenceTypes[] = { Tp::ConnectionPresenceTypeAvailable,
    Tp::ConnectionPresenceTypeAway,
    Tp::ConnectionPresenceTypeAway,
    Tp::ConnectionPresenceTypeBusy,
    Tp::ConnectionPresenceTypeBusy,
    Tp::ConnectionPresenceTypeExtendedAway,
    Tp::ConnectionPresenceTypeHidden,
    Tp::ConnectionPresenceTypeOffline };

static const char *accountPresenceStatuses[] = { "available", "away", "brb", "busy",
    "dnd", "xa", "hidden", "offline" };

AccountButton::AccountButton(const Tp::AccountPtr &account, QWidget* parent): QToolButton(parent),m_busyOverlay(0)
{
    m_account = account;
    m_statusIndex = -1;

    m_busyOverlay = new KPixmapSequenceOverlayPainter(this);
    m_busyOverlay->setWidget(this);
    m_busyOverlay->setSequence(KPixmapSequence(QString("process-working")));

    QString iconPath = account->iconName();

    //if the icon has not been set, we use the protocol icon
    if(iconPath.isEmpty()) {
        iconPath = QString("im-%1").arg(account->protocolName());
    }

    setIcon(KIcon(iconPath));

    if(!account->isValid()) {
        //we paint a warning symbol in the right-bottom corner
        QPixmap pixmap = icon().pixmap(32, 32);
        QPainter painter(&pixmap);
        KIcon("dialog-error").paint(&painter, 15, 15, 16, 16);

        setIcon(KIcon(pixmap));
    }

    setMaximumWidth(24);

    setAutoRaise(true);
    setPopupMode(QToolButton::InstantPopup);
    setArrowType(Qt::NoArrow);

    QActionGroup *presenceActions = new QActionGroup(this);
    presenceActions->setExclusive(true);

    QAction *onlineAction =     new QAction(KIcon("user-online"), i18nc("@action:inmenu", "Available"), this);
    QAction *awayAction =       new QAction(KIcon("user-away"), i18nc("@action:inmenu", "Away"), this);
    QAction *brbAction =        new QAction(KIcon("user-busy"), i18nc("@action:inmenu", "Be right back"), this);
    QAction *busyAction =       new QAction(KIcon("user-busy"), i18nc("@action:inmenu", "Busy"), this);
    QAction *dndAction =        new QAction(KIcon("user-busy"), i18nc("@action:inmenu", "Do not disturb"), this);
    QAction *xaAction =         new QAction(KIcon("user-away-extended"), i18nc("@action:inmenu", "Extended Away"), this);
    QAction *invisibleAction =  new QAction(KIcon("user-invisible"), i18nc("@action:inmenu", "Invisible"), this);
    QAction *offlineAction =    new QAction(KIcon("user-offline"), i18nc("@action:inmenu", "Offline"), this);

    //let's set the indexes as data(), so we don't have to rely on putting the actions into indexed list/menu/etc
    onlineAction->setData(0);
    awayAction->setData(1);
    brbAction->setData(2);
    busyAction->setData(3);
    dndAction->setData(4);
    xaAction->setData(5);
    invisibleAction->setData(6);
    offlineAction->setData(7);

    presenceActions->addAction(onlineAction);
    presenceActions->addAction(awayAction);
    presenceActions->addAction(brbAction);
    presenceActions->addAction(busyAction);
    presenceActions->addAction(dndAction);
    presenceActions->addAction(xaAction);
    presenceActions->addAction(invisibleAction);
    presenceActions->addAction(offlineAction);

    addActions(presenceActions->actions());

    //make all the actions checkable
    foreach(QAction *a, actions()) {
        a->setCheckable(true);

        if(m_account->currentPresence().status() == QLatin1String(accountPresenceStatuses[a->data().toInt()])) {
            a->setChecked(true);
            m_statusIndex = a->data().toInt();
        }
    }

    connect(this, SIGNAL(triggered(QAction*)),
            this, SLOT(setAccountStatus(QAction*)));

    connect(m_account.data(),SIGNAL(connectionStatusChanged(Tp::ConnectionStatus)),
            this, SLOT(connectionChanged(Tp::ConnectionStatus)));

    connect(m_account.data(), SIGNAL(currentPresenceChanged(Tp::Presence)),
            this, SLOT(preseneceChanged(Tp::Presence)));

    if(m_statusIndex == -1) {
        m_statusIndex = 7;
    }

    updateToolTip();
}

QString AccountButton::accountId()
{
    return m_account->uniqueIdentifier();
}

void AccountButton::setAccountStatus(QAction *action)
{
    int statusIndex = action->data().toInt();
    Q_ASSERT(statusIndex >= 0 && statusIndex <= 7);

    m_statusIndex = statusIndex;

    Tp::SimplePresence presence;
    presence.type = accountPresenceTypes[statusIndex];
    presence.status = QLatin1String(accountPresenceStatuses[statusIndex]);

    Q_ASSERT(!m_account.isNull());

    Tp::PendingOperation* presenceRequest = m_account->setRequestedPresence(presence);

    connect(presenceRequest, SIGNAL(finished(Tp::PendingOperation*)),
            this, SLOT(updateToolTip()));
}

void AccountButton::updateToolTip()
{
    if(m_account->currentPresence().statusMessage().isEmpty()) {
        setToolTip(QString("%1\n%2").arg(m_account->displayName())
                                    .arg(actions().value(m_statusIndex)->text()));
    }
    else {
        setToolTip(QString("%1\n%2\n%3").arg(m_account->displayName())
                                        .arg(actions().value(m_statusIndex)->text())
                                        .arg(m_account->currentPresence().statusMessage()));
    }
}

void AccountButton::connectionChanged(Tp::ConnectionStatus status)
{
    switch (status) {
    case Tp::ConnectionStatusConnecting:
        showBusyIndicator();
        break;
    case Tp::ConnectionStatusConnected:
    case Tp::ConnectionStatusDisconnected:
        hideBusyIndicator();
        break;
    default:
        break;
    }
}

void AccountButton::showBusyIndicator()
{
    m_busyOverlay->start();
}

void AccountButton::hideBusyIndicator()
{
    m_busyOverlay->stop();
}

void AccountButton::preseneceChanged(Tp::Presence presence)
{
    foreach(QAction *a, actions()) {
        if(presence.status() == QLatin1String(accountPresenceStatuses[a->data().toInt()])) {
            a->setChecked(true);
            m_statusIndex = a->data().toInt();
            break;
        }
    }
}