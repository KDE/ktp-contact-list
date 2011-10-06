#include "kpresence.h"

KPresence::KPresence() :
    Tp::Presence()
{
}

KPresence::KPresence(const Tp::Presence &presence) :
    Tp::Presence(presence)
{
}

KIcon KPresence::icon() const
{
    switch (type()) {
    case Tp::ConnectionPresenceTypeAvailable:
        return KIcon(QLatin1String("user-online"));
    case Tp::ConnectionPresenceTypeBusy:
        return KIcon("user-busy");
    case Tp::ConnectionPresenceTypeAway:
        return KIcon("user-away");
    case Tp::ConnectionPresenceTypeExtendedAway:
        return KIcon("user-away-extended");
    case Tp::ConnectionPresenceTypeHidden:
        return KIcon("user-invisible");
    case Tp::ConnectionPresenceTypeOffline:
        return KIcon("user-offline");
    default:
        return KIcon();
    }
}

bool KPresence::operator <(const KPresence &other) const
{
    /// Sets the sorting order of presences
    QHash<uint, int> m_presenceSorting;

    m_presenceSorting[Tp::ConnectionPresenceTypeAvailable] = 0;
    m_presenceSorting[Tp::ConnectionPresenceTypeBusy] = 1;
    m_presenceSorting[Tp::ConnectionPresenceTypeHidden] = 2;
    m_presenceSorting[Tp::ConnectionPresenceTypeAway] = 3;
    m_presenceSorting[Tp::ConnectionPresenceTypeExtendedAway] = 4;
    m_presenceSorting[Tp::ConnectionPresenceTypeHidden] = 5;
    //don't distinguish between the following three presences
    m_presenceSorting[Tp::ConnectionPresenceTypeError] = 6;
    m_presenceSorting[Tp::ConnectionPresenceTypeUnknown] = 6;
    m_presenceSorting[Tp::ConnectionPresenceTypeUnset] = 6;
    m_presenceSorting[Tp::ConnectionPresenceTypeOffline] = 7;

    if (m_presenceSorting[type()] < m_presenceSorting[other.type()]) {
        return true;
    } else if (m_presenceSorting[type()] == m_presenceSorting[other.type()]) {
        return (statusMessage() < other.statusMessage());
    } else {
        return false;
    }
}

