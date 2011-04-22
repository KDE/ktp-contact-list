/***************************************************************************
 *   Copyright (C) 2006-2010 by Peter Penz <peter.penz19@gmail.com>        *
 *   Copyright (C) 2006 by Gregor Kališnik <gregor@podnapisi.net>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
 ***************************************************************************/

#ifndef FILTERBAR_H
#define FILTERBAR_H

#include <QWidget>

class QToolButton;
class KLineEdit;

/**
 * @brief Provides an input field for filtering the currently shown items.
 *
 * @author Gregor Kališnik <gregor@podnapisi.net>
 */
class FilterBar : public QWidget
{
    Q_OBJECT

public:
    explicit FilterBar(QWidget* parent = 0);
    virtual ~FilterBar();

    /**
     * Selects the whole text of the filter bar.
     */
    void selectAll();

    /**
     * \returns true if the pin button is checked, false otherwise.
     */
    bool isPinned() const;

public slots:
    /** Clears the input field. */
    void clear();

    /**
     * Sets the appearance of the pin button.
     *
     * Displays the pin button with a different icon according to its state
     * (checked or unchecked).
     *
     * \param pinned if true, the pin button will be checked, otherwise it
     * will be unchecked.
     */
    void setPinned(bool pinned);

    /**
     * Unpins the bar.
     */
    void unpinSlot();

signals:
    /**
     * Signal that reports the name filter has been
     * changed to \a nameFilter.
     */
    void filterChanged(const QString& nameFilter);

    /**
     * Emitted as soon as the filterbar should get closed.
     */
    void closeRequest();

protected:
    virtual void showEvent(QShowEvent* event);
    virtual void keyReleaseEvent(QKeyEvent* event);

private:
    KLineEdit *m_filterInput;
    /**
     * Button to pin the filter bar to the contact list.
     *
     * The status of the button (checked or unchecked) will be written in the
     * config file when the contact list gets closed, and the filter bar will
     * shown or hidden accordingly next time the contact list is launched.
     */
    QToolButton *m_pinButton;
};

#endif
