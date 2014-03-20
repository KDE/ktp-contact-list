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

#include <TelepathyQt/Presence>
#include <QLineEdit>
#include <QKeyEvent>

#include <KTp/Models/presence-model.h>

class FilteredModel : public QSortFilterProxyModel
{
    Q_OBJECT
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
    KTp::Presence presence = index.data(KTp::PresenceModel::PresenceRole).value<KTp::Presence>();
    return ! presence.statusMessage().isEmpty();
}

CustomPresenceDialog::CustomPresenceDialog(KTp::PresenceModel *model, QWidget *parent)
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

    connect(m_listView, SIGNAL(clicked(QModelIndex)),
            this, SLOT(presenceViewSelectionChanged(QModelIndex)));

    connect(m_listView, SIGNAL(activated(QModelIndex)),
            this, SLOT(presenceViewSelectionChanged(QModelIndex)));

    m_statusMessage = new KComboBox(true, mainDialogWidget);

    m_statusMessage->addItem(KIcon("user-online"), i18n("Set custom available message..."), qVariantFromValue<KTp::Presence>(Tp::Presence::available()));
    m_statusMessage->addItem(KIcon("user-busy"), i18n("Set custom busy message..."), qVariantFromValue<KTp::Presence>(Tp::Presence::busy()));
    m_statusMessage->addItem(KIcon("user-away"), i18n("Set custom away message..."), qVariantFromValue<KTp::Presence>(Tp::Presence::away()));
    m_statusMessage->addItem(KIcon("user-away-extended"), i18n("Set custom extended away message..."), qVariantFromValue<KTp::Presence>(Tp::Presence::xa()));

    m_statusMessage->setAutoCompletion(false);
    m_statusMessage->show();

    m_statusMessage->lineEdit()->setPlaceholderText(m_statusMessage->currentText());

    connect(m_statusMessage, SIGNAL(editTextChanged(QString)),
            this, SLOT(presenceMessageTextChanged(QString)));

    m_addStatus = new QPushButton(KIcon("list-add"), i18n("Add Presence"), mainDialogWidget);
    m_removeStatus = new QPushButton(KIcon("list-remove"), i18n("Remove Presence"), mainDialogWidget);
    m_removeStatus->setEnabled(false);

    //this triggers the presenceMessageTextChanged() slot and disables the m_addStatus button
    m_statusMessage->lineEdit()->setText(QString());

    QVBoxLayout *vLayout = new QVBoxLayout(mainDialogWidget);
    vLayout->addWidget(m_statusMessage);

    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->addWidget(m_listView);

    QVBoxLayout *vLayout2 = new QVBoxLayout();
    vLayout2->addWidget(m_addStatus);
    vLayout2->addWidget(m_removeStatus);
    vLayout2->addStretch(1);

    hLayout->addLayout(vLayout2);
    vLayout->addLayout(hLayout);

    setMainWidget(mainDialogWidget);

    connect(m_addStatus, SIGNAL(clicked()), SLOT(addCustomPresence()));
    connect(m_removeStatus, SIGNAL(clicked()), SLOT(removeCustomPresence()));
    connect(m_statusMessage, SIGNAL(currentIndexChanged(QString)), SLOT(comboboxIndexChanged(QString)));

    m_statusMessage->installEventFilter(this);
}

void CustomPresenceDialog::addCustomPresence()
{
    int presenceIndex = m_statusMessage->currentIndex();
    KTp::Presence presence = m_statusMessage->itemData(presenceIndex).value<KTp::Presence>();
    presence.setStatus(presence.type(), QString(), m_statusMessage->currentText());

    m_listView->setCurrentIndex(qobject_cast<FilteredModel*>(m_listView->model())->mapFromSource(m_model->addPresence(presence)));
    m_statusMessage->lineEdit()->clear();
    m_listView->setFocus();
    m_removeStatus->setEnabled(true);

    m_model->syncCustomPresencesToDisk();
    m_model->updatePresenceApplet();
}

void CustomPresenceDialog::removeCustomPresence()
{
    if (! m_listView->currentIndex().isValid()) {
        return;
    }

    KTp::Presence presence = m_listView->currentIndex().data(KTp::PresenceModel::PresenceRole).value<KTp::Presence>();
    m_model->removePresence(presence);

    if (m_listView->model()->rowCount(QModelIndex()) == 0) {
        m_removeStatus->setEnabled(false);
    }

    m_model->syncCustomPresencesToDisk();
    m_model->updatePresenceApplet();
}

void CustomPresenceDialog::comboboxIndexChanged(const QString& text)
{
    m_statusMessage->lineEdit()->setText(QString());
    m_statusMessage->lineEdit()->setPlaceholderText(text);
}

bool CustomPresenceDialog::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == m_statusMessage && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->modifiers() == Qt::NoModifier && (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter)) {
            addCustomPresence();
            m_statusMessage->lineEdit()->clear();
            m_listView->setFocus();
            return true;
        } else {
            return false;
        }
    } else {
        // standard event processing
        return QObject::eventFilter(obj, event);
    }
}

void CustomPresenceDialog::presenceMessageTextChanged(const QString& text)
{
    if (text.isEmpty()) {
        m_addStatus->setEnabled(false);
    } else {
        m_addStatus->setEnabled(true);
    }
}

void CustomPresenceDialog::presenceViewSelectionChanged(const QModelIndex& index)
{
    if (index.isValid()) {
        m_removeStatus->setEnabled(true);
    } else {
        m_removeStatus->setEnabled(false);
    }
}

#include "custom-presence-dialog.moc"
#include "moc_custom-presence-dialog.cpp" //hack because we have two QObejcts in teh same file
