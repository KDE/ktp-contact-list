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

#include "contactoverlays.moc"

// Qt includes

// KDE includes

#include <KLocale>
#include <KIconLoader>
#include <KDebug>

#include <TelepathyQt4/ContactCapabilities>

class TextChannelContactOverlay::Button : public ContactViewHoverButton
{
public:

    Button(QAbstractItemView* parentView, const KGuiItem& gui);
    virtual QSize sizeHint() const;

protected:

    KGuiItem gui;

    virtual QPixmap icon();
    virtual void updateToolTip();

};

TextChannelContactOverlay::Button::Button(QAbstractItemView* parentView, const KGuiItem& gui)
    : ContactViewHoverButton(parentView), gui(gui)
{
}

QSize TextChannelContactOverlay::Button::sizeHint() const
{
    return QSize(KIconLoader::SizeSmall, KIconLoader::SizeSmall);
}

QPixmap TextChannelContactOverlay::Button::icon()
{
    return KIconLoader::global()->loadIcon(gui.iconName(),
                                           KIconLoader::NoGroup,
                                           KIconLoader::SizeSmall);
}

void TextChannelContactOverlay::Button::updateToolTip()
{
    setToolTip(gui.toolTip());
}

// -------------------------------------------------------------------------

TextChannelContactOverlay::TextChannelContactOverlay(QObject* parent)
    : HoverButtonDelegateOverlay(parent),
      m_referenceModel(0)
{
    m_gui = KGuiItem(i18n("Start text channel"), "text-x-generic", 
                     i18n("Start text channel"), i18n("Whats this"));
}

TextChannelContactOverlay::Button *TextChannelContactOverlay::button() const
{
    return static_cast<Button*>(HoverButtonDelegateOverlay::button());
}

void TextChannelContactOverlay::setReferenceModel(const FakeContactsModel* model)
{
    m_referenceModel = model;
}

void TextChannelContactOverlay::setActive(bool active)
{
    HoverButtonDelegateOverlay::setActive(active);

    if (active)
    {
        connect(button(), SIGNAL(clicked(bool)),
                this, SLOT(slotClicked(bool)));
    }
    else
    {
        // button is deleted
    }
}

ContactViewHoverButton* TextChannelContactOverlay::createButton()
{
    return new Button(view(), m_gui);
}

void TextChannelContactOverlay::updateButton(const QModelIndex& index)
{
    const QRect rect = m_view->visualRect(index);
    const QSize size = button()->size();

    const int gap = 5;
    const int x   = rect.right() - gap - 96 - size.width();
    const int y   = rect.bottom() - gap - size.height();
    button()->move(QPoint(x, y));
}

void TextChannelContactOverlay::slotClicked(bool checked)
{
    Q_UNUSED(checked);
    QModelIndex index = button()->index();

    if (index.isValid())
    {
        emit activated(index);
    }
}

bool TextChannelContactOverlay::checkIndex(const QModelIndex& index) const
{
    if(index.data(ModelRoles::ContactCapabilities).value<Tp::ContactCapabilities>().textChats()) {
        return true;
    }
    
    return false;
}

// ------------------------------------------------------------------------

class AudioChannelContactOverlay::Button : public ContactViewHoverButton
{
public:
    
    Button(QAbstractItemView* parentView, const KGuiItem& gui);
    virtual QSize sizeHint() const;
    
protected:
    
    KGuiItem gui;
    
    virtual QPixmap icon();
    virtual void updateToolTip();
    
};

AudioChannelContactOverlay::Button::Button(QAbstractItemView* parentView, const KGuiItem& gui)
: ContactViewHoverButton(parentView), gui(gui)
{
}

QSize AudioChannelContactOverlay::Button::sizeHint() const
{
    return QSize(KIconLoader::SizeSmall, KIconLoader::SizeSmall);
}

QPixmap AudioChannelContactOverlay::Button::icon()
{
    return KIconLoader::global()->loadIcon(gui.iconName(),
                                           KIconLoader::NoGroup,
                                           KIconLoader::SizeSmall);
}

void AudioChannelContactOverlay::Button::updateToolTip()
{
    setToolTip(gui.toolTip());
}

// -------------------------------------------------------------------------

AudioChannelContactOverlay::AudioChannelContactOverlay(QObject* parent)
: HoverButtonDelegateOverlay(parent),
m_referenceModel(0)
{
    m_gui = KGuiItem(i18n("Start audio channel"), "voicecall", 
                     i18n("Start audio channel"), i18n("Whats this"));
                              
}

AudioChannelContactOverlay::Button *AudioChannelContactOverlay::button() const
{
    return static_cast<Button*>(HoverButtonDelegateOverlay::button());
}

void AudioChannelContactOverlay::setReferenceModel(const FakeContactsModel* model)
{
    m_referenceModel = model;
}

void AudioChannelContactOverlay::setActive(bool active)
{
    HoverButtonDelegateOverlay::setActive(active);
    
    if (active)
    {
        connect(button(), SIGNAL(clicked(bool)),
                this, SLOT(slotClicked(bool)));
    }
    else
    {
        // button is deleted
    }
}

ContactViewHoverButton* AudioChannelContactOverlay::createButton()
{
    return new Button(view(), m_gui);
}

void AudioChannelContactOverlay::updateButton(const QModelIndex& index)
{
    const QRect rect = m_view->visualRect(index);
    const QSize size = button()->size();
    
    const int gap = 5;
    const int x   = rect.right() - gap - 72 - size.width();
    const int y   = rect.bottom() - gap - size.height();
    button()->move(QPoint(x, y));
}

void AudioChannelContactOverlay::slotClicked(bool checked)
{
    Q_UNUSED(checked);
    QModelIndex index = button()->index();
    
    if (index.isValid())
    {
        //emit activated(index);
    }
}

bool AudioChannelContactOverlay::checkIndex(const QModelIndex& index) const
{
    if(index.data(ModelRoles::ContactCapabilities).value<Tp::ContactCapabilities>().streamedMediaAudioCalls()) {
        return true;
    }
    
    return false;
}

// ----------------------------------------------------------

class VideoChannelContactOverlay::Button : public ContactViewHoverButton
{
public:
    
    Button(QAbstractItemView* parentView, const KGuiItem& gui);
    virtual QSize sizeHint() const;
    
protected:
    
    KGuiItem gui;
    
    virtual QPixmap icon();
    virtual void updateToolTip();
    
};

VideoChannelContactOverlay::Button::Button(QAbstractItemView* parentView, const KGuiItem& gui)
: ContactViewHoverButton(parentView), gui(gui)
{
}

QSize VideoChannelContactOverlay::Button::sizeHint() const
{
    return QSize(KIconLoader::SizeSmall, KIconLoader::SizeSmall);
}

QPixmap VideoChannelContactOverlay::Button::icon()
{
    return KIconLoader::global()->loadIcon(gui.iconName(),
                                           KIconLoader::NoGroup,
                                           KIconLoader::SizeSmall);
}

void VideoChannelContactOverlay::Button::updateToolTip()
{
    setToolTip(gui.toolTip());
}

// -------------------------------------------------------------------------

VideoChannelContactOverlay::VideoChannelContactOverlay(QObject* parent)
: HoverButtonDelegateOverlay(parent),
m_referenceModel(0)
{
    m_gui = KGuiItem(i18n("Start video channel"), "camera-web", 
                     i18n("Start video channel"), i18n("Whats this"));          
}

VideoChannelContactOverlay::Button *VideoChannelContactOverlay::button() const
{
    return static_cast<Button*>(HoverButtonDelegateOverlay::button());
}

void VideoChannelContactOverlay::setReferenceModel(const FakeContactsModel* model)
{
    m_referenceModel = model;
}

void VideoChannelContactOverlay::setActive(bool active)
{
    HoverButtonDelegateOverlay::setActive(active);
    
    if (active)
    {
        connect(button(), SIGNAL(clicked(bool)),
                this, SLOT(slotClicked(bool)));
    }
    else
    {
        // button is deleted
    }
}

ContactViewHoverButton* VideoChannelContactOverlay::createButton()
{
    return new Button(view(), m_gui);
}

void VideoChannelContactOverlay::updateButton(const QModelIndex& index)
{
    const QRect rect = m_view->visualRect(index);
    const QSize size = button()->size();
    
    const int gap = 5;
    const int x   = rect.right() - gap - 50 - size.width();
    const int y   = rect.bottom() - gap - size.height();
    button()->move(QPoint(x, y));
}

void VideoChannelContactOverlay::slotClicked(bool checked)
{
    Q_UNUSED(checked);
    QModelIndex index = button()->index();
    
    if (index.isValid())
    {
        //emit activated(index);
    }
}

bool VideoChannelContactOverlay::checkIndex(const QModelIndex& index) const
{
    if(index.data(ModelRoles::ContactCapabilities).value<Tp::ContactCapabilities>().streamedMediaVideoCallsWithAudio()) {
        return true;
    }
    
    return false;
}

// ----------------------------------------------------------

class FileTransferContactOverlay::Button : public ContactViewHoverButton
{
public:
    
    Button(QAbstractItemView* parentView, const KGuiItem& gui);
    virtual QSize sizeHint() const;
    
protected:
    
    KGuiItem gui;
    
    virtual QPixmap icon();
    virtual void updateToolTip();
    
};

FileTransferContactOverlay::Button::Button(QAbstractItemView* parentView, const KGuiItem& gui)
: ContactViewHoverButton(parentView), gui(gui)
{
}

QSize FileTransferContactOverlay::Button::sizeHint() const
{
    return QSize(KIconLoader::SizeSmall, KIconLoader::SizeSmall);
}

QPixmap FileTransferContactOverlay::Button::icon()
{
    return KIconLoader::global()->loadIcon(gui.iconName(),
                                           KIconLoader::NoGroup,
                                           KIconLoader::SizeSmall);
}

void FileTransferContactOverlay::Button::updateToolTip()
{
    setToolTip(gui.toolTip());
}

// -------------------------------------------------------------------------

FileTransferContactOverlay::FileTransferContactOverlay(QObject* parent)
: HoverButtonDelegateOverlay(parent),
m_referenceModel(0)
{
    m_gui = KGuiItem(i18n("Send file"), "mail-attachment", 
                     i18n("Send file"), i18n("Whats this"));          
}

FileTransferContactOverlay::Button *FileTransferContactOverlay::button() const
{
    return static_cast<Button*>(HoverButtonDelegateOverlay::button());
}

void FileTransferContactOverlay::setReferenceModel(const FakeContactsModel* model)
{
    m_referenceModel = model;
}

void FileTransferContactOverlay::setActive(bool active)
{
    HoverButtonDelegateOverlay::setActive(active);
    
    if (active)
    {
        connect(button(), SIGNAL(clicked(bool)),
                this, SLOT(slotClicked(bool)));
    }
    else
    {
        // button is deleted
    }
}

ContactViewHoverButton* FileTransferContactOverlay::createButton()
{
    return new Button(view(), m_gui);
}

void FileTransferContactOverlay::updateButton(const QModelIndex& index)
{
    const QRect rect = m_view->visualRect(index);
    const QSize size = button()->size();
    
    const int gap = 5;
    const int x   = rect.right() - gap - 132 - size.width();
    const int y   = rect.bottom() - gap - size.height();
    button()->move(QPoint(x, y));
}

void FileTransferContactOverlay::slotClicked(bool checked)
{
    Q_UNUSED(checked);
    QModelIndex index = button()->index();
    
    if (index.isValid())
    {
        //emit activated(index);
    }
}

bool FileTransferContactOverlay::checkIndex(const QModelIndex& index) const
{
    if(index.data(ModelRoles::ContactCapabilities).value<Tp::ContactCapabilities>().fileTransfers()) {
        return true;
    }
    
    return false;
}