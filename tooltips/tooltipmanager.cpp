/*
 * Copyright (C) 2008 by Konstantin Heil <konst.heil@stud.uni-heidelberg.de>
 * Copyright (C) 2011 Geoffry Song <goffrie@gmail.com>
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

#include "tooltipmanager.h"

#include "ktooltip.h"
#include "contacttooltip.h"

#include <QRect>
#include <QTimer>
#include <QPainter>
#include <QScrollBar>
#include <QApplication>
#include <QDesktopWidget>
#include <QAbstractItemView>

#ifdef Q_WS_X11
#include <QX11Info>
#endif

#include <KIcon>
#include <KColorScheme>

#include <KTp/types.h>

class ToolTipManager::Private
{
public:
    Private() :
        view(0),
        timer(0)
        { }

    QAbstractItemView *view;
    QTimer            *timer;
    QPersistentModelIndex        item;
    QRect              itemRect;
};

ToolTipManager::ToolTipManager(QAbstractItemView *parent)
    : QObject(parent)
    , d(new ToolTipManager::Private)
{
    d->view = parent;

    connect(parent, SIGNAL(viewportEntered()), this, SLOT(hideToolTip()));
    connect(parent, SIGNAL(entered(QModelIndex)), this, SLOT(requestToolTip(QModelIndex)));

    d->timer = new QTimer(this);
    d->timer->setSingleShot(true);
    connect(d->timer, SIGNAL(timeout()), this, SLOT(prepareToolTip()));

    // When the mousewheel is used, the items don't get a hovered indication
    // (Qt-issue #200665). To assure that the tooltip still gets hidden,
    // the scrollbars are observed.
    connect(parent->horizontalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(hideToolTip()));
    connect(parent->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(hideToolTip()));

    d->view->viewport()->installEventFilter(this);
}

ToolTipManager::~ToolTipManager()
{
    delete d;
}

bool ToolTipManager::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == d->view->viewport()) {
        switch (event->type()) {
            case QEvent::Leave:
                hideToolTip();
                break;
            case QEvent::MouseButtonPress:
                hideToolTip();
                break;
            case QEvent::ToolTip:
                return true;
            default:
                break;
        }
    } else if (watched == KToolTip::currentTip()) {
        if (event->type() == QEvent::Leave) {
            hideToolTip();
        }
        return false;
    }

    return QObject::eventFilter(watched, event);
}

void ToolTipManager::requestToolTip(const QModelIndex &index)
{
    // only request a tooltip for the name column and when no selection or
    // drag & drop operation is done (indicated by the left mouse button)
    if (!(QApplication::mouseButtons() & Qt::LeftButton) && index.isValid()) {
        KToolTip::hideTip();

        QRect rect = d->view->visualRect(index);
        d->itemRect = QRect(d->view->viewport()->mapToGlobal(rect.topLeft()),
                            d->view->viewport()->mapToGlobal(rect.bottomRight()));
        d->item = index;
        d->timer->start(300);
    } else {
        hideToolTip();
    }
}

void ToolTipManager::hideToolTip()
{
    if ( KToolTip::currentTip() && KToolTip::currentTip()->geometry().contains(QCursor::pos()) ) return;
    d->timer->stop();
    KToolTip::hideTip();
}

void ToolTipManager::prepareToolTip()
{
    if (d->item.isValid()) {
        showToolTip(d->item);
    }
}

void ToolTipManager::showToolTip(const QModelIndex &menuItem)
{
    if (QApplication::mouseButtons() & Qt::LeftButton || !menuItem.isValid()) {
        return;
    }

    if (menuItem.data(KTp::RowTypeRole).toUInt() != KTp::ContactRowType) {
        return;
    }

    QWidget *tip = KToolTip::createTipWindow(createTipContent(menuItem));

    // calculate the x- and y-position of the tooltip
    const QSize size = tip->size();
    const QRect desktop = QApplication::desktop()->screenGeometry( QCursor::pos() );

    // d->itemRect defines the area of the item, where the tooltip should be
    // shown. Per default the tooltip is shown to the right
    // If the tooltip content exceeds the desktop borders, it must be assured that:
    // - the content is fully visible, if possible
    // - the content is not drawn inside d->itemRect
    const int margin = 3;
    const bool hasRoomToLeft  = (d->itemRect.left()   - size.width()  - margin >= desktop.left());
    const bool hasRoomToRight = (d->itemRect.right()  + size.width()  + margin <= desktop.right());
    const bool hasRoomAbove   = (d->itemRect.top()    - size.height() - margin >= desktop.top());
    const bool hasRoomBelow   = (d->itemRect.bottom() + size.height() + margin <= desktop.bottom());
    if (!hasRoomAbove && !hasRoomBelow && !hasRoomToLeft && !hasRoomToRight) {
        delete tip;
        tip = 0;
        return;
    }

    int x = 0;
    int y = 0;

    if (hasRoomToLeft || hasRoomToRight) {
        x = hasRoomToRight ? d->itemRect.right() + margin : d->itemRect.left() - size.width() - margin;
        y = qMin(qMax(desktop.top() + margin, d->itemRect.center().y() - size.height() / 2), desktop.bottom() - size.height() - margin);
    } else {
        Q_ASSERT(hasRoomBelow || hasRoomAbove);
        y = hasRoomBelow ? d->itemRect.bottom() + margin : d->itemRect.top() - size.height() - margin;
        x = qMin(qMax(desktop.left() + margin, d->itemRect.center().x() - size.width() / 2), desktop.right() - size.width() - margin);
    }

    tip->installEventFilter(this);
    // the ownership of tip is transferred to KToolTip
    KToolTip::showTip(QPoint(x, y), tip);
}

QWidget * ToolTipManager::createTipContent(const QModelIndex &index)
{
     return new ContactToolTip(index);
}

#include "tooltipmanager.moc"
