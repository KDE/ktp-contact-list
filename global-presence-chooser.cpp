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

#include "dialogs/custom-presence-dialog.h"
#include "dialogs/advanced-presence-dialog.h"

#include <KTp/presence.h>
#include <KTp/global-presence.h>
#include <KTp/Models/presence-model.h>

#include <KLocalizedString>
#include <KSharedConfig>
#include <KLineEdit>
#include <KPixmapSequence>
#include <KPixmapSequenceOverlayPainter>
#include <KMessageBox>
#include <KIconLoader>

#include <TelepathyQt/Account>

#include <QFontDatabase>
#include <QMouseEvent>
#include <QToolTip>
#include <QStyle>
#include <QPushButton>
#include <QMenu>
#include <QPointer>

extern const QString KDED_STATUS_MESSAGE_PARSER_WHATSTHIS(
        i18n("<p>Tokens can be used wherever a status message can be set to create a dynamic status message.</p>")
        + i18n("<p><strong>%tr+&lt;val&gt;</strong>: Countdown to 0 from <strong>&lt;val&gt;</strong> minutes. e.g. %tr+30</p>")
        + i18n("<p><strong>%time+[&lt;val&gt;]</strong>: The current local time, or if a value is specified, the local time <strong>&lt;val&gt;</strong> minutes in the future. e.g. %time+10</p>")
        + i18n("<p><strong>%utc+[&lt;val&gt;]</strong>: The current UTC time, or if a value is specified, the UTC time <strong>&lt;val&gt;</strong> minutes into the future. e.g. %utc</p>")
        + i18n("<p><strong>%te+[&lt;val&gt;]</strong>: Time elapsed from message activation. Append an initial elapsed time &quot;&lt;val&gt;&quot; in minutes. e.g. %te+5</p>")
        + i18n("<p><strong>%title</strong>: Now Playing track title.</p>")
        + i18n("<p><strong>%artist</strong>: Now Playing track or album artist.</p>")
        + i18n("<p><strong>%album</strong>: Now Playing album.</p>")
        + i18n("<p><strong>%track</strong>: Now Playing track number.</p>")
        + i18n("<p><strong>%um+[&lt;val&gt;]</strong>: When specified globally or in an account presence status message, overrides all automatic presence messages. When specified in an automatic presence status message, is substituted for the global or account presence status message (if specified). When <strong>val = g</strong> in an account presence status message or an automatic presence status message, overrides the account presence status message or automatic presence status message with the global presence status message. e.g. %um, %um+g</p>")
        + i18n("<p><strong>%tu+&lt;val&gt;</strong>: Refresh the status message every <strong>&lt;val&gt;</strong> minutes. e.g. %tu+2</p>")
        + i18n("<p><strong>%tx+&lt;val&gt;</strong>: Expire the status message after <strong>&lt;val&gt;</strong> minutes, or when the Now Playing active player stops (<strong>val = np</strong>). e.g. %tx+20, %tx+np</p>")
        + i18n("<p><strong>%xm+&quot;&lt;val&gt;&quot;</strong>: Specify a message to follow %tr, %time, %utc, and %tx token expiry. e.g. %xm+&quot;Back %time. %tx+3 %xm+&quot;Running late&quot;&quot;</p>")
        + i18n("<p><strong>%tf+&quot;&lt;val&gt;&quot;</strong>: Specify the format for local time using QDateTime::toString() expressions. e.g. %tf+&quot;h:mm AP t&quot;</p>")
        + i18n("<p><strong>%uf+&quot;&lt;val&gt;&quot;</strong>: Specify the format for UTC time using QDateTime::toString() expressions. e.g. %uf+&quot;hh:mm t&quot;</p>")
        + i18n("<p><strong>%sp+&quot;&lt;val&gt;&quot;</strong>: Change the separator for empty fields. e.g. %sp+&quot;-&quot;</p>")
        + i18n("<p>Using tokens requires the Telepathy KDED module to be loaded. Tokens can be escaped by prepending a backslash character, e.g. &#92;%sp</p>")
        );

//A sneaky class that adds an extra entries to the end of the presence model.
class PresenceModelExtended : public QAbstractListModel
{
    Q_OBJECT
public:
    PresenceModelExtended(KTp::PresenceModel *presenceModel, QObject *parent);
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    KTp::Presence temporaryPresence() const;
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

PresenceModelExtended::PresenceModelExtended(KTp::PresenceModel *presenceModel, QObject *parent) :
    QAbstractListModel(parent),
    m_model(presenceModel)
{
    connect(m_model, &QAbstractItemModel::rowsInserted, this, &PresenceModelExtended::sourceRowsInserted);
    connect(m_model, &QAbstractItemModel::rowsRemoved, this, &PresenceModelExtended::sourceRowsRemoved);
}

//return number of rows + the extra items added to end of list
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
    if (role == Qt::SizeHintRole) {
        const QFontMetrics fontMetrics(QFontDatabase::systemFont(QFontDatabase::GeneralFont));
        return QSize(0, qMax(fontMetrics.height(), (int)(KIconLoader::SizeSmall)) + 8);
    }
    if (index.row() == rowCount() - 1) {
        switch (role) {
        case Qt::DisplayRole:
            return i18n("Configure Custom Presences...");
        case Qt::DecorationRole:
            return QIcon::fromTheme("configure");
        }
    } else if (index.row() == rowCount() - 2) {
        switch (role) {
        case Qt::DisplayRole:
            return i18n("Advanced Presence Setting...");
        case Qt::DecorationRole:
            return QIcon::fromTheme("configure");
        }
    } else if (m_temporaryPresence.isValid() && index.row() == rowCount() - 3) {
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

KTp::Presence PresenceModelExtended::temporaryPresence() const
{
    return m_temporaryPresence;
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

QModelIndex PresenceModelExtended::addTemporaryPresence(const KTp::Presence &presence)
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

void PresenceModelExtended::removeTemporaryPresence()
{
    if (!m_temporaryPresence.isValid()) {
        return; //if not already set, do nothing.
    }

    int row = m_model->rowCount();
    beginRemoveRows(QModelIndex(), row, row);
    m_temporaryPresence = KTp::Presence();
    endRemoveRows();
}

//----------------------------------------------------------------------------------------------------------

GlobalPresenceChooser::GlobalPresenceChooser(QWidget *parent) :
    KComboBox(parent),
    m_globalPresence(new KTp::GlobalPresence(this)),
    m_model(new KTp::PresenceModel(this)),
    m_modelExtended(new PresenceModelExtended(m_model, this)),
    m_busyOverlay(new KPixmapSequenceOverlayPainter(this)),
    m_changePresenceMessageButton(new QPushButton(this))
{
    this->setModel(m_modelExtended);

    m_busyOverlay->setSequence(KIconLoader::global()->loadPixmapSequence("process-working", KIconLoader::SizeSmallMedium));
    setEditable(false);

    m_changePresenceMessageButton->setIcon(QIcon::fromTheme("document-edit"));
    m_changePresenceMessageButton->setFlat(true);
    m_changePresenceMessageButton->setToolTip(i18n("Click to change your presence message"));

    connect(this, static_cast<void(GlobalPresenceChooser::*)(int)>(&KComboBox::currentIndexChanged), &GlobalPresenceChooser::onAllComboChanges);
    connect(this, static_cast<void(GlobalPresenceChooser::*)(int)>(&KComboBox::activated), &GlobalPresenceChooser::onUserActivatedComboChange);
    connect(m_globalPresence, &KTp::GlobalPresence::currentPresenceChanged, this, &GlobalPresenceChooser::onPresenceChanged);
    connect(m_globalPresence, &KTp::GlobalPresence::connectionStatusChanged, this, &GlobalPresenceChooser::onConnectionStatusChanged);
    connect(m_changePresenceMessageButton, &QPushButton::clicked, this, &GlobalPresenceChooser::onChangePresenceMessageClicked);

    onPresenceChanged(m_globalPresence->currentPresence());
    //we need to check if there is some account connecting and if so, spin the spinner
    onConnectionStatusChanged(m_globalPresence->connectionStatus());
}

bool GlobalPresenceChooser::event(QEvent *e)
{
    if (e->type() == QEvent::ToolTip) {
        QHelpEvent *helpEvent = static_cast<QHelpEvent *>(e);

        QString toolTipText;

        if (isEditable()) {
            toolTipText = KDED_STATUS_MESSAGE_PARSER_WHATSTHIS;
        } else {
            if (m_globalPresence->accountManager().isNull()) {
                return false;
            }

            toolTipText.append("<table>");

            for (const Tp::AccountPtr &account : m_globalPresence->accountManager()->enabledAccounts()->accounts()) {
                KTp::Presence accountPresence(account->currentPresence());
                QString presenceIconPath = KIconLoader::global()->iconPath(accountPresence.icon().name(), 1);
                QString presenceIconString = QString::fromLatin1("<img src=\"%1\">").arg(presenceIconPath);
                QString accountIconPath = KIconLoader::global()->iconPath(account->iconName(), 1);
                QString accountIconString = QString::fromLatin1("<img src=\"%1\" width=\"%2\" height=\"%2\">").arg(accountIconPath).arg(KIconLoader::SizeSmallMedium);
                QString presenceString;
                if (account->connectionStatus() == Tp::ConnectionStatusConnecting) {
                    presenceString = i18nc("Presence string when the account is connecting", "Connecting...");
                } else if (!account->currentPresence().statusMessage().isEmpty()){
                    presenceString = QString::fromLatin1("(%1) ").arg(accountPresence.displayString()) + accountPresence.statusMessage();
                } else {
                    presenceString = accountPresence.displayString();
                }
                toolTipText.append(QString::fromLatin1("<tr><td>%1 %2</td></tr><tr><td style=\"padding-left: 24px\">%3&nbsp;%4</td></tr>").arg(accountIconString, account->displayName(), presenceIconString, presenceString));
            }

            toolTipText.append("</table>");
        }

        QToolTip::showText(helpEvent->globalPos(), toolTipText, this);
        return true;
    }

    if (e->type() == QEvent::Resize) {
        repositionOverlays();
    }

    if (e->type() == QEvent::ContextMenu) {
        if (isEditable()) {
            m_lineEditContextMenu = lineEdit()->createStandardContextMenu();

            return true;
        }
    }

    if (e->type() == QEvent::KeyPress) {
        QKeyEvent *ke = static_cast<QKeyEvent*>(e);

        if (ke->key() == Qt::Key_Enter || ke->key() == Qt::Key_Return) {
            if (isEditable()) {
                onConfirmPresenceMessageClicked();
                return true;
            }
        }
        if (ke->key() == Qt::Key_Escape) {
            if (isEditable()) {
                setEditable(false);
                m_changePresenceMessageButton->show();
            }
        }
        if (ke->key() == Qt::Key_Down || ke->key() == Qt::Key_Up) {
            if (!isEditable()) {
                showPopup();
                return true;
            }
        }
    }

    if (e->type() == QEvent::FocusOut) {
        //just cancel editable and let it exec parent event()
        if (!m_lineEditContextMenu.isNull()) {
            if (!m_lineEditContextMenu.data()->isHidden()) {
                //if we're showing the context menu, do not process this event further
                return true;
            }
            //...otherwise delete the menu and hide the lineedit
            m_lineEditContextMenu.data()->deleteLater();
        }
        if (isEditable()) {
            setEditable(false);
            m_changePresenceMessageButton->show();
        }
    }

    return KComboBox::event(e); // krazy:exclude=qclasses
}

void GlobalPresenceChooser::setEditable(bool editable)
{
    if (editable) {
        m_busyOverlay->setWidget(0);
    } else {
        m_busyOverlay->setWidget(this);
        if (m_globalPresence->connectionStatus() == KTp::GlobalPresence::Connecting) {
            m_busyOverlay->start(); // If telepathy is still connecting, overlay must be spinning again.
        }
    }
    KComboBox::setEditable(editable);
}

void GlobalPresenceChooser::onUserActivatedComboChange(int index)
{
    if (index == -1) {
        return;
    }

    if (index == count() - 2) {
        QPointer<AdvancedPresenceDialog> dialog = new AdvancedPresenceDialog(m_model, m_globalPresence, this);
        dialog.data()->exec();
        delete dialog.data();
    } else if (index == count() - 1) {
        QPointer<CustomPresenceDialog> dialog = new CustomPresenceDialog(m_model, this);
        dialog.data()->exec();
        delete dialog.data();
    } else {
        KTp::Presence presence = itemData(index, KTp::PresenceModel::PresenceRole).value<KTp::Presence>();
        m_globalPresence->setPresence(presence);
    }

    onPresenceChanged(m_globalPresence->currentPresence());
}

void GlobalPresenceChooser::onAllComboChanges(int index)
{
    int lastPresenceIndex = m_model->rowCount();
    if (index < lastPresenceIndex) {
        KTp::Presence presence = itemData(index, KTp::PresenceModel::PresenceRole).value<KTp::Presence>();
        if ((presence.type() == Tp::ConnectionPresenceTypeOffline) ||
                (presence.type() == Tp::ConnectionPresenceTypeHidden)) {
            m_changePresenceMessageButton->hide();
        } else {
            m_changePresenceMessageButton->show();
        }
    }

    clearFocus();
}

void GlobalPresenceChooser::onPresenceChanged(const KTp::Presence &presence)
{
    if (presence.type() == Tp::ConnectionPresenceTypeUnset) {
        setCurrentIndex(-1);
        m_busyOverlay->start();
        return;
    }

    const QModelIndexList &matchIndexList = m_model->match(m_model->index(0, 0), KTp::PresenceModel::PresenceRole, QVariant::fromValue<KTp::Presence>(presence));
    if (!matchIndexList.isEmpty()) {
        m_modelExtended->removeTemporaryPresence();
        setCurrentIndex(matchIndexList.at(0).row());
    } else {
        const QModelIndex &index = m_modelExtended->addTemporaryPresence(presence);
        setCurrentIndex(index.row());
    }

    m_busyOverlay->stop();
}

void GlobalPresenceChooser::onConnectionStatusChanged(KTp::GlobalPresence::ConnectionStatus connectionStatus)
{
    if (connectionStatus == KTp::GlobalPresence::Connecting) {
        repositionOverlays();
        m_busyOverlay->start();
    } else {
        m_busyOverlay->stop();
    }
}

void GlobalPresenceChooser::repositionOverlays()
{
    //set 2px margins so that the button is not bigger than the combo
    m_changePresenceMessageButton->setMaximumHeight(height() - 2);
    m_changePresenceMessageButton->setMaximumWidth(height() - 2);
    QPoint topLeft;
    if (m_changePresenceMessageButton->layoutDirection() == Qt::RightToLeft) {
        //move the button 22px from the left edge
        m_changePresenceMessageButton->move(22, 0);
        //place the spinner 2px right from the button, 4 is added to take the margin.
        topLeft.setX(m_changePresenceMessageButton->pos().x() + m_busyOverlay->sequence().frameSize().width() + 4);
        topLeft.setY((height() - m_busyOverlay->sequence().frameSize().height()) / 2);
    } else {
        //move the button 22px from the right edge
        m_changePresenceMessageButton->move(width() - m_changePresenceMessageButton->width() - 22, 0);
        //place the spinner 2px left from the button
        topLeft.setX(m_changePresenceMessageButton->pos().x() - m_busyOverlay->sequence().frameSize().width() - 2);
        topLeft.setY((height() - m_busyOverlay->sequence().frameSize().height()) / 2);
    }
    m_busyOverlay->setRect(QRect(topLeft, m_busyOverlay->sequence().frameSize()));
}

void GlobalPresenceChooser::onChangePresenceMessageClicked()
{
    m_changePresenceMessageButton->hide();

    setEditable(true);

    //if current presence has no presence message, delete the text
    if (m_globalPresence->globalPresence().statusMessage().isEmpty()) {
        lineEdit()->clear();
    } else {
        lineEdit()->setText(m_globalPresence->globalPresence().statusMessage());
    }

    lineEdit()->setFocus();
}

void GlobalPresenceChooser::onConfirmPresenceMessageClicked()
{
    m_changePresenceMessageButton->show();
    KTp::Presence presence = itemData(currentIndex(), KTp::PresenceModel::PresenceRole).value<KTp::Presence>();
    presence.setStatusMessage(lineEdit()->text());

    setEditable(false);

    m_globalPresence->setPresence(presence);
}


#include "global-presence-chooser.moc"
#include "moc_global-presence-chooser.cpp" //hack because we have two QObjects in the same file
