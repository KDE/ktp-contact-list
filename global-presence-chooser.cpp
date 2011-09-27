#include "global-presence-chooser.h"

#include "global-presence.h"

#include <KIcon>
#include <KLocale>
#include <KLineEdit>

#include <TelepathyQt4/Presence>
#include <QMouseEvent>

GlobalPresenceChooser::GlobalPresenceChooser(QWidget *parent) :
    KComboBox(parent),
    m_globalPresence(new GlobalPresence(this))
{
    setInsertPolicy(NoInsert);
    setTrapReturnKey(true);
    addItem(KIcon("user-online"), i18n("Available"), qVariantFromValue(Tp::Presence::available()));
    addItem(KIcon("user-away"), i18n("Away"), qVariantFromValue(Tp::Presence::away()));
    addItem(KIcon("user-away"), i18n("Be Right Back"), qVariantFromValue(Tp::Presence::brb()));
    addItem(KIcon("user-busy"), i18n("Busy"), qVariantFromValue(Tp::Presence::busy()));
    addItem(KIcon("user-busy"), i18n("Do Not Disturb"), qVariantFromValue(Tp::Presence(
                                                                            Tp::ConnectionPresenceTypeBusy,
                                                                              QLatin1String("dnd"),
                                                                              QLatin1String(""))));
    addItem(KIcon("user-away-extended"), i18n("Extended Away"), qVariantFromValue(Tp::Presence::xa()));
    addItem(KIcon("user-invisible"), i18n("Invisible"), qVariantFromValue(Tp::Presence::hidden()));
    addItem(KIcon("user-offline"), i18n("Offline"), qVariantFromValue(Tp::Presence::offline()));

    connect(this, SIGNAL(activated(int)), SLOT(onCurrentIndexChanged(int)));
    connect(m_globalPresence, SIGNAL(currentPresenceChanged(Tp::Presence)), SLOT(onPresenceChanged(Tp::Presence)));
    connect(this, SIGNAL(returnPressed(QString)), SLOT(onPresenceMessageChanged(QString)));
}

void GlobalPresenceChooser::setAccountManager(const Tp::AccountManagerPtr &accountManager)
{
    m_globalPresence->setAccountManager(accountManager);
}

void GlobalPresenceChooser::onCurrentIndexChanged(int index)
{
    Tp::Presence presence = itemData(index).value<Tp::Presence>();
    m_globalPresence->setPresence(presence);
    setEditable(true);
    lineEdit()->selectAll();
    lineEdit()->setFocus();
}

void GlobalPresenceChooser::onPresenceChanged(const Tp::Presence &presence)
{
    qDebug() << "presence changing";
    for (int i=0; i < count() ; i++) {
        Tp::Presence itemPresence = itemData(i).value<Tp::Presence>();
        if (itemPresence.type() == presence.type() && itemPresence.status() == presence.status()) {
            setCurrentIndex(i);
            qDebug() << "found item";
        }
    }

    //FIXME if we can't find the correct value, create an entry.
}

// void GlobalPresenceChooser::enterEvent(QEvent* event)
// {
// }
//
// void GlobalPresenceChooser::leaveEvent(QEvent* event)
// {
// }

void GlobalPresenceChooser::onPresenceMessageChanged(const QString &message)
{
    Tp::Presence presence = m_globalPresence->currentPresence();
    presence.setStatus(presence.type(), presence.status(), message);

    bool presenceExists = false;
    int currentPresenceIndex;
    for (int i=0; i < count() ; i++) {
        Tp::Presence itemPresence = itemData(i).value<Tp::Presence>();
        if (itemPresence.type() == presence.type()) {
            currentPresenceIndex = i;
        }
        if (itemPresence.type() == presence.type() && itemPresence.status() == presence.status() && itemPresence.statusMessage() == presence.statusMessage()) {
            qDebug() << "This presence already exists, setting this one instead";
            setCurrentIndex(i);
            presenceExists = true;
            break;
        }
    }

    if (!presenceExists) {
        qDebug() << "Adding new presence";
        insertItem(currentPresenceIndex, itemIcon(currentIndex()), message, qVariantFromValue(presence));
    }
}

void GlobalPresenceChooser::mousePressEvent(QMouseEvent *event)
{
    if (!isEditable()) {
        setEditable(true);
    }

    KComboBox::mousePressEvent(event);
}

void GlobalPresenceChooser::focusOutEvent(QFocusEvent* event)
{
    setEditable(false);
}
