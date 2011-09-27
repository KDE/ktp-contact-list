#ifndef GLOBALPRESENCECHOOSER_H
#define GLOBALPRESENCECHOOSER_H

#include <KComboBox>

#include <TelepathyQt4/AccountManager>

class QFocusEvent;
class QMouseEvent;
class GlobalPresence;

class GlobalPresenceChooser : public KComboBox
{
    Q_OBJECT
public:
    explicit GlobalPresenceChooser(QWidget *parent = 0);
    void setAccountManager(const Tp::AccountManagerPtr &accountManager);

protected:
    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void focusOutEvent(QFocusEvent *event);

private slots:
    void onCurrentIndexChanged(int index);
    void onPresenceChanged(const Tp::Presence &presence);
    void onPresenceMessageChanged(const QString &message);

private:
    GlobalPresence *m_globalPresence;
};

#endif // GLOBALPRESENCECHOOSER_H
