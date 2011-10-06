#ifndef GLOBALPRESENCECHOOSER_H
#define GLOBALPRESENCECHOOSER_H

#include <KComboBox>

#include <TelepathyQt4/AccountManager>

class QFocusEvent;
class QMouseEvent;
class GlobalPresence;
class PresenceModel;

class GlobalPresenceChooser : public KComboBox
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
    PresenceModel *m_model;
};

#endif // GLOBALPRESENCECHOOSER_H
