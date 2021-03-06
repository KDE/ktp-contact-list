/*
 * Copyright (C) 2008 by Fredrik Höglund <fredrik@kde.org>
 * Copyright (C) 2011 by Geoffry Song <goffrie@gmail.com>
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

#ifndef KTOOLTIP_H
#define KTOOLTIP_H

class QPoint;
class QString;
class QWidget;

/**
 * Allows to show tooltips having a widget as content.
 */
namespace KToolTip
{
    void showText(const QPoint &pos, const QString &text);

    /**
     * Shows the tip @p content at the global position indicated by @p pos.
     *
     * Ownership of the content widget is transferred to KToolTip. The widget will be deleted
     * automatically when it is hidden.
     *
     * The tip is shown immediately when this function is called.
     */
    void showTip(const QPoint &pos, QWidget *window);
    QWidget *createTipWindow(QWidget *content);
    QWidget *currentTip();
    void hideTip();
}

#endif
