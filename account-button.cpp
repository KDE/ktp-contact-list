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

#include "account-button.h"

#include <QtGui/QPainter>
#include <QtGui/QPixmap>

#include <KAction>
#include <KIcon>
#include <KLocale>
#include <KMenu>
#include <KPixmapSequenceOverlayPainter>
#include <KPixmapSequence>
#include <KIconLoader>
#include <KLineEdit>

#include <TelepathyQt/Account>
#include <TelepathyQt/PendingOperation>

AccountButton::AccountButton(const Tp::AccountPtr &account, QWidget* parent)
  : QToolButton(parent),
    m_busyOverlay(0),
    m_offlineAction(0)
{
    m_account = account;

    m_busyOverlay = new KPixmapSequenceOverlayPainter(this);
    m_busyOverlay->setWidget(this);
    m_busyOverlay->setSequence(KPixmapSequence(QString("process-working")));

    QString iconPath = m_account->iconName();

    setIcon(KIcon(iconPath));
    if (!m_account->isValid()) {
        //we paint a warning symbol in the right-bottom corner
        QPixmap errorPixmap = KIconLoader::global()->loadIcon("dialog-error", KIconLoader::NoGroup, 16);
        QPixmap pixmap = icon().pixmap(32, 32);
        QPainter painter(&pixmap);
        painter.drawPixmap(15, 15, 16, 16, errorPixmap);

        setIcon(KIcon(pixmap));
    }

    setMaximumWidth(24);

    setAutoRaise(true);
    setPopupMode(QToolButton::InstantPopup);
    setArrowType(Qt::NoArrow);

    QActionGroup *presenceActions = new QActionGroup(this);
    presenceActions->setExclusive(true);

    KAction *onlineAction =     new KAction(KIcon("user-online"), i18nc("@action:inmenu This is an IM user status", "Available"), this);
    KAction *awayAction =       new KAction(KIcon("user-away"), i18nc("@action:inmenu This is an IM user status", "Away"), this);
    KAction *brbAction =        new KAction(KIcon("user-away"), i18nc("@action:inmenu This is an IM user status", "Be right back"), this);
    KAction *busyAction =       new KAction(KIcon("user-busy"), i18nc("@action:inmenu This is an IM user status", "Busy"), this);
    KAction *dndAction =        new KAction(KIcon("user-busy"), i18nc("@action:inmenu This is an IM user status", "Do not disturb"), this);
    KAction *xaAction =         new KAction(KIcon("user-away-extended"), i18nc("@action:inmenu This is an IM user status", "Extended Away"), this);
    KAction *invisibleAction =  new KAction(KIcon("user-invisible"), i18nc("@action:inmenu This is an IM user status", "Invisible"), this);
    m_offlineAction =    new KAction(KIcon("user-offline"), i18nc("@action:inmenu This is an IM user status", "Offline"), this);

    m_presenceMessageWidget = new KLineEdit(this);
    m_presenceMessageWidget->setClearButtonShown(true);
    m_presenceMessageWidget->setClickMessage(i18nc("@action:inmenu This is the IM presence message" ,"Set message..."));
    m_presenceMessageWidget->setTrapReturnKey(true);

    connect(m_presenceMessageWidget, SIGNAL(returnPressed(QString)),
            this, SLOT(setCustomPresenceMessage(QString)));

    //this makes sure the klineedit loses focus once enter was pressed
    connect(m_presenceMessageWidget, SIGNAL(returnPressed(QString)),
            this, SLOT(setFocus()));

    QWidgetAction *presenceMessageAction = new QWidgetAction(this);
    presenceMessageAction->setDefaultWidget(m_presenceMessageWidget);

    //let's set the presences as data so we can easily just use the Tp::Presence when the action has been triggered
    onlineAction->setData(qVariantFromValue(Tp::Presence::available()));
    awayAction->setData(qVariantFromValue(Tp::Presence::away()));
    brbAction->setData(qVariantFromValue(Tp::Presence::brb()));
    busyAction->setData(qVariantFromValue(Tp::Presence::busy()));
    dndAction->setData(qVariantFromValue(Tp::Presence(
        Tp::ConnectionPresenceTypeBusy,
        QLatin1String("dnd"),
        QLatin1String(""))));
    xaAction->setData(qVariantFromValue(Tp::Presence::xa()));
    invisibleAction->setData(qVariantFromValue(Tp::Presence::hidden()));
    m_offlineAction->setData(qVariantFromValue(Tp::Presence::offline()));

    presenceActions->addAction(onlineAction);
    presenceActions->addAction(awayAction);
    presenceActions->addAction(brbAction);
    presenceActions->addAction(busyAction);
    presenceActions->addAction(dndAction);
    presenceActions->addAction(xaAction);
    presenceActions->addAction(invisibleAction);
    presenceActions->addAction(m_offlineAction);
    presenceActions->addAction(presenceMessageAction);

    KMenu *presenceMenu = new KMenu(this);
    presenceMenu->setMinimumWidth(180);
    presenceMenu->addActions(presenceActions->actions());

    QFont titleFont = KGlobalSettings::menuFont();
    QFontMetrics titleFontMetrics(titleFont);
    QString accountName = titleFontMetrics.elidedText(m_account->displayName(), Qt::ElideMiddle, presenceMenu->width());

    presenceMenu->addTitle(KIcon(), accountName, presenceMenu->actions().first());

    setMenu(presenceMenu);

    //set the current status as checked and paint presence overlay
    presenceChanged(m_account->currentPresence());

    connect(this, SIGNAL(triggered(QAction*)),
            this, SLOT(setAccountStatus(QAction*)));

    connect(m_account.data(),SIGNAL(connectionStatusChanged(Tp::ConnectionStatus)),
            this, SLOT(connectionChanged(Tp::ConnectionStatus)));

    connect(m_account.data(), SIGNAL(currentPresenceChanged(Tp::Presence)),
            this, SLOT(presenceChanged(Tp::Presence)));
    
    connect(m_account.data(), SIGNAL(iconNameChanged(QString)), 
            this, SLOT(updateIcon(QString)));

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
    presence.statusMessage = m_customPresenceMessage;

    Q_ASSERT(!m_account.isNull());

    Tp::PendingOperation* presenceRequest = m_account->setRequestedPresence(presence);

    connect(presenceRequest, SIGNAL(finished(Tp::PendingOperation*)),
            this, SLOT(updateToolTip()));
}

void AccountButton::updateToolTip()
{
    //check if the custom status message has been set
    if (m_account->currentPresence().statusMessage().isEmpty()) {
        setToolTip(QString("%1\n%2").arg(m_account->displayName())
                                    .arg(presenceDisplayString(m_account->currentPresence())));
    } else {
        setToolTip(QString("%1\n%2\n%3").arg(m_account->displayName())
                                        .arg(presenceDisplayString(m_account->currentPresence()))
                                        .arg(m_account->currentPresence().statusMessage()));
    }
}

void AccountButton::connectionChanged(const Tp::ConnectionStatus &status)
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

void AccountButton::presenceChanged(const Tp::Presence &presence)
{
    if (!presence.isValid()) {
        return;
    }

    resetMenuFormatting();
    QFont presenceFont = KGlobalSettings::generalFont();
    presenceFont.setBold(true);
    presenceFont.setItalic(true);

    QAction *action = actionForPresence(presence);
    if (!action) {
        action = m_offlineAction;
    }

    action->setFont(presenceFont);
    updateToolTip();

    QPixmap pixmap = icon().pixmap(32, 32);
    QPainter painter(&pixmap);
    KIcon(action->icon()).paint(&painter, 15, 15, 16, 16);

    setIcon(KIcon(pixmap));
}

QAction *AccountButton::actionForPresence(const Tp::Presence &presence) const
{
    QAction *match = 0;

    foreach (QAction *a, menu()->actions()) {
        Tp::Presence actionPresence = qVariantValue<Tp::Presence>(a->data());
        if (presence.status() == actionPresence.status()) {
            // if a matching status is found, return it immediately
            return a;
        } else if (!match && presence.type() == actionPresence.type()) {
            // if no matching status is found, the first action with matching
            // type will be returned, so save it for later
            match = a;
        }
    }

    // return the best match, which could be a null pointer
    return match;
}

/*  since there is no easy way to get this string by Tp::Presence,
    we need to loop through all the actions and return the right one.
    This will also get us i18n strings for free. */
QString AccountButton::presenceDisplayString(const Tp::Presence &presence)
{
    QAction *action = actionForPresence(presence);
    if (action) {
        return KGlobal::locale()->removeAcceleratorMarker(action->text());
    } else {
        return QString();
    }
}

void AccountButton::setCustomPresenceMessage(const QString& message)
{
    m_customPresenceMessage = message;

    Tp::SimplePresence presence;
    presence.type = m_account->currentPresence().type();
    presence.status = m_account->currentPresence().status();
    presence.statusMessage = m_customPresenceMessage;

    Q_ASSERT(!m_account.isNull());

    Tp::PendingOperation* presenceRequest = m_account->setRequestedPresence(presence);

    connect(presenceRequest, SIGNAL(finished(Tp::PendingOperation*)),
            this, SLOT(updateToolTip()));

    m_presenceMessageWidget->setText(message);
}

void AccountButton::resetMenuFormatting()
{
    QFont presenceFont = KGlobalSettings::generalFont();
    presenceFont.setBold(false);
    presenceFont.setItalic(false);

    foreach (QAction *a, menu()->actions()) {
        a->setFont(presenceFont);
    }
}

void AccountButton::updateIcon(const QString &iconPath)
{
    setIcon(KIcon(iconPath));
}

#include "account-button.moc"
