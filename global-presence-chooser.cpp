#include "global-presence-chooser.h"

#include "global-presence.h"
#include "presence-model.h"

#include <KIcon>
#include <KLocale>
#include <KLineEdit>
#include <KDebug>

#include <TelepathyQt4/Presence>

#include <QMouseEvent>

GlobalPresenceChooser::GlobalPresenceChooser(QWidget *parent) :
    KComboBox(parent),
    m_globalPresence(new GlobalPresence(this)),
    m_model(new PresenceModel(this))
{
    this->setModel(m_model);

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
    Tp::Presence presence = itemData(index, PresenceModel::PresenceRole).value<Tp::Presence>();
    m_globalPresence->setPresence(presence);
}

void GlobalPresenceChooser::onPresenceChanged(const Tp::Presence &presence)
{
    kDebug() << "presence changing";
    for (int i=0; i < count() ; i++) {
        Tp::Presence itemPresence = itemData(i, PresenceModel::PresenceRole).value<Tp::Presence>();
        if (itemPresence.type() == presence.type() && itemPresence.statusMessage() == presence.statusMessage()) {
            setCurrentIndex(i);
            kDebug() << "found item";
            return;
        }
    }

    //FIXME this needs to only be a temporary presence, which we delete afterwards
    QModelIndex index = m_model->addPresence(presence);
    setCurrentIndex(index.row());
}

