/*
    Qt item view mouse hover button

    Copyright (C) 2008 Peter Penz <peter dot penz at gmx dot at>
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

#ifndef CONTACTVIEWHOVERBUTTON_H
#define CONTACTVIEWHOVERBUTTON_H

// Qt includes

#include <QtGui/QAbstractButton>
#include <QtGui/QAbstractItemView>

// Local includes

class QTimeLine;

class ContactViewHoverButton : public QAbstractButton
{
    Q_OBJECT

public:
    ContactViewHoverButton(QAbstractItemView *parentView);
    void initIcon();
    void reset();
    void setIndex(const QModelIndex &index);
    QModelIndex index() const;
    void setVisible(bool visible);

    /// Reimplement to match the size of your icon
    virtual QSize sizeHint() const = 0;

protected:
    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);
    void paintEvent(QPaintEvent *event);

    /// Return your icon here. Will be queried again on toggle.
    virtual QPixmap icon() = 0;
    /// Optionally update tooltip here. Will be called again on state change.
    virtual void updateToolTip();

protected Q_SLOTS:
    void setFadingValue(int value);
    void refreshIcon();
    void startFading();
    void stopFading();

protected:
    QPersistentModelIndex m_index;
    bool                  m_isHovered;
    int                   m_fadingValue;
    QPixmap               m_icon;
    QTimeLine            *m_fadingTimeLine;
};

#endif /* CONTACTVIEWHOVERBUTTON_H */
