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

#include "advanced-presence-dialog.h"
#include "global-presence-chooser.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QModelIndex>
#include <QComboBox>
#include <QDialogButtonBox>

#include <KLocalizedString>

#include <QLabel>
#include <QCheckBox>
#include <QLineEdit>
#include <QKeyEvent>

#include <KTp/presence.h>
#include <KTp/global-presence.h>
#include <KTp/Models/presence-model.h>
#include <KTp/Models/accounts-list-model.h>

//A sneaky class that adds an extra entries to the end of the presence model.
class AccountPresenceModelExtended : public QAbstractListModel
{
    Q_OBJECT
public:
    AccountPresenceModelExtended(KTp::PresenceModel *presenceModel, QObject *parent);
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    /** Adds a presence to the model which is to be used when the presence has been set externally and we need to show it, but not save it to the config*/
    QModelIndex addTemporaryPresence(const KTp::Presence &presence);
    void removeTemporaryPresence();
private slots:
    void sourceRowsInserted(const QModelIndex &index, int start, int end);
    void sourceRowsRemoved(const QModelIndex &index, int start, int end);
private:
    KTp::Presence m_temporaryPresence;
    KTp::PresenceModel *m_model;
};

AccountPresenceModelExtended::AccountPresenceModelExtended(KTp::PresenceModel *presenceModel, QObject *parent) :
    QAbstractListModel(parent),
    m_model(presenceModel)
{
    connect(m_model, &QAbstractItemModel::rowsInserted, this, &AccountPresenceModelExtended::sourceRowsInserted);
    connect(m_model, &QAbstractItemModel::rowsRemoved, this, &AccountPresenceModelExtended::sourceRowsRemoved);
}

//return number of rows + the extra items added to end of list
int AccountPresenceModelExtended::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    int rowCount = m_model->rowCount(parent);
    if (m_temporaryPresence.isValid()) {
        rowCount++;
    }
    return rowCount;
}

QVariant AccountPresenceModelExtended::data(const QModelIndex &index, int role) const
{
    if (m_temporaryPresence.isValid() && index.row() == rowCount() - 1) {
        switch (role) {
        case Qt::DisplayRole:
            return m_temporaryPresence.statusMessage();
        case Qt::DecorationRole:
            return m_temporaryPresence.icon();
        case KTp::PresenceModel::PresenceRole:
            return QVariant::fromValue<KTp::Presence>(m_temporaryPresence);
        }
    } else {
        return m_model->data(m_model->index(index.row()), role);
    }
    return QVariant();
}

void AccountPresenceModelExtended::sourceRowsInserted(const QModelIndex &index, int start, int end)
{
    beginInsertRows(createIndex(index.row(), 0), start, end);
    endInsertRows();
}

void AccountPresenceModelExtended::sourceRowsRemoved(const QModelIndex &index, int start, int end)
{
    beginRemoveRows(createIndex(index.row(), 0), start, end);
    endRemoveRows();
}

void AccountPresenceModelExtended::removeTemporaryPresence()
{
    if (!m_temporaryPresence.isValid()) {
        return; //if not already set, do nothing.
    }

    int row = m_model->rowCount();
    beginRemoveRows(QModelIndex(), row, row);
    m_temporaryPresence = KTp::Presence();
    endRemoveRows();
}

QModelIndex AccountPresenceModelExtended::addTemporaryPresence(const KTp::Presence &presence)
{
    int row = m_model->rowCount();

    //if the temp presence already exists, don't remove and readd it
    //but simply replace it
    if (m_temporaryPresence.isValid()) {
        m_temporaryPresence = presence;
        emit dataChanged(this->createIndex(row, 0), this->createIndex(row, 0));
    } else {
        beginInsertRows(QModelIndex(), row, row);
        m_temporaryPresence = presence;
        endInsertRows();
    }

    return this->createIndex(row, 0);
}

AdvancedPresenceDialog::AdvancedPresenceDialog(KTp::PresenceModel *presenceModel, KTp::GlobalPresence *globalPresence, QWidget *parent)
    : QDialog(parent),
      m_presenceModel(presenceModel),
      m_accountsModel(new KTp::AccountsListModel()),
      m_globalPresence(globalPresence)
{
    setupDialog();
}

void AdvancedPresenceDialog::setupDialog()
{
    setWindowTitle(i18n("Advanced Presence Setting"));

    QVBoxLayout *vLayout = new QVBoxLayout();
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, this);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    if (m_globalPresence->enabledAccounts()->accounts().isEmpty()) {
        QLabel *emptyBox = new QLabel();
        emptyBox->setText(i18n("It appears that you do not have any accounts configured"));
        QVBoxLayout *emptyAccountLayout = new QVBoxLayout();
        emptyAccountLayout->addWidget(emptyBox);

        vLayout->addWidget(emptyBox);
    } else {
        m_accountsModel->setAccountSet(m_globalPresence->enabledAccounts());
        for (int i = 0; i < m_accountsModel->rowCount(); i++) {
            const QModelIndex &index = m_accountsModel->index(i, 0);

            if (!m_accountsModel->data(index, KTp::AccountsListModel::AccountRole).value<Tp::AccountPtr>()->isValid())
                continue;

            QVBoxLayout *vAccountLayout = new QVBoxLayout();
            QHBoxLayout *hAccountLayout = new QHBoxLayout();
            QHBoxLayout *lHAccountLayout = new QHBoxLayout();

            const QIcon &accountIcon = m_accountsModel->data(index, Qt::DecorationRole).value<QIcon>();
            QLabel *icoLabel = new QLabel();
            icoLabel->setPixmap(accountIcon.pixmap(accountIcon.actualSize(QSize(16, 16))));
            QLabel *label = new QLabel(m_accountsModel->data(index, Qt::DisplayRole).value<QString>());

            auto setComboLineEdit = [=] () {
                if (m_comboBoxes[i]->currentData(KTp::PresenceModel::PresenceRole).value<KTp::Presence>().statusMessage().isEmpty()) {
                    m_comboBoxes[i]->lineEdit()->setPlaceholderText(i18n("Set a status message ..."));
                    m_comboBoxes[i]->lineEdit()->setReadOnly(false);
                } else {
                    m_comboBoxes[i]->lineEdit()->setPlaceholderText(m_comboBoxes[i]->currentData(Qt::DisplayRole).value<QString>());
                    m_comboBoxes[i]->lineEdit()->setReadOnly(true);
                }

                m_comboBoxes[i]->lineEdit()->setToolTip(m_comboBoxes[i]->currentData(Qt::DisplayRole).value<QString>());
            };

            QCheckBox *checkBox = new QCheckBox();
            checkBox->setChecked(true);
            connect(checkBox, &QCheckBox::clicked, [=] (bool checked) {
                m_comboBoxes[i]->setEnabled(checked);
                KTp::Presence presence;
                if (checked) {
                    setComboLineEdit();
                    presence = m_comboBoxes[i]->currentData(KTp::PresenceModel::PresenceRole).value<KTp::Presence>();
                } else {
                    m_comboBoxes[i]->lineEdit()->setPlaceholderText(m_comboBoxes[i]->currentData(Qt::DisplayRole).value<QString>());
                    presence.setStatus(Tp::ConnectionPresenceTypeUnset, QLatin1String("unset"), QString());
                }

                m_accountsModel->setData(index, QVariant::fromValue<KTp::Presence>(presence), KTp::AccountsListModel::StatusHandlerPresenceRole);
            });

            KTp::Presence accountPresence = m_accountsModel->data(index, KTp::AccountsListModel::StatusHandlerPresenceRole).value<KTp::Presence>();
            if (accountPresence.type() == Tp::ConnectionPresenceTypeUnset) {
                if (m_globalPresence->globalPresence().type() == Tp::ConnectionPresenceTypeUnset) {
                    accountPresence = m_accountsModel->data(index, KTp::AccountsListModel::RequestedPresenceRole).value<KTp::Presence>();
                } else {
                    accountPresence = m_globalPresence->globalPresence();
                    checkBox->setChecked(false);
                }
            }

            m_comboBoxes.insert(i, new QComboBox());
            m_extendedModels.insert(i, new AccountPresenceModelExtended(m_presenceModel, this));
            m_comboBoxes[i]->setModel(m_extendedModels[i]);
            m_comboBoxes[i]->setEnabled(checkBox->isChecked());
            m_comboBoxes[i]->setEditable(true);
            m_comboBoxes[i]->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
            m_comboBoxes[i]->setWhatsThis(KDED_STATUS_MESSAGE_PARSER_WHATSTHIS);
            m_comboBoxes[i]->installEventFilter(this);

            int width = m_comboBoxes[i]->minimumSizeHint().width();
            m_comboBoxes[i]->setMinimumContentsLength(width);

            connect(m_comboBoxes[i], static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), [=] {
                setComboLineEdit();

                if (m_comboBoxes[i]->currentIndex() < m_presenceModel->rowCount()) {
                    m_extendedModels[i]->removeTemporaryPresence();
                }

                m_accountsModel->setData(index, m_comboBoxes[i]->currentData(KTp::PresenceModel::PresenceRole), KTp::AccountsListModel::StatusHandlerPresenceRole);
            });

            const QModelIndexList &matchIndexList = m_presenceModel->match(m_presenceModel->index(0, 0), KTp::PresenceModel::PresenceRole, QVariant::fromValue<KTp::Presence>(accountPresence));
            if (!matchIndexList.isEmpty()) {
                m_comboBoxes[i]->setCurrentIndex(matchIndexList.at(0).row());
            } else {
                const QModelIndex &tempPresenceIndex = m_extendedModels[i]->addTemporaryPresence(accountPresence);
                m_comboBoxes[i]->setCurrentIndex(tempPresenceIndex.row());
            }

            setComboLineEdit();

            lHAccountLayout->addWidget(icoLabel);
            lHAccountLayout->addWidget(label, Qt::AlignLeft);
            vAccountLayout->addLayout(lHAccountLayout);
            hAccountLayout->addWidget(checkBox);
            hAccountLayout->addWidget(m_comboBoxes[i]);
            vAccountLayout->addLayout(hAccountLayout);
            vLayout->addLayout(vAccountLayout);
        }
    }

    vLayout->addWidget(buttonBox);
    setLayout(vLayout);
}

bool AdvancedPresenceDialog::eventFilter(QObject* obj, QEvent* event)
{
    QComboBox *comboBox = qobject_cast<QComboBox*>(obj);

    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->modifiers() == Qt::NoModifier && (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) && m_comboBoxes.values().contains(comboBox)) {
            const QModelIndex &index = m_accountsModel->index(m_comboBoxes.key(comboBox), 0);
            KTp::Presence accountPresence = comboBox->currentData(KTp::PresenceModel::PresenceRole).value<KTp::Presence>();
            accountPresence.setStatusMessage(comboBox->lineEdit()->text());

            const QModelIndexList &matchIndexList = m_presenceModel->match(m_presenceModel->index(0, 0), KTp::PresenceModel::PresenceRole, QVariant::fromValue<KTp::Presence>(accountPresence));
            if (matchIndexList.isEmpty()) {
                const QModelIndex &tempPresenceIndex = m_extendedModels[index.row()]->addTemporaryPresence(accountPresence);
                comboBox->setCurrentIndex(tempPresenceIndex.row());
            }

            comboBox->lineEdit()->setPlaceholderText(comboBox->currentData(Qt::DisplayRole).value<QString>());
            comboBox->lineEdit()->setToolTip(comboBox->currentData(Qt::DisplayRole).value<QString>());
            comboBox->clearFocus();

            m_accountsModel->setData(index, comboBox->currentData(KTp::PresenceModel::PresenceRole), KTp::AccountsListModel::StatusHandlerPresenceRole);

            return true;
        } else {
            return false;
        }
    }

    if (event->type() == QEvent::FocusOut) {
        comboBox->clearFocus();
    }

    // standard event processing
    return QObject::eventFilter(obj, event);
}

#include "advanced-presence-dialog.moc"
