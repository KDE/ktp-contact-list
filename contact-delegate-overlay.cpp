/*
*  Qt item view for images - delegate additions
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

#include "contact-delegate-overlay.h"

#include <QEvent>
#include <QTimer>
#include <QMouseEvent>

#include <KDebug>

#include "contact-view-hover-button.h"

ContactDelegateOverlay::ContactDelegateOverlay(QObject* parent)
    : QObject(parent), m_view(0), m_delegate(0)
{
}

ContactDelegateOverlay::~ContactDelegateOverlay()
{
}

void ContactDelegateOverlay::setActive(bool)
{
}

void ContactDelegateOverlay::visualChange()
{
    kDebug();
}

void ContactDelegateOverlay::mouseMoved(QMouseEvent*, const QRect&, const QModelIndex&)
{
}

void ContactDelegateOverlay::paint(QPainter*, const QStyleOptionViewItem&, const QModelIndex&)
{
}

void ContactDelegateOverlay::setView(QAbstractItemView* view)
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

void ContactDelegateOverlay::setDelegate(QAbstractItemDelegate* delegate)
{
//     if (m_delegate) {
//         disconnect(m_delegate, SIGNAL(visualChange()),
//                    this, SLOT(visualChange()));
//     }

    m_delegate = delegate;

//     if (m_delegate) {
//         connect(m_delegate, SIGNAL(visualChange()),
//                 this, SLOT(visualChange()));
//     }
}

QAbstractItemDelegate* ContactDelegateOverlay::delegate() const
{
    return m_delegate;
}

// -----------------------------

AbstractWidgetDelegateOverlay::AbstractWidgetDelegateOverlay(QObject* parent)
    : ContactDelegateOverlay(parent),
      m_mouseButtonPressedOnWidget(false)
{
}

AbstractWidgetDelegateOverlay::~AbstractWidgetDelegateOverlay()
{

}

void AbstractWidgetDelegateOverlay::setActive(bool active)
{
    if (active) {
        if (!m_widget.isNull()) {
            m_widget.data()->deleteLater();
        }

        m_widget = createWidget();

        m_widget.data()->setFocusPolicy(Qt::NoFocus);
        m_widget.data()->hide(); // hide per default

        m_view->viewport()->installEventFilter(this);
        m_widget.data()->installEventFilter(this);

        if (view()->model()) {
            connect(m_view->model(), SIGNAL(rowsRemoved(QModelIndex, int, int)),
                    this, SLOT(slotRowsRemoved(QModelIndex, int, int)));

            connect(m_view->model(), SIGNAL(layoutChanged()),
                    this, SLOT(slotLayoutChanged()));

            connect(m_view->model(), SIGNAL(modelReset()),
                    this, SLOT(slotReset()));
        }

        connect(m_view, SIGNAL(entered(QModelIndex)),
                this, SLOT(slotEntered(QModelIndex)));

        connect(m_view, SIGNAL(viewportEntered()),
                this, SLOT(slotViewportEntered()));
    } else {
        m_widget.data()->deleteLater();

        if (m_view) {
            m_view->viewport()->removeEventFilter(this);

            if (view()->model()) {
                disconnect(m_view->model(), 0, this, 0);
            }

            disconnect(m_view, SIGNAL(entered(QModelIndex)),
                       this, SLOT(slotEntered(QModelIndex)));

            disconnect(m_view, SIGNAL(viewportEntered()),
                       this, SLOT(slotViewportEntered()));
        }
    }
}

void AbstractWidgetDelegateOverlay::hide()
{
    if (!m_widget.isNull()) {
        m_widget.data()->hide();
    }
}

QWidget* AbstractWidgetDelegateOverlay::parentWidget() const
{
    return m_view->viewport();
}

void AbstractWidgetDelegateOverlay::slotReset()
{
    hide();
}

void AbstractWidgetDelegateOverlay::slotEntered(const QModelIndex& index)
{
    hide();

    if (index.isValid() && checkIndex(index)) {
//         QTimer::singleShot(500, m_widget, SLOT(show()));
        m_widget.data()->show();
        emit overlayActivated(index);
    }
}

void AbstractWidgetDelegateOverlay::slotWidgetAboutToShow(const QModelIndex& index)
{
    Q_UNUSED(index);
    m_widget.data()->show();
}

bool AbstractWidgetDelegateOverlay::checkIndex(const QModelIndex& index) const
{
    Q_UNUSED(index);
    return true;
}

void AbstractWidgetDelegateOverlay::slotViewportEntered()
{
    hide();
}

void AbstractWidgetDelegateOverlay::slotRowsRemoved(const QModelIndex&, int, int)
{
    hide();
}

void AbstractWidgetDelegateOverlay::slotLayoutChanged()
{
    hide();
}

void AbstractWidgetDelegateOverlay::viewportLeaveEvent(QObject*, QEvent*)
{
    hide();
    emit overlayHidden();
}

bool AbstractWidgetDelegateOverlay::eventFilter(QObject* obj, QEvent* event)
{
    if (!m_widget.isNull() && obj == m_widget.data()) {
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
                viewportLeaveEvent(obj, event);
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

    return ContactDelegateOverlay::eventFilter(obj, event);
}

// -----------------------------

HoverButtonDelegateOverlay::HoverButtonDelegateOverlay(QObject* parent)
    : AbstractWidgetDelegateOverlay(parent)
{
}

ContactViewHoverButton* HoverButtonDelegateOverlay::button() const
{
    return qobject_cast<ContactViewHoverButton*>(m_widget.data());
}

void HoverButtonDelegateOverlay::setActive(bool active)
{
    AbstractWidgetDelegateOverlay::setActive(active);

    if (active) {
        button()->initIcon();
    }
}

QWidget* HoverButtonDelegateOverlay::createWidget()
{
    return createButton();
}

void HoverButtonDelegateOverlay::visualChange()
{
    if (!m_widget.isNull() && m_widget.data()->isVisible()) {
        updateButton(button()->index());
    }
}

void HoverButtonDelegateOverlay::slotReset()
{
    AbstractWidgetDelegateOverlay::slotReset();

    button()->reset();
}

void HoverButtonDelegateOverlay::slotEntered(const QModelIndex& index)
{
    AbstractWidgetDelegateOverlay::slotEntered(index);

    if (index.isValid() && checkIndex(index)) {
        button()->setIndex(index);
        updateButton(index);
    } else {
        button()->setIndex(index);
    }
}

// -----------------------------

ContactDelegateOverlayContainer::~ContactDelegateOverlayContainer()
{
}

void ContactDelegateOverlayContainer::installOverlay(ContactDelegateOverlay* overlay)
{
    if (!overlay->acceptsDelegate(asDelegate())) {
        kError() << "Cannot accept delegate" << asDelegate() << "for installing" << overlay;
        return;
    }

    overlay->setDelegate(asDelegate());
    m_overlays << overlay;
    // let the view call setActive

//     QObject::connect(overlay, SIGNAL(destroyed(QObject*)),
//                      asDelegate(), SLOT(overlayDestroyed(QObject*)));
}

void ContactDelegateOverlayContainer::removeOverlay(ContactDelegateOverlay* overlay)
{
    overlay->setActive(false);
    overlay->setDelegate(0);
    m_overlays.removeAll(overlay);
    QObject::disconnect(overlay, 0, asDelegate(), 0);
}

void ContactDelegateOverlayContainer::setAllOverlaysActive(bool active)
{
    foreach (ContactDelegateOverlay* overlay, m_overlays) {
        overlay->setActive(active);
    }
}

void ContactDelegateOverlayContainer::setViewOnAllOverlays(QAbstractItemView* view)
{
    foreach (ContactDelegateOverlay* overlay, m_overlays) {
        overlay->setView(view);
    }
}

void ContactDelegateOverlayContainer::removeAllOverlays()
{
    foreach (ContactDelegateOverlay* overlay, m_overlays) {
        overlay->setActive(false);
        overlay->setDelegate(0);
        overlay->setView(0);
    }
    m_overlays.clear();
}

void ContactDelegateOverlayContainer::overlayDestroyed(QObject* o)
{
    ContactDelegateOverlay* overlay = qobject_cast<ContactDelegateOverlay*>(o);
    if (overlay) {
        removeOverlay(overlay);
    }
}

void ContactDelegateOverlayContainer::mouseMoved(QMouseEvent* e, const QRect& visualRect, const QModelIndex& index)
{
    foreach (ContactDelegateOverlay* overlay, m_overlays) {
        overlay->mouseMoved(e, visualRect, index);
    }
}

void ContactDelegateOverlayContainer::drawDelegates(QPainter* p, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    foreach (ContactDelegateOverlay* overlay, m_overlays) {
        overlay->paint(p, option, index);
    }
}

#include "contact-delegate-overlay.moc"
