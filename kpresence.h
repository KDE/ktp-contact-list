#ifndef KPRESENCE_H
#define KPRESENCE_H

#include <TelepathyQt4/Presence>

#include <KIcon>

class KPresence : public Tp::Presence
{
public:
    KPresence();
    KPresence(const Tp::Presence &presence);
    KIcon icon() const;

    /** Returns which presence is "more available" */
    bool operator <(const KPresence &other) const;
};

#endif // KPRESENCE_H
