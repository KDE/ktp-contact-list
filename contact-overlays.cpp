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

#include "contact-overlays.h"

#include <KLocale>
#include <KIconLoader>
#include <KDebug>

#include <KTp/Models/accounts-model.h>
#include <KTp/Models/contact-model-item.h>

class GuiItemContactViewHoverButton : public ContactViewHoverButton
{
public:

    GuiItemContactViewHoverButton(QAbstractItemView *parentView, const KGuiItem &gui);
    virtual QSize sizeHint() const;

protected:

    virtual QPixmap icon();
    virtual void updateToolTip();

private:

    KGuiItem m_guiItem;
};

GuiItemContactViewHoverButton::GuiItemContactViewHoverButton(QAbstractItemView *parentView, const KGuiItem &gui)
    : ContactViewHoverButton(parentView), m_guiItem(gui)
{
}

QSize GuiItemContactViewHoverButton::sizeHint() const
{
    return QSize(KIconLoader::SizeSmall, KIconLoader::SizeSmall);
}

QPixmap GuiItemContactViewHoverButton::icon()
{
    return KIconLoader::global()->loadIcon(m_guiItem.iconName(),
                                           KIconLoader::NoGroup,
                                           KIconLoader::SizeSmall);
}

void GuiItemContactViewHoverButton::updateToolTip()
{
    setToolTip(m_guiItem.toolTip());
}

// -------------------------------------------------------------------------

StartChannelContactOverlay::StartChannelContactOverlay(QObject *parent, const KGuiItem &gui,
                                                       int capabilityRole, int xpos)
    : ContactDelegateOverlay(parent),
      m_gui(gui),
      m_capabilityRole(capabilityRole),
      m_xpos(xpos)
{
}

void StartChannelContactOverlay::setActive(bool active)
{
    ContactDelegateOverlay::setActive(active);

    if (active) {
        connect(button(), SIGNAL(clicked(bool)),
                this, SLOT(slotClicked(bool)));
    } else {
        // button is deleted
    }
}

ContactViewHoverButton* StartChannelContactOverlay::createButton()
{
    return new GuiItemContactViewHoverButton(view(), m_gui);
}

void StartChannelContactOverlay::updateButton(const QModelIndex &index)
{
    const QRect rect = m_view->visualRect(index);
    const QSize size = button()->size();

    const int gap = 2;
    const int x   = rect.left() + m_xpos; // rect.right() - gap - 96 - size.width();
    const int y   = rect.bottom() - gap - size.height();
    button()->move(QPoint(x, y));
}

void StartChannelContactOverlay::slotClicked(bool checked)
{
    Q_UNUSED(checked);
    QModelIndex index = button()->index();

    if (index.isValid()) {
        ContactModelItem* contactItem = index.data(AccountsModel::ItemRole).value<ContactModelItem*>();
        if (contactItem) {
            emit activated(contactItem);
        }
    }
}

bool StartChannelContactOverlay::checkIndex(const QModelIndex& index) const
{
    return index.data(m_capabilityRole).toBool() && index.data(AccountsModel::ItemRole).userType() == qMetaTypeId<ContactModelItem*>();
}

// ------------------------------------------------------------------------

TextChannelContactOverlay::TextChannelContactOverlay(QObject *parent)
    : StartChannelContactOverlay(
        parent,
        KGuiItem(i18n("Start Chat"), "text-x-generic",
                 i18n("Start Chat"), i18n("Start a text chat")),
        AccountsModel::TextChatCapabilityRole,
        40)
{
}

// ------------------------------------------------------------------------

AudioChannelContactOverlay::AudioChannelContactOverlay(QObject *parent)
    : StartChannelContactOverlay(
        parent,
        KGuiItem(i18n("Start Audio Call"), "audio-headset",
                 i18n("Start Audio Call"), i18n("Start an audio call")),
        AccountsModel::AudioCallCapabilityRole,
        64)

{
}

// -------------------------------------------------------------------------

VideoChannelContactOverlay::VideoChannelContactOverlay(QObject *parent)
    : StartChannelContactOverlay(
        parent,
        KGuiItem(i18n("Start Video Call"), "camera-web",
                 i18n("Start Video Call"), i18n("Start a video call")),
        AccountsModel::VideoCallCapabilityRole,
        88)
{
}

// -------------------------------------------------------------------------

FileTransferContactOverlay::FileTransferContactOverlay(QObject *parent)
    : StartChannelContactOverlay(
        parent,
        KGuiItem(i18n("Send File..."), "mail-attachment",
                 i18n("Send File..."), i18n("Send a file")),
        AccountsModel::FileTransferCapabilityRole,
        128)
{
}

// -------------------------------------------------------------------------

DesktopSharingContactOverlay::DesktopSharingContactOverlay(QObject *parent)
    : StartChannelContactOverlay(
        parent,
        KGuiItem(i18n("Share my desktop"), "krfb",
                 i18n("Share my desktop"), i18n("Share desktop using RFB")),
        AccountsModel::DesktopSharingCapabilityRole,
        152)
{
}


#include "contact-overlays.moc"
