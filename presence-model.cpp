#include "presence-model.h"

#include <QFont>

#include <KIcon>
#include <KLocalizedString>

#include <KConfig>
#include <KConfigGroup>



PresenceModel::PresenceModel(QObject *parent) :
    QAbstractListModel(parent)
{
    loadDefaultPresences();

    KSharedConfigPtr config = KSharedConfig::openConfig("telepathy-kde-contactlistrc");
    m_presenceGroup = new KConfigGroup( config, "Custom Presence List" );
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

    addPresence(Tp::Presence::xa());
    addPresence(Tp::Presence::xa("Abducted by aliens, back later"));


    addPresence(Tp::Presence::away());
    addPresence(Tp::Presence::away("Back Soon!"));
    addPresence(Tp::Presence::away("Off to eat some cheese"));


    addPresence(Tp::Presence::hidden());
    addPresence(Tp::Presence::offline());


    //FIXME FIXME FIXIME this is just a hack!
    addPresence(Tp::Presence::offline("Configure Custom Messages..."));
}

QModelIndex PresenceModel::addPresence(const Tp::Presence &presence)
{
    QList<KPresence>::iterator i = qLowerBound(m_presences.begin(), m_presences.end(), KPresence(presence));

    m_presences.insert(i, presence);
    int index = m_presences.indexOf(presence);
    //this is technically a backwards and wrong, but I can't get a row from a const iterator, and using qLowerBound does seem a good approach
    beginInsertRows(QModelIndex(), index, index);
    endInsertRows();

    //save changes
    m_presenceGroup->writeEntry("presences", presence);

    return createIndex(index, 0);
}

void PresenceModel::removePresence(const Tp::Presence &presence)
{
    int row = m_presences.indexOf(presence);
    beginRemoveRows(QModelIndex(), row, row);
    m_presences.removeOne(presence);
    endRemoveRows();

    //FIXME edit the config file too
}
