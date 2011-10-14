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

#include "custom-presence-dialog.h"

#include "presence-model.h"

#include <QtGui/QListView>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QPushButton>
#include <QtGui/QSortFilterProxyModel>
#include <QtCore/QModelIndex>

#include <KDE/KDialog>
#include <KDE/KLocalizedString>
#include <KDE/KConfig>
#include <KDE/KSharedConfigPtr>

#include <TelepathyQt4/Presence>
#include <QLineEdit>

class FilteredModel : public QSortFilterProxyModel {
public:
    FilteredModel(QObject *parent);
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
};

FilteredModel::FilteredModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

bool FilteredModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
    Tp::Presence presence = index.data(PresenceModel::PresenceRole).value<Tp::Presence>();
    return ! presence.statusMessage().isEmpty();
}

CustomPresenceDialog::CustomPresenceDialog(PresenceModel *model, QWidget *parent)
    : KDialog(parent),
      m_model(model)
{
    setupDialog();
}

void CustomPresenceDialog::setupDialog()
{
    setCaption(i18n("Edit Custom Presences"));
    setButtons(KDialog::Close);

    QWidget *mainDialogWidget = new QWidget(this);

    FilteredModel *filteredModel = new FilteredModel(this);
    filteredModel->setSourceModel(m_model);

    m_listView = new QListView(mainDialogWidget);
    m_listView->setModel(filteredModel);

    m_statusMessage = new KComboBox(true, mainDialogWidget);
    m_statusMessage->setTrapReturnKey(false);

    m_statusMessage->addItem(KIcon("user-online"), i18n("Set custom available message ..."),qVariantFromValue(Tp::Presence::available()));
    m_statusMessage->addItem(KIcon("user-busy"), i18n("Set custom busy message ..."), qVariantFromValue(Tp::Presence::busy()));
    m_statusMessage->addItem(KIcon("user-away"), i18n("Set custom away message ..."), qVariantFromValue(Tp::Presence::away()));
    m_statusMessage->addItem(KIcon("user-away-extended"), i18n("Set custom extended away message ..."), qVariantFromValue(Tp::Presence::xa()));

    m_statusMessage->setAutoCompletion(false);
    m_statusMessage->show();

    m_statusMessage->lineEdit()->setPlaceholderText(m_statusMessage->currentText());
    m_statusMessage->lineEdit()->setText(QString());

    QPushButton *addStatus = new QPushButton(KIcon("list-add"), i18n("Add Presence"), mainDialogWidget);
    QPushButton *removeStatus = new QPushButton(KIcon("list-remove"), i18n("Remove Presence"), mainDialogWidget);

    QVBoxLayout *vLayout = new QVBoxLayout(mainDialogWidget);
    vLayout->addWidget(m_statusMessage);

    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->addWidget(m_listView);

    QVBoxLayout *vLayout2 = new QVBoxLayout();
    vLayout2->addWidget(addStatus);
    vLayout2->addWidget(removeStatus);
    vLayout2->addStretch(1);

    hLayout->addLayout(vLayout2);
    vLayout->addLayout(hLayout);

    setMainWidget(mainDialogWidget);

    connect(addStatus, SIGNAL(clicked()), SLOT(addCustomPresence()));
    connect(removeStatus, SIGNAL(clicked()), SLOT(removeCustomPresence()));
    connect(m_statusMessage, SIGNAL(returnPressed()), SLOT(addCustomPresence()));
    connect(m_statusMessage, SIGNAL(currentIndexChanged(QString)), SLOT(comboboxIndexChanged(QString)));
}

void CustomPresenceDialog::addCustomPresence()
{
    int presenceIndex = m_statusMessage->currentIndex();
    Tp::Presence presence = m_statusMessage->itemData(presenceIndex).value<Tp::Presence>();
    presence.setStatus(presence.type(), QString(), m_statusMessage->currentText());

    m_model->addPresence(presence);
}

void CustomPresenceDialog::removeCustomPresence()
{
    if (! m_listView->currentIndex().isValid()) {
        return;
    }

    Tp::Presence presence = m_listView->currentIndex().data(PresenceModel::PresenceRole).value<Tp::Presence>();
    m_model->removePresence(presence);
}

void CustomPresenceDialog::comboboxIndexChanged(const QString& text)
{
    m_statusMessage->lineEdit()->setText(QString());
    m_statusMessage->lineEdit()->setPlaceholderText(text);
}
