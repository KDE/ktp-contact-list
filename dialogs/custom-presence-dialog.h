/*
  Copyright © 2011 Rohan Garg <rohan16garg@gmail.com>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License as
  published by the Free Software Foundation; either version 2 of
  the License or (at your option) version 3 or any later version
  accepted by the membership of KDE e.V. (or its successor approved
  by the membership of KDE e.V.), which shall act as a proxy
  defined in Section 14 of version 3 of the license.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef CUSTOM_PRESENCE_DIALOG_H
#define CUSTOM_PRESENCE_DIALOG_H

//Qt includes
#include <QWidget>

//KDE includes
#include <KComboBox>

//Forward declrare classes
class KIcon;
class QListWidget;
class KConfig;
class KConfigGroup;

class customPresenceDialog : public QWidget
{
  Q_OBJECT

public:
    explicit customPresenceDialog(QWidget *parent = 0);

public Q_SLOTS:
    ///Adds a custom presence to the config file
    void addCustomPresence();

    ///Removes a custom presence from the config file
    void removeCustomPresence();

Q_SIGNALS:
  void configChanged();

private:
    ///Setup the initial dialog
    void setupDialog();

    ///Returns corresponding icon for index read from the config file
    static KIcon iconForIndex(int index);

    ///Retruns corresponding index for icon
    static int indexForIcon(KIcon icon);

    ///ListWidget to display custom presence's
    QListWidget  *m_listWidget;

    ///Combobox to type custom presence's
    KComboBox    *m_statusMessage;

    ///KConfig variable to read/write/sync changes to disk
    KConfig      *m_config;

    ///KConfigGroup variable to read/write/sync changes to disk
    KConfigGroup *m_presenceGroup;
  
};

#endif // CUSTOM_PRESENCE_DIALOG_H
