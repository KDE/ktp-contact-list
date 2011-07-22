/*
 * Contact List Application
 *
 * Copyright (C) 2011 David Edmundson <kde@davidedmundson.co.uk>
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

#ifndef CONTACTLISTAPPLICATION_H
#define CONTACTLISTAPPLICATION_H

#include <KUniqueApplication>

class ContactListApplication : public KUniqueApplication
{
    Q_OBJECT
public:
    explicit ContactListApplication();
    virtual void commitData(QSessionManager &sm);

    /** Returns true if the application is starting to shut down, if set no warning dialogs should be shown to the user*/
    bool isShuttingDown() const;

private:
    bool m_isShuttingDown;

};

#endif // CONTACTLISTAPPLICATION_H
