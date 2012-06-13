/*
 * Global Presence - A Drop down menu for selecting presence
 *
 * Copyright (C) 2011 David Edmundson <kde@davidedmundson.co.uk>
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

#ifndef GLOBALPRESENCECHOOSER_H
#define GLOBALPRESENCECHOOSER_H

#include <KComboBox>

#include <TelepathyQt/AccountManager>
#include <KTp/presence.h>

class QMenu;
class QPushButton;
class KPixmapSequenceOverlayPainter;
class PresenceModel;
class PresenceModelExtended;

namespace KTp {
class GlobalPresence;
}

class GlobalPresenceChooser : public KComboBox
{
    Q_OBJECT
public:
    explicit GlobalPresenceChooser(QWidget *parent = 0);
    void setAccountManager(const Tp::AccountManagerPtr &accountManager);

    void repositionOverlays();

protected:
    virtual bool event(QEvent *event);

private Q_SLOTS:
    void onCurrentIndexChanged(int index);
    void onPresenceChanged(const KTp::Presence &presence);
    void onConnectionStatusChanged(Tp::ConnectionStatus connectionStatus);
    void onChangePresenceMessageClicked();
    void onConfirmPresenceMessageClicked();

private:
    KTp::GlobalPresence *m_globalPresence;
    PresenceModel *m_model;
    PresenceModelExtended *m_modelExtended;

    KPixmapSequenceOverlayPainter *m_busyOverlay;
    Tp::AccountManagerPtr m_accountManager;
    QPushButton *m_changePresenceMessageButton;
    QWeakPointer<QMenu> m_lineEditContextMenu;
};

#endif // GLOBALPRESENCECHOOSER_H
