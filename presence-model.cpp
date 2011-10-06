#include "presence-model.h"

#include <QFont>

#include <KIcon>
#include <KLocalizedString>

PresenceModel::PresenceModel(QObject *parent) :
    QAbstractListModel(parent)
{
    loadDefaultPresences();
}

QVariant PresenceModel::data(const QModelIndex &index, int role) const
{
    KPresence presence = m_presences[index.row()];
    switch (role) {
    case Qt::DisplayRole:
        if (presence.statusMessage().isEmpty()) {
            switch (presence.type()) {
            case Tp::ConnectionPresenceTypeAvailable:
                return i18n("Available");
            case Tp::ConnectionPresenceTypeBusy:
                return i18n("Busy");
            case Tp::ConnectionPresenceTypeAway:
                return i18n("Away");
            case Tp::ConnectionPresenceTypeExtendedAway:
                return i18n("Extended Away");
            case Tp::ConnectionPresenceTypeHidden:
                return i18n("Invisible");
            case Tp::ConnectionPresenceTypeOffline:
                return i18n("Offline");
            default:
                return i18n("Unknown");
            }
        } else {
            return presence.statusMessage();
        }

    case Qt::DecorationRole:
        return presence.icon();

    case Qt::FontRole:
        if (presence.statusMessage().isEmpty()) {
            QFont font;
            font.setBold(true);
            return font;
        }

    case PresenceModel::PresenceRole:
        return QVariant::fromValue<Tp::Presence>(presence);

    }

    return QVariant();
}

int PresenceModel::rowCount(const QModelIndex &parent) const
{
    return m_presences.size();
}

void PresenceModel::loadDefaultPresences()
{
    addPresence(Tp::Presence::available());
    addPresence(Tp::Presence::busy());
    addPresence(Tp::Presence::away());
    addPresence(Tp::Presence::away("Back Soon!"));
    addPresence(Tp::Presence::away("Off to eat some cheese"));
    addPresence(Tp::Presence::xa());
    addPresence(Tp::Presence::xa("Abducted by aliens, back later"));
    addPresence(Tp::Presence::hidden());
    addPresence(Tp::Presence::offline());

    addPresence(Tp::Presence::available("Configure Custom Messages..."));
}

QModelIndex PresenceModel::addPresence(const Tp::Presence &presence)
{
    qDebug() << presence.status();
    qDebug() << presence.statusMessage();


    beginInsertRows(QModelIndex(), m_presences.size(), m_presences.size());
    m_presences.append(presence);
    endInsertRows();

    return createIndex(m_presences.size(), 0);
}
