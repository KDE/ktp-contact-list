#ifndef PRESENCEMODEL_H
#define PRESENCEMODEL_H

#include "kpresence.h"

#include <QAbstractListModel>

#include <KConfigGroup>

class PresenceModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit PresenceModel(QObject *parent = 0);
    ~PresenceModel();

    enum Roles {
        //Also supplies Qt::DisplayRole and Qt::DecorationRole
        PresenceRole = Qt::UserRole
    };

    /** Adds a custom presence to the model, and write value to config file.
      @return the newly added item
    */
    QModelIndex addPresence(const Tp::Presence &presence);

    void removePresence(const Tp::Presence &presence);

    /** Returns the index of a given presence, adding it if needed*/
    QModelIndex indexOf(const Tp::Presence &presence);

protected:
    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual int rowCount(const QModelIndex &parent) const;

signals:

public slots:

private:

    /** Loads standard presences (online, away etc) into */
    void loadDefaultPresences();

    /** Loads any user custom presences into the model*/
    void loadCustomPresences();


    QList<KPresence> m_presences;

    //this is wrong, KConfigGroup is a sharedptr..
    KConfigGroup m_presenceGroup;
};

#endif // PRESENCEMODEL_H
