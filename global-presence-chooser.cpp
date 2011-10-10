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

#include "global-presence.h"
#include "presence-model.h"
#include "dialogs/custom-presence-dialog.h"

#include <KIcon>
#include <KLocale>
#include <KLineEdit>
#include <KDebug>

#include <TelepathyQt4/Presence>

#include <QMouseEvent>

//A sneaky class that adds an extra entry to the end of the presence model
//called "Configure Presences"
class PresenceModelPlusConfig : public QAbstractListModel
{
    Q_OBJECT
public:
    PresenceModelPlusConfig(PresenceModel *presenceModel, QObject *parent);
    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
private slots:
    void sourceRowsInserted(const QModelIndex &index, int start, int end);
    void sourceRowsRemoved(const QModelIndex &index, int start, int end);
private:
    PresenceModel *m_model;
};

PresenceModelPlusConfig::PresenceModelPlusConfig(PresenceModel *presenceModel, QObject *parent) :
    QAbstractListModel(parent),
    m_model(presenceModel)
{
    connect(m_model, SIGNAL(rowsInserted(QModelIndex, int, int)), SLOT(sourceRowsInserted(QModelIndex,int,int)));
    connect(m_model, SIGNAL(rowsRemoved(QModelIndex, int, int)), SLOT(sourceRowsRemoved(QModelIndex,int,int)));
}

//return number of rows + an extra item for the "configure presences" button
int PresenceModelPlusConfig::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_model->rowCount(parent) + 1;
}

QVariant PresenceModelPlusConfig::data(const QModelIndex &index, int role) const
{
    if (index.row() == rowCount(index.parent())-1) {
        switch(role) {
        case Qt::DisplayRole:
            return i18n("Configure Custom Presences...");
        case Qt::DecorationRole:
            return KIcon("configure");
        }
    } else {
        return m_model->data(m_model->index(index.row()), role);
    }
    return QVariant();
}

void PresenceModelPlusConfig::sourceRowsInserted(const QModelIndex &index, int start, int end)
{
    beginInsertRows(createIndex(index.row(), 0), start, end);
    endInsertRows();
}

void PresenceModelPlusConfig::sourceRowsRemoved(const QModelIndex &index, int start, int end)
{
    beginRemoveRows(createIndex(index.row(), 0), start, end);
    endRemoveRows();
}


GlobalPresenceChooser::GlobalPresenceChooser(QWidget *parent) :
    KComboBox(parent),
    m_globalPresence(new GlobalPresence(this)),
    m_model(new PresenceModel(this))
{
    this->setModel(new PresenceModelPlusConfig(m_model, this));

    connect(this, SIGNAL(activated(int)), SLOT(onCurrentIndexChanged(int)));
    connect(m_globalPresence, SIGNAL(currentPresenceChanged(Tp::Presence)), SLOT(onPresenceChanged(Tp::Presence)));

    onPresenceChanged(m_globalPresence->currentPresence());
}

void GlobalPresenceChooser::setAccountManager(const Tp::AccountManagerPtr &accountManager)
{
    m_globalPresence->setAccountManager(accountManager);
}

void GlobalPresenceChooser::onCurrentIndexChanged(int index)
{
    //if they select the "configure item"
    if (index == count()-1) {
        CustomPresenceDialog dialog(m_model, this);
        dialog.exec();
        onPresenceChanged(m_globalPresence->currentPresence());
    } else {
        Tp::Presence presence = itemData(index, PresenceModel::PresenceRole).value<Tp::Presence>();
        m_globalPresence->setPresence(presence);
    }
}

void GlobalPresenceChooser::onPresenceChanged(const Tp::Presence &presence)
{
    kDebug();
    for (int i=0; i < count() ; i++) {
        Tp::Presence itemPresence = itemData(i, PresenceModel::PresenceRole).value<Tp::Presence>();
        if (itemPresence.type() == presence.type() && itemPresence.statusMessage() == presence.statusMessage()) {
            setCurrentIndex(i);
            return;
        }
    }

    //FIXME this needs to only be a temporary presence, which we delete afterwards
    QModelIndex index = m_model->addPresence(presence);
    setCurrentIndex(index.row());
}

#include "global-presence-chooser.moc"
#include "moc_global-presence-chooser.cpp" //hack because we have two QObejcts in teh same file
