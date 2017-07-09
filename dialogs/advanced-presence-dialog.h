/*
  Copyright © 2017 James D. Smith <smithjd15@gmail.com>

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

#ifndef ADVANCED_PRESENCE_DIALOG_H
#define ADVANCED_PRESENCE_DIALOG_H

#include <QDialog>
#include <QComboBox>

class QPushButton;
class QListView;
class QComboBox;
class AccountPresenceModelExtended;

namespace KTp {
    class Presence;
    class GlobalPresence;
    class PresenceModel;
    class AccountsListModel;
}

class AdvancedPresenceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AdvancedPresenceDialog(KTp::PresenceModel *presenceModel, KTp::GlobalPresence *globalPresence, QWidget *parent = 0);
    bool eventFilter(QObject* obj, QEvent* event);

private:
    ///Setup the initial dialog
    void setupDialog();

    KTp::PresenceModel *m_presenceModel;
    KTp::AccountsListModel *m_accountsModel;
    KTp::GlobalPresence *m_globalPresence;

    QHash<int,QComboBox*> m_comboBoxes;
    QHash<int,AccountPresenceModelExtended*> m_extendedModels;
};

#endif // ADVANCED_PRESENCE_DIALOG_H
