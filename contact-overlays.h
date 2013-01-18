/*
 *  Contact overlay buttons
 *
 *  Copyright (C) 2009 Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *  Copyright (C) 2011 Martin Klapetek <martin dot klapetek at gmail dot com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef CONTACTOVERLAYS_H
#define CONTACTOVERLAYS_H

#include <KGuiItem>

#include "contact-delegate-overlay.h"
#include "contact-view-hover-button.h"

#include <TelepathyQt/Types>

class StartChannelContactOverlay : public ContactDelegateOverlay
{
    Q_OBJECT

public:
    StartChannelContactOverlay(QObject *parent, const KGuiItem &gui,
                               int capabilityRole, int xpos);

public Q_SLOTS:
    virtual void setActive(bool active);

Q_SIGNALS:
    void activated(const Tp::AccountPtr &account, const Tp::ContactPtr &contact);

protected:

    virtual ContactViewHoverButton* createButton();
    virtual void updateButton(const QModelIndex& index);
    virtual bool checkIndex(const QModelIndex& index) const;

protected Q_SLOTS:

    void slotClicked(bool checked);

private:

    KGuiItem m_gui;
    int m_capabilityRole;
    int m_xpos;
};

// ---------------------------------------------------------------------

class TextChannelContactOverlay : public StartChannelContactOverlay
{
    Q_OBJECT

public:
    TextChannelContactOverlay(QObject *parent);
};

// ---------------------------------------------------------------------

class AudioChannelContactOverlay : public StartChannelContactOverlay
{
    Q_OBJECT

public:
    AudioChannelContactOverlay(QObject *parent);
};

// ---------------------------------------------------------------------

class VideoChannelContactOverlay : public StartChannelContactOverlay
{
    Q_OBJECT

public:
    VideoChannelContactOverlay(QObject *parent);
};

// ---------------------------------------------------------------------

class FileTransferContactOverlay : public StartChannelContactOverlay
{
    Q_OBJECT

public:
    FileTransferContactOverlay(QObject *parent);
};

// ---------------------------------------------------------------------

class DesktopSharingContactOverlay : public StartChannelContactOverlay
{
    Q_OBJECT

public:
    DesktopSharingContactOverlay(QObject *parent);
};

// ---------------------------------------------------------------------

class LogViewerOverlay: public StartChannelContactOverlay
{
    Q_OBJECT

public:
    LogViewerOverlay(QObject *parent);
};

#endif // VERSIONSOVERLAYS_H
