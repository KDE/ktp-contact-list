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

#include "account-button.h"

AccountButton::AccountButton(const Tp::AccountPtr &account, QWidget* parent)
  : QToolButton(parent), m_busyOverlay(0)
{
    m_account = account;

    m_busyOverlay = new KPixmapSequenceOverlayPainter(this);
    m_busyOverlay->setWidget(this);
    m_busyOverlay->setSequence(KPixmapSequence(QString("process-working")));

    QString iconPath = account->iconName();

    setIcon(KIcon(iconPath));

    if (!account->isValid()) {
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

    //let's set the presences as data so we can easily just use the Tp::Presence when the action has been triggered
    onlineAction->setData(qVariantFromValue(Tp::Presence::available()));
    awayAction->setData(qVariantFromValue(Tp::Presence::away()));
    brbAction->setData(qVariantFromValue(Tp::Presence::brb()));
    busyAction->setData(qVariantFromValue(Tp::Presence::busy()));
    dndAction->setData(qVariantFromValue(Tp::Presence::busy()));
    xaAction->setData(qVariantFromValue(Tp::Presence::xa()));
    invisibleAction->setData(qVariantFromValue(Tp::Presence::hidden()));
    offlineAction->setData(qVariantFromValue(Tp::Presence::offline()));

    presenceActions->addAction(onlineAction);
    presenceActions->addAction(awayAction);
    presenceActions->addAction(brbAction);
    presenceActions->addAction(busyAction);
    presenceActions->addAction(dndAction);
    presenceActions->addAction(xaAction);
    presenceActions->addAction(invisibleAction);
    presenceActions->addAction(offlineAction);

    addActions(presenceActions->actions());

    //make all the actions checkable and set the current status as checked
    foreach (QAction *a, actions()) {
        a->setCheckable(true);

        if (m_account->currentPresence().status() == qVariantValue<Tp::Presence>(a->data()).status()) {
            a->setChecked(true);
        }
    }

    connect(this, SIGNAL(triggered(QAction*)),
            this, SLOT(setAccountStatus(QAction*)));

    connect(m_account.data(),SIGNAL(connectionStatusChanged(Tp::ConnectionStatus)),
            this, SLOT(connectionChanged(Tp::ConnectionStatus)));

    connect(m_account.data(), SIGNAL(currentPresenceChanged(Tp::Presence)),
            this, SLOT(preseneceChanged(Tp::Presence)));

    updateToolTip();
}

QString AccountButton::accountId()
{
    return m_account->uniqueIdentifier();
}

void AccountButton::setAccountStatus(QAction *action)
{
    Tp::SimplePresence presence;
    presence.type = qVariantValue<Tp::Presence>(action->data()).type();
    presence.status = qVariantValue<Tp::Presence>(action->data()).status();

    Q_ASSERT(!m_account.isNull());

    Tp::PendingOperation* presenceRequest = m_account->setRequestedPresence(presence);

    connect(presenceRequest, SIGNAL(finished(Tp::PendingOperation*)),
            this, SLOT(updateToolTip()));
}

void AccountButton::updateToolTip()
{
    //check if the custom status message has been set
    if(m_account->currentPresence().statusMessage().isEmpty()) {
        setToolTip(QString("%1\n%2").arg(m_account->displayName())
                                    .arg(presenceDisplayString(m_account->currentPresence())));
    }
    else {
        setToolTip(QString("%1\n%2\n%3").arg(m_account->displayName())
                                        .arg(presenceDisplayString(m_account->currentPresence()))
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
        if (m_account->currentPresence().status() == qVariantValue<Tp::Presence>(a->data()).status()) {
            a->setChecked(true);
            updateToolTip();
            break;
        }
    }
}

/*  since there is no easy way to get this string by Tp::Presence,
    we need to loop through all the actions and return the right one.
    This will also get us i18n strings for free. */
QString AccountButton::presenceDisplayString(const Tp::Presence)
{
    foreach(QAction *a, actions()) {
        if (m_account->currentPresence().status() == qVariantValue<Tp::Presence>(a->data()).status()) {
            return a->text();
        }
    }

    return QString();
}