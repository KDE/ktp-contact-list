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

#include <TelepathyQt4/AccountManager>

class KPixmapSequenceOverlayPainter;
class GlobalPresence;
class PresenceModel;

class GlobalPresenceChooser : public KComboBox
{
    Q_OBJECT
public:
    explicit GlobalPresenceChooser(QWidget *parent = 0);
    void setAccountManager(const Tp::AccountManagerPtr &accountManager);

protected:
    virtual bool event(QEvent *event);

private slots:
    void onCurrentIndexChanged(int index);
    void onPresenceChanged(const Tp::Presence &presence);
    void onPresenceChanging(bool isChanging);

private:
    GlobalPresence *m_globalPresence;
    PresenceModel *m_model;
    KPixmapSequenceOverlayPainter *m_busyOverlay;
    Tp::AccountManagerPtr m_accountManager;
};

#endif // GLOBALPRESENCECHOOSER_H
