/*
    Qt item view for images - delegate additions

    Copyright (C) 2009 Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
    Copyright (C) 2011 Martin Klapetek <martin dot klapetek at gmail dot com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "contact-delegate-overlay.h"

#include <QEvent>
#include <QTimer>
#include <QMouseEvent>

#include <KDebug>

#include "contact-view-hover-button.h"

ContactDelegateOverlay::ContactDelegateOverlay(QObject *parent)
    : QObject(parent),
      m_view(0),
      m_delegate(0),
      m_mouseButtonPressedOnWidget(false)
{
}

ContactDelegateOverlay::~ContactDelegateOverlay()
{
}

void ContactDelegateOverlay::setActive(bool active)
{
    if (active) {
        if (!m_button.isNull()) {
            m_button.data()->deleteLater();
        }

        m_button = createButton();

        m_button.data()->setFocusPolicy(Qt::NoFocus);
        m_button.data()->hide(); // hide per default
        m_button.data()->initIcon();

        m_view->viewport()->installEventFilter(this);

        if (view()->model()) {
            connect(m_view->model(), SIGNAL(rowsRemoved(QModelIndex,int,int)),
                    this, SLOT(slotHideButton()));

            connect(m_view->model(), SIGNAL(layoutChanged()),
                    this, SLOT(slotHideButton()));

            connect(m_view->model(), SIGNAL(modelReset()),
                    this, SLOT(slotReset()));
        }

        connect(m_view, SIGNAL(entered(QModelIndex)),
                this, SLOT(slotEntered(QModelIndex)));

        connect(m_view, SIGNAL(viewportEntered()),
                this, SLOT(slotHideButton()));
    } else {
        m_button.data()->deleteLater();

        if (m_view) {
            m_view->viewport()->removeEventFilter(this);

            if (view()->model()) {
                disconnect(m_view->model(), 0, this, 0);
            }

            disconnect(m_view, SIGNAL(entered(QModelIndex)),
                       this, SLOT(slotEntered(QModelIndex)));

            disconnect(m_view, SIGNAL(viewportEntered()),
                       this, SLOT(slotHideButton()));
        }
    }
}

void ContactDelegateOverlay::visualChange()
{
    if (!m_button.isNull() && m_button.data()->isVisible()) {
        updateButton(button()->index());
    }
}

void ContactDelegateOverlay::setView(QAbstractItemView *view)
{
    if (m_view) {
        disconnect(this, SIGNAL(update(QModelIndex)),
                   m_view, SLOT(update(QModelIndex)));
    }

    m_view = view;

    if (m_view) {
        connect(this, SIGNAL(update(QModelIndex)),
                m_view, SLOT(update(QModelIndex)));
    }
}

QAbstractItemView* ContactDelegateOverlay::view() const
{
    return m_view;
}

void ContactDelegateOverlay::setDelegate(QAbstractItemDelegate *delegate)
{
    m_delegate = delegate;
}

QAbstractItemDelegate* ContactDelegateOverlay::delegate() const
{
    return m_delegate;
}

void ContactDelegateOverlay::slotHideButton()
{
    if (!m_button.isNull()) {
        m_button.data()->hide();
    }
}

void ContactDelegateOverlay::slotReset()
{
    slotHideButton();
    button()->reset();
}

void ContactDelegateOverlay::slotEntered(const QModelIndex& index)
{
    slotHideButton();

    if (index.isValid() && checkIndex(index)) {
        m_button.data()->setIndex(index);
        updateButton(index);
        QTimer::singleShot(0, m_button.data(), SLOT(show()));
        emit overlayActivated(index);
    }
}

bool ContactDelegateOverlay::checkIndex(const QModelIndex& index) const
{
    Q_UNUSED(index);
    return true;
}

bool ContactDelegateOverlay::eventFilter(QObject *obj, QEvent *event)
{
    if (!m_button.isNull() && obj == m_button.data()) {
        switch (event->type()) {
            case QEvent::MouseButtonPress:
                if (static_cast<QMouseEvent*>(event)->buttons() & Qt::LeftButton) {
                    m_mouseButtonPressedOnWidget = true;
                }
                break;
            case QEvent::MouseButtonRelease:
                m_mouseButtonPressedOnWidget = false;
                break;
            default:
                break;
        }
    } else {   // events on view's viewport
        switch (event->type()) {
            case QEvent::Leave:
                slotHideButton();
                emit overlayHidden();
                break;
            case QEvent::MouseMove:
                if (m_mouseButtonPressedOnWidget) {
                    // Don't forward mouse move events to the viewport,
                    // otherwise a rubberband selection will be shown when
                    // clicking on the selection toggle and moving the mouse
                    // above the viewport.
                    return true;
                }
                break;
            case QEvent::MouseButtonRelease:
                m_mouseButtonPressedOnWidget = false;
                break;
            default:
                break;
        }
    }

    return false;
}

ContactViewHoverButton* ContactDelegateOverlay::button() const
{
    return m_button.data();
}

// -----------------------------

ContactDelegateOverlayContainer::~ContactDelegateOverlayContainer()
{
}

void ContactDelegateOverlayContainer::installOverlay(ContactDelegateOverlay *overlay)
{
    if (!overlay->acceptsDelegate(asDelegate())) {
        kError() << "Cannot accept delegate" << asDelegate() << "for installing" << overlay;
        return;
    }

    overlay->setDelegate(asDelegate());
    m_overlays << overlay;
}

void ContactDelegateOverlayContainer::removeOverlay(ContactDelegateOverlay *overlay)
{
    overlay->setActive(false);
    overlay->setDelegate(0);
    m_overlays.removeAll(overlay);
    QObject::disconnect(overlay, 0, asDelegate(), 0);
}

void ContactDelegateOverlayContainer::setAllOverlaysActive(bool active)
{
    foreach (ContactDelegateOverlay *overlay, m_overlays) {
        overlay->setActive(active);
    }
}

void ContactDelegateOverlayContainer::setViewOnAllOverlays(QAbstractItemView *view)
{
    foreach (ContactDelegateOverlay *overlay, m_overlays) {
        overlay->setView(view);
    }
}

void ContactDelegateOverlayContainer::removeAllOverlays()
{
    foreach (ContactDelegateOverlay *overlay, m_overlays) {
        overlay->setActive(false);
        overlay->setDelegate(0);
        overlay->setView(0);
    }
    m_overlays.clear();
}

void ContactDelegateOverlayContainer::overlayDestroyed(QObject *o)
{
    ContactDelegateOverlay *overlay = qobject_cast<ContactDelegateOverlay*>(o);
    if (overlay) {
        removeOverlay(overlay);
    }
}

#include "contact-delegate-overlay.moc"
