#ifndef KPRESENCE_H
#define KPRESENCE_H

#include <TelepathyQt4/Presence>

#include <KIcon>


class KPresence : public Tp::Presence
{
public:
    KPresence(const Tp::Presence &presence);
    KIcon icon();
};

#endif // KPRESENCE_H
