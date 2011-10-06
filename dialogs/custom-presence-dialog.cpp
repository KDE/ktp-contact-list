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

#include <QListWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>

#include <KDialog>
#include <KLocalizedString>
#include <KConfig>
#include <KSharedConfigPtr>

#include <TelepathyQt4/Presence>	

customPresenceDialog::customPresenceDialog(QWidget* parent)
  : QWidget(parent)
{
    setupDialog();
}

void customPresenceDialog::setupDialog()
{
    KDialog *dialog = new KDialog(this);
    dialog->setCaption(i18n("Edit Custom Messages"));
    dialog->setButtons(KDialog::Close);
 
    QWidget *mainDialogWidget = new QWidget(dialog);
    m_listWidget = new QListWidget(mainDialogWidget);
    m_statusMessage = new KComboBox(true, mainDialogWidget);
    m_statusMessage->setTrapReturnKey(false);
 
    m_statusMessage->addItem(KIcon("user-online"), QString("Set custom available message ..."),qVariantFromValue(Tp::Presence::available()));
    m_statusMessage->addItem(KIcon("user-busy"), QString("Set custom busy message ..."), qVariantFromValue(Tp::Presence::busy()));
    m_statusMessage->addItem(KIcon("user-away"), QString("Set custom away message ..."), qVariantFromValue(Tp::Presence::away()));
 
    m_statusMessage->setAutoCompletion(false);
    m_statusMessage->show();
 
    QPushButton *addStatus = new QPushButton(KIcon("list-add"), i18n("Add Status"), mainDialogWidget);
    QPushButton *removeStatus = new QPushButton(KIcon("list-remove"), i18n("Remove Status"), mainDialogWidget);
 
    QVBoxLayout *vLayout = new QVBoxLayout(mainDialogWidget);
    vLayout->addWidget(m_statusMessage);
 
    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->addWidget(m_listWidget);
 
    QVBoxLayout *vLayout2 = new QVBoxLayout();
    vLayout2->addWidget(addStatus);
    vLayout2->addWidget(removeStatus);
    vLayout2->addStretch(1);
 
    hLayout->addLayout(vLayout2);
    vLayout->addLayout(hLayout);

    dialog->setMainWidget(mainDialogWidget);
    dialog->show();
    
    KSharedConfigPtr config = KSharedConfig::openConfig("telepathy-kde-contactlistrc");
    m_presenceGroup = new KConfigGroup( config, "Custom Presence List" );
    int presenceIcon;
    foreach(const QString& presenceString, m_presenceGroup->keyList()) {
       presenceIcon = m_presenceGroup->readEntry<int>(presenceString, 0);
       new QListWidgetItem(iconForIndex(presenceIcon), presenceString.left(presenceString.size() - 2), m_listWidget);
    }
    
    connect(addStatus, SIGNAL(clicked()), SLOT(addCustomPresence()));
    connect(removeStatus, SIGNAL(clicked()), SLOT(removeCustomPresence()));
    connect(m_statusMessage, SIGNAL(returnPressed()), SLOT(addCustomPresence()));
}

void customPresenceDialog::addCustomPresence()
{
    int presenceIndex = m_statusMessage->currentIndex();
    QString uniquePresenceString = m_statusMessage->currentText() + "_" + QString::number(m_statusMessage->currentIndex());
    new QListWidgetItem(iconForIndex(presenceIndex), m_statusMessage->currentText(), m_listWidget);
    m_presenceGroup->writeEntry(uniquePresenceString, m_statusMessage->currentIndex());
    m_presenceGroup->sync();
    emit configChanged();
}

void customPresenceDialog::removeCustomPresence()
{
   int index = indexForIcon(KIcon(m_listWidget->currentItem()->icon()));
   if(index == -1) {
     return;
   } else { 
      m_presenceGroup->deleteEntry(m_listWidget->currentItem()->text() + "_" + QString::number(index));
      m_presenceGroup->sync();
      emit configChanged();;
      delete m_listWidget->currentItem();
   }
}

KIcon customPresenceDialog::iconForIndex(int index)
{
    QString iconName;

    switch (index) {
        case 0:
            iconName = QLatin1String("user-online");
            break;
        case 1:
            iconName = QLatin1String("user-busy");
            break;
        case 2:
            iconName = QLatin1String("user-away");
            break;
    }

    return KIcon(iconName);
}

int customPresenceDialog::indexForIcon(KIcon icon)
{
    if (icon.name() == QLatin1String("user-online")) {
      return 0;
    }
    else if (icon.name() == QLatin1String("user-busy")) {
      return 1;
    }
    else if (icon.name() == QLatin1String("user-away")) {
      return 2;
    }
    return -1;
}

#include "custom-presence-dialog.moc"
