/*
 * Person Tooltip
 *
 * Copyright (C) 2011 David Edmundson <kde@davidedmundson.co.uk>
 * Copyright (C) 2013 Martin Klapetek <mklapetek@kde.org>
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

#ifndef PERSONTOOLTIP_H
#define PERSONTOOLTIP_H

#include <QWidget>
#include <QModelIndex>


namespace Ui {
    class PersonToolTip;
}

class PersonToolTip : public QWidget
{
    Q_OBJECT

public:
    explicit PersonToolTip(const QModelIndex &index);
    ~PersonToolTip();

    static QString getTextWithHyperlinks(QString text);

public slots:
    void openLink(QString);

private:
    Ui::PersonToolTip *ui;
};

Q_DECLARE_METATYPE(QModelIndex);

#endif // PERSONTOOLTIP_H
