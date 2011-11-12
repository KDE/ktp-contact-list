/*
 * Global Presence - A Drop down menu for selecting presence
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

#include "global-presence-chooser.h"

#include "presence-model.h"

#include "common/global-presence.h"
#include "dialogs/custom-presence-dialog.h"

#include <KIcon>
#include <KLocale>
#include <KLineEdit>
#include <KDebug>
#include <KPixmapSequence>
#include <KPixmapSequenceOverlayPainter>

#include <TelepathyQt4/Presence>
#include <TelepathyQt4/Account>

#include <QMouseEvent>
#include <QtGui/QToolTip>
#include <KMessageBox>


//A sneaky class that adds an extra entry to the end of the presence model
//called "Configure Presences"
class PresenceModelExtended : public QAbstractListModel
{
    Q_OBJECT
public:
    PresenceModelExtended(PresenceModel *presenceModel, QObject *parent);
    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QModelIndex addTemporaryPresence(const KPresence &presence);
    void removeTemporaryPresence();
private slots:
    void sourceRowsInserted(const QModelIndex &index, int start, int end);
    void sourceRowsRemoved(const QModelIndex &index, int start, int end);
private:
    KPresence m_temporaryPresence;
    PresenceModel *m_model;
};

PresenceModelExtended::PresenceModelExtended(PresenceModel *presenceModel, QObject *parent) :
    QAbstractListModel(parent),
    m_model(presenceModel)
{
    connect(m_model, SIGNAL(rowsInserted(QModelIndex, int, int)), SLOT(sourceRowsInserted(QModelIndex,int,int)));
    connect(m_model, SIGNAL(rowsRemoved(QModelIndex, int, int)), SLOT(sourceRowsRemoved(QModelIndex,int,int)));
}

//return number of rows + an extra item for the "configure presences" button
int PresenceModelExtended::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    int rowCount = m_model->rowCount(parent) + 2;
    if (m_temporaryPresence.isValid()) {
        rowCount++;
    }
    return rowCount;
}

QVariant PresenceModelExtended::data(const QModelIndex &index, int role) const
{
    if (index.row() == rowCount(index.parent())-1) {
        switch(role) {
        case Qt::DisplayRole:
            return i18n("Configure Custom Presences...");
        case Qt::DecorationRole:
            return KIcon("configure");
        }
    } else if (index.row() == rowCount(index.parent())-2) {
        switch(role) {
            case Qt::DisplayRole:
                return i18n("Now listening to...");
            case Qt::DecorationRole:
                return KIcon("speaker");
        }
    } else if (m_temporaryPresence.isValid() && index.row() == rowCount(index.parent()) -3) {
        switch(role) {
        case Qt::DisplayRole:
            return m_temporaryPresence.statusMessage();
        case Qt::DecorationRole:
            return m_temporaryPresence.icon();
        case PresenceModel::PresenceRole:
            return QVariant::fromValue<Tp::Presence>(m_temporaryPresence);
        }
    }
    else {
        return m_model->data(m_model->index(index.row()), role);
    }
    return QVariant();
}

void PresenceModelExtended::sourceRowsInserted(const QModelIndex &index, int start, int end)
{
    beginInsertRows(createIndex(index.row(), 0), start, end);
    endInsertRows();
}

void PresenceModelExtended::sourceRowsRemoved(const QModelIndex &index, int start, int end)
{
    beginRemoveRows(createIndex(index.row(), 0), start, end);
    endRemoveRows();
}

QModelIndex PresenceModelExtended::addTemporaryPresence(const KPresence &presence)
{
    int row = m_model->rowCount(QModelIndex());
    beginInsertRows(QModelIndex(),row, row);
    m_temporaryPresence = presence;
    endInsertRows();
    return this->createIndex(row, 0);
}

void PresenceModelExtended::removeTemporaryPresence()
{
    int row = m_model->rowCount(QModelIndex());
    beginRemoveRows(QModelIndex(),row, row);
    m_temporaryPresence = KPresence();
    endRemoveRows();
}

//----------------------------------------------------------------------------------------------------------

GlobalPresenceChooser::GlobalPresenceChooser(QWidget *parent) :
    KComboBox(parent),
    m_globalPresence(new GlobalPresence(this)),
    m_model(new PresenceModel(this)),
    m_modelExtended(new PresenceModelExtended(m_model, this))
{
    this->setModel(m_modelExtended);

    m_busyOverlay = new KPixmapSequenceOverlayPainter(this);
    m_busyOverlay->setSequence(KPixmapSequence("process-working"));
    m_busyOverlay->setWidget(this);

    connect(this, SIGNAL(activated(int)), SLOT(onCurrentIndexChanged(int)));
    connect(m_globalPresence, SIGNAL(currentPresenceChanged(Tp::Presence)), SLOT(onPresenceChanged(Tp::Presence)));
    connect(m_globalPresence, SIGNAL(changingPresence(bool)), SLOT(onPresenceChanging(bool)));

    onPresenceChanged(m_globalPresence->currentPresence());
}

void GlobalPresenceChooser::setAccountManager(const Tp::AccountManagerPtr &accountManager)
{
    m_accountManager = accountManager;
    m_globalPresence->setAccountManager(accountManager);
}

bool GlobalPresenceChooser::event(QEvent *e)
{
    if (e->type() == QEvent::ToolTip) {
        if (m_accountManager.isNull()) {
            return false;
        }

        QHelpEvent *helpEvent = static_cast<QHelpEvent *>(e);

        QString toolTipText;
        toolTipText.append("<table>");

        Q_FOREACH(const Tp::AccountPtr &account, m_accountManager->allAccounts()) {
            if (account->isEnabled()) {
                KPresence accountPresence(account->currentPresence());
                QString presenceIconPath = KIconLoader::global()->iconPath(accountPresence.icon().name(), 1);
                QString presenceIconString = QString::fromLatin1("<img src=\"%1\">").arg(presenceIconPath);
                QString accountIconPath = KIconLoader::global()->iconPath(account->iconName(), 1);
                QString accountIconString = QString::fromLatin1("<img src=\"%1\">").arg(accountIconPath);
                QString presenceString = accountPresence.displayString();
                toolTipText.append(QString::fromLatin1("<tr><td>%1 %2</td></tr><tr><td style=\"padding-left: 24px\">%3&nbsp;%4</td></tr>").arg(accountIconString, account->displayName(), presenceIconString, presenceString));
            }
        }

        toolTipText.append("</table>");
        QToolTip::showText(helpEvent->globalPos(), toolTipText, this);
        return true;
    }

    if (e->type() == QEvent::Resize) {
        repositionSpinner();
    }
    return QComboBox::event(e);
}

void GlobalPresenceChooser::onCurrentIndexChanged(int index)
{
    //if they select the "configure item"
    if (index == count()-1) {
        CustomPresenceDialog dialog(m_model, this);
        dialog.exec();
        onPresenceChanged(m_globalPresence->currentPresence());
    } else if (index == count()-2) {
        KSharedConfigPtr config = KSharedConfig::openConfig(QLatin1String("ktelepathyrc"));
        KConfigGroup kdedConfig = config->group("KDED");

        bool pluginEnabled = kdedConfig.readEntry("nowPlayingEnabled", false);

        if (!pluginEnabled) {
            if (KMessageBox::questionYesNo(this,
                i18n("This plugin is currently disabled. Do you want to enable it and use as your presence?"),
                     i18n("Plugin disabled")) == KMessageBox::Yes) {

                    kdedConfig.writeEntry("nowPlayingEnabled", true);
                    kdedConfig.sync();

                    QDBusMessage message = QDBusMessage::createSignal(QLatin1String("/Telepathy"),
                                                                      QLatin1String( "org.kde.Telepathy"),
                                                                      QLatin1String("settingsChange"));
                                                                      QDBusConnection::sessionBus().send(message);
            } else {
                onPresenceChanged(m_globalPresence->currentPresence());
                return;
            }
        }

        QDBusMessage message = QDBusMessage::createSignal(QLatin1String("/Telepathy"),
                                                          QLatin1String( "org.kde.Telepathy"),
                                                          QLatin1String("activateNowPlaying"));
        QDBusConnection::sessionBus().send(message);
    } else {
        QDBusMessage message = QDBusMessage::createSignal(QLatin1String("/Telepathy"),
                                                          QLatin1String( "org.kde.Telepathy"),
                                                          QLatin1String("deactivateNowPlaying"));
        QDBusConnection::sessionBus().send(message);
        Tp::Presence presence = itemData(index, PresenceModel::PresenceRole).value<Tp::Presence>();
        m_globalPresence->setPresence(presence);
    }
}

void GlobalPresenceChooser::onPresenceChanged(const Tp::Presence &presence)
{
    m_modelExtended->removeTemporaryPresence();
    kDebug();
    for (int i=0; i < count() ; i++) {
        Tp::Presence itemPresence = itemData(i, PresenceModel::PresenceRole).value<Tp::Presence>();
        if (itemPresence.type() == presence.type() && itemPresence.statusMessage() == presence.statusMessage()) {
            setCurrentIndex(i);
            return;
        }
    }

    QModelIndex index = m_modelExtended->addTemporaryPresence(presence);
    setCurrentIndex(index.row());
}

void GlobalPresenceChooser::onPresenceChanging(bool isChanging)
{
    if (isChanging) {
        repositionSpinner();
        m_busyOverlay->start();
    } else {
        m_busyOverlay->stop();
    }
}

void GlobalPresenceChooser::repositionSpinner()
{
    QPoint topLeft(width() - m_busyOverlay->sequence().frameSize().width() - 22,
                   (height() - m_busyOverlay->sequence().frameSize().height())/2);
    m_busyOverlay->setRect(QRect(topLeft, m_busyOverlay->sequence().frameSize()));
}


#include "global-presence-chooser.moc"
#include "moc_global-presence-chooser.cpp" //hack because we have two QObejcts in teh same file
