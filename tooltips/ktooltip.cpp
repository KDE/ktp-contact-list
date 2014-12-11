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

#include "ktooltip.h"
#include "ktooltipwindow_p.h"

#include <QLabel>
#include <QPointer>
#include <QPoint>
#include <QWidget>
#include <QtGlobal>


class KToolTipManager
{
public:
    KToolTipManager();
    ~KToolTipManager();

    void showTip(const QPoint &pos, QWidget *content);
    KToolTipWindow *createTipWindow(QWidget *content);
    KToolTipWindow *currentTip();
    void hideTip();

private:
    QPointer<KToolTipWindow> m_window;
};

Q_GLOBAL_STATIC(KToolTipManager, s_instance)

KToolTipManager::KToolTipManager()
{
}

KToolTipManager::~KToolTipManager()
{
    if (!m_window.isNull()) {
        m_window.data()->deleteLater();
    }
}

void KToolTipManager::showTip(const QPoint &pos, QWidget *content)
{
    hideTip();
    if (!m_window.isNull()) {
        delete m_window.data();
    }
    KToolTipWindow *tooltip = qobject_cast<KToolTipWindow*>(content);
    m_window = (tooltip ? tooltip : createTipWindow(content));
    m_window.data()->move(pos);
    m_window.data()->show();
}

KToolTipWindow *KToolTipManager::createTipWindow(QWidget* content)
{
    return new KToolTipWindow(content);
}

KToolTipWindow *KToolTipManager::currentTip()
{
    return m_window.data();
}

void KToolTipManager::hideTip()
{
    if (!m_window.isNull()) {
        m_window.data()->hide();
        m_window.data()->deleteLater();
    }
}

namespace KToolTip
{
    void showText(const QPoint &pos, const QString &text)
    {
        QLabel *label = new QLabel(text);
        label->setForegroundRole(QPalette::ToolTipText);
        showTip(pos, createTipWindow(label));
    }

    void showTip(const QPoint &pos, QWidget *content)
    {
        s_instance->showTip(pos, content);
    }

    QWidget *createTipWindow(QWidget *content)
    {
        return s_instance->createTipWindow(content);
    }

    QWidget *currentTip()
    {
        return s_instance->currentTip();
    }

    void hideTip()
    {
        s_instance->hideTip();
    }
}

