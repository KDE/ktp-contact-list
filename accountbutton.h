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
#include <QLineEdit>

#include <TelepathyQt4/Account>

class QAction;
class KPixmapSequenceOverlayPainter;

class AccountButton : public QToolButton
{
    Q_OBJECT
    
public:
    AccountButton(const Tp::AccountPtr &account, QWidget *parent = 0);
    
    QString accountId();
    
public Q_SLOTS:
    void setAccountStatus(QAction *action);
    void updateToolTip();
    void connectionChanged(Tp::ConnectionStatus status);
    void showBusyIndicator();
    void hideBusyIndicator();
    void preseneceChanged(Tp::Presence presence);
    
private:
    Tp::AccountPtr                  m_account;
    int                             m_statusIndex;
    KPixmapSequenceOverlayPainter  *m_busyOverlay;
};

#endif // TELEPATHY_ACCOUNTBUTTON_H
