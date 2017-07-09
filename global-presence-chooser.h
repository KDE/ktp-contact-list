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

#include <KTp/global-presence.h>
#include <KTp/presence.h>

class QMenu;
class QPushButton;
class KPixmapSequenceOverlayPainter;
class PresenceModelExtended;

namespace KTp {
    class GlobalPresence;
    class PresenceModel;
}

extern const QString KDED_STATUS_MESSAGE_PARSER_WHATSTHIS;

class GlobalPresenceChooser : public KComboBox
{
    Q_OBJECT
public:
    explicit GlobalPresenceChooser(QWidget *parent = 0);

    void repositionOverlays();

    KTp::GlobalPresence *globalPresence() {return m_globalPresence;};

protected:
    virtual bool event(QEvent *event);
    virtual void setEditable(bool editable); /** Hides overlay and calls ancestor's method. */

private Q_SLOTS:
    void onUserActivatedComboChange(int index);
    void onAllComboChanges(int index);
    void onPresenceChanged(const KTp::Presence &presence);
    void onConnectionStatusChanged(KTp::GlobalPresence::ConnectionStatus connectionStatus);
    void onChangePresenceMessageClicked();
    void onConfirmPresenceMessageClicked();

private:
    KTp::GlobalPresence *m_globalPresence;
    KTp::PresenceModel *m_model;
    PresenceModelExtended *m_modelExtended;

    KPixmapSequenceOverlayPainter *m_busyOverlay;
    QPushButton *m_changePresenceMessageButton;
    QPointer<QMenu> m_lineEditContextMenu;
};

#endif // GLOBALPRESENCECHOOSER_H
