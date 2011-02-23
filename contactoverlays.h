/*
 *  Contact overlay buttons
 *  Copyright (C) 2009  Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *  Copyright (C) 2011  Martin Klapetek <martin dot klapetek at gmail dot com>
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

// Qt includes

// KDE includes

#include <KGuiItem>

// Local includes

#include "contactdelegateoverlay.h"
#include "contactviewhoverbutton.h"
#include "fakecontactsmodel.h"

class TextChannelContactOverlay : public HoverButtonDelegateOverlay
{
    Q_OBJECT

public:

    TextChannelContactOverlay(QObject* parent);
    virtual void setActive(bool active);

    void setReferenceModel(const FakeContactsModel* model);

Q_SIGNALS:

    void activated(const QModelIndex index);

protected:

    virtual ContactViewHoverButton* createButton();
    virtual void updateButton(const QModelIndex& index);
    virtual bool checkIndex(const QModelIndex& index) const;

protected Q_SLOTS:

    void slotClicked(bool checked);

protected:

    KGuiItem                 m_gui;
    const FakeContactsModel* m_referenceModel;

    class Button;
    Button *button() const;
};

// ---------------------------------------------------------------------

class AudioChannelContactOverlay : public HoverButtonDelegateOverlay
{
    Q_OBJECT
    
public:
    
    AudioChannelContactOverlay(QObject* parent);
    virtual void setActive(bool active);
    
    void setReferenceModel(const FakeContactsModel* model);
    
Q_SIGNALS:
    
    //void activated(const ImageInfo& info);
    
protected:
    
    virtual ContactViewHoverButton* createButton();
    virtual void updateButton(const QModelIndex& index);
    virtual bool checkIndex(const QModelIndex& index) const;
    
protected Q_SLOTS:
    
    void slotClicked(bool checked);
    
protected:
    
    KGuiItem                 m_gui;
    const FakeContactsModel* m_referenceModel;
    
    class Button;
    Button *button() const;
};

// ---------------------------------------------------------------------

class VideoChannelContactOverlay : public HoverButtonDelegateOverlay
{
    Q_OBJECT
    
public:
    
    VideoChannelContactOverlay(QObject* parent);
    virtual void setActive(bool active);
    
    void setReferenceModel(const FakeContactsModel* model);
    
Q_SIGNALS:
    
    //void activated(const ImageInfo& info);
    
protected:
    
    virtual ContactViewHoverButton* createButton();
    virtual void updateButton(const QModelIndex& index);
    virtual bool checkIndex(const QModelIndex& index) const;
    
protected Q_SLOTS:
    
    void slotClicked(bool checked);
    
protected:
    
    KGuiItem                 m_gui;
    const FakeContactsModel* m_referenceModel;
    
    class Button;
    Button *button() const;
};

// ---------------------------------------------------------------------

class FileTransferContactOverlay : public HoverButtonDelegateOverlay
{
    Q_OBJECT
    
public:
    
    FileTransferContactOverlay(QObject* parent);
    virtual void setActive(bool active);
    
    void setReferenceModel(const FakeContactsModel* model);
    
Q_SIGNALS:
    
    //void activated(const ImageInfo& info);
    
protected:
    
    virtual ContactViewHoverButton* createButton();
    virtual void updateButton(const QModelIndex& index);
    virtual bool checkIndex(const QModelIndex& index) const;
    
protected Q_SLOTS:
    
    void slotClicked(bool checked);
    
protected:
    
    KGuiItem                 m_gui;
    const FakeContactsModel* m_referenceModel;
    
    class Button;
    Button *button() const;
};

#endif // VERSIONSOVERLAYS_H
