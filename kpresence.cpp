#include "kpresence.h"

KPresence::KPresence() :
    Tp::Presence()
{

}

KPresence::KPresence(const Tp::Presence &presence) :
    Tp::Presence(presence)
{
}

KIcon KPresence::icon()
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

