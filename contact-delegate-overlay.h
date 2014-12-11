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

#ifndef CONTACTDELEGATEOVERLAY_H
#define CONTACTDELEGATEOVERLAY_H

// Qt includes

#include <QAbstractItemView>
#include <QPointer>

class ContactViewHoverButton;

class ContactDelegateOverlay : public QObject
{
    Q_OBJECT

public:
    ContactDelegateOverlay(QObject *parent = 0);
    ~ContactDelegateOverlay();

    /** If active is true, this will call createWidget(), initialize the widget for use,
     *  and setup connections for the virtual slots.
     *  If active is false, this will delete the widget and
     *  disconnect all signal from model and view to this object (!) */
    virtual void setActive(bool active);

    ContactViewHoverButton* button() const;

    void setView(QAbstractItemView *view);
    QAbstractItemView* view() const;
    void setDelegate(QAbstractItemDelegate *delegate);
    QAbstractItemDelegate* delegate() const;
    virtual bool acceptsDelegate(QAbstractItemDelegate*) const { return true; }

Q_SIGNALS:
    /// Emitted when the overlay is shown
    void overlayActivated(QModelIndex);

    /// Emitted when the overlay is hidden
    void overlayHidden();

    void update(const QModelIndex &index);

protected Q_SLOTS:
    /** Called when any change from the delegate occurs - when the overlay is installed,
     *  when size hints, styles or fonts change */
    virtual void visualChange();

    /** Default implementation shows the widget iff the index is valid and checkIndex returns true. */
    virtual void slotEntered(const QModelIndex &index);

    /** Hides and resets the button */
    virtual void slotReset();

    /** Called when the widget shall be hidden (mouse cursor left index, viewport, uninstalled etc.).
     *  Default implementation hide()s m_widget. */
    virtual void slotHideButton();

protected:
    /** Return true here if you want to show the overlay for the given index.
     *  The default implementation returns true. */
    virtual bool checkIndex(const QModelIndex &index) const;

    /** Create your widget here. Pass view() as parent. */
    virtual ContactViewHoverButton* createButton() = 0;

    /** Called when a new index is entered. Reposition your button here,
     *  adjust and store state. */
    virtual void updateButton(const QModelIndex &index) = 0;

    bool eventFilter(QObject *obj, QEvent *event);

    QAbstractItemView     *m_view;
    QAbstractItemDelegate *m_delegate;
    QPointer<ContactViewHoverButton> m_button;
    bool m_mouseButtonPressedOnWidget;
};

#define REQUIRE_DELEGATE(Delegate) \
public: \
    void setDelegate(Delegate* delegate) { ContactDelegateOverlay::setDelegate(delegate); } \
    Delegate* delegate() const { return static_cast<Delegate*>(ContactDelegateOverlay::delegate()); } \
    virtual bool acceptsDelegate(QAbstractItemDelegate*d) const { return dynamic_cast<Delegate*>(d); } \
private:


// -------------------------------------------------------------------------------------------

class ContactDelegateOverlayContainer
{
public:
    /**
     * This is a sample implementation for
     * delegate management methods, to be inherited by a delegate.
     * Does not inherit QObject, the delegate already does.
     */

    virtual ~ContactDelegateOverlayContainer();

    void installOverlay(ContactDelegateOverlay *overlay);
    void removeOverlay(ContactDelegateOverlay *overlay);
    void setAllOverlaysActive(bool active);
    void setViewOnAllOverlays(QAbstractItemView *view);
    void removeAllOverlays();

protected:
    /// Declare as slot in the derived class calling this method
    virtual void overlayDestroyed(QObject *o);

    /// Returns the delegate, typically, the derived class
    virtual QAbstractItemDelegate* asDelegate() = 0;

protected:
    QList<ContactDelegateOverlay*> m_overlays;

};

#endif /* CONTACTDELEGATEOVERLAY_H */
