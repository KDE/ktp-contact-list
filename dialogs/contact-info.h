#ifndef CONTACTINFO_H
#define CONTACTINFO_H

#include <KDialog>
#include <TelepathyQt4/Contact>


namespace Ui {
    class ContactInfo;
}

class ContactInfo : public KDialog
{
    Q_OBJECT

public:
    explicit ContactInfo(Tp::ContactPtr contact, QWidget *parent = 0);
    ~ContactInfo();

private:
    Ui::ContactInfo *ui;
};

#endif // CONTACTINFO_H
