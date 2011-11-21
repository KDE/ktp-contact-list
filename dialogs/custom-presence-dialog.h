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

//KDE includes
#include <KComboBox>
#include <KDialog>

class QPushButton;
//Forward declrare classes
class KIcon;
class QListView;
class PresenceModel;

class CustomPresenceDialog : public KDialog
{
    Q_OBJECT

public:
    explicit CustomPresenceDialog(PresenceModel *model, QWidget *parent = 0);
    bool eventFilter(QObject* obj, QEvent* event);

private Q_SLOTS:
    void addCustomPresence();
    void removeCustomPresence();
    void comboboxIndexChanged(const QString &text);
    void presenceMessageTextChanged(const QString &text);
    void presenceViewSelectionChanged(const QModelIndex &index);

private:
    ///Setup the initial dialog
    void setupDialog();

    ///Returns corresponding icon for index read from the config file
    static KIcon iconForIndex(int index);

    ///ListWidget to display custom presence's
    QListView  *m_listView;

    ///Combobox to type custom presence's
    KComboBox    *m_statusMessage;

    PresenceModel *m_model;

    QPushButton *m_addStatus;
    QPushButton *m_removeStatus;

//     FilteredModel *m_filteredModel;
};

#endif // CUSTOM_PRESENCE_DIALOG_H
