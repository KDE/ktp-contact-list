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

#ifndef TELEPATHY_ACCOUNTBUTTON_H
#define TELEPATHY_ACCOUNTBUTTON_H

#include <QToolButton>

#include <TelepathyQt/Types>
#include <TelepathyQt/Presence>

class QAction;
class KPixmapSequenceOverlayPainter;
class KLineEdit;

class AccountButton : public QToolButton
{
    Q_OBJECT

public:
    explicit AccountButton(const Tp::AccountPtr &account, QWidget *parent = 0);

    ///Returns the unique account ID
    QString accountId();

    ///Returns the action (menu item) string for displaying elsewhere on the screen
    QString presenceDisplayString(const Tp::Presence &presence);

public Q_SLOTS:
    ///Sets the account status contained in action (connects to triggered(QAction*) signal)
    void setAccountStatus(QAction *action);

    ///Updates the tooltip with the latest selected status
    void updateToolTip();

    ///Called when the connection status changes
    void connectionChanged(const Tp::ConnectionStatus &status);

    ///Shows the animated busy icon over the button
    void showBusyIndicator();

    ///Hides the animated busy icon over the button
    void hideBusyIndicator();

    ///Called when the account presence changes
    void presenceChanged(const Tp::Presence &presence);

    ///Sets the custom presence message
    void setCustomPresenceMessage(const QString &message);
    
    ///Update account item icon when profile type changes
    void updateIcon(const QString &iconPath);

private:

    QAction *actionForPresence(const Tp::Presence &presence) const;
    void resetMenuFormatting();

    ///Holds the account it controls
    Tp::AccountPtr                  m_account;

    ///Contains the custom presence string
    QString                         m_customPresenceMessage;

    ///The busy icon which is painted when connecting
    KPixmapSequenceOverlayPainter  *m_busyOverlay;

    KLineEdit *m_presenceMessageWidget;

    QAction *m_offlineAction;
};

#endif // TELEPATHY_ACCOUNTBUTTON_H
