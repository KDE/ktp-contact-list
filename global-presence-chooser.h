#ifndef GLOBALPRESENCECHOOSER_H
#define GLOBALPRESENCECHOOSER_H

#include <QComboBox>

#include <TelepathyQt4/AccountManager>

class GlobalPresence;

class GlobalPresenceChooser : public QComboBox
{
    Q_OBJECT
public:
    explicit GlobalPresenceChooser(QWidget *parent = 0);
    void setAccountManager(const Tp::AccountManagerPtr &accountManager);

private slots:
    void onCurrentIndexChanged(int index);
    void onPresenceChanged(const Tp::Presence &presence);

private:
    GlobalPresence *m_globalPresence;
};

#endif // GLOBALPRESENCECHOOSER_H
