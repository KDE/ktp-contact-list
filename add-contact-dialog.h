#ifndef ADDCONTACTDIALOG_H
#define ADDCONTACTDIALOG_H

#include <KDialog>

#include <TelepathyQt4/Types>

namespace Ui {
    class AddContactDialog;
}

class AccountsModel;

class AddContactDialog : public KDialog
{
    Q_OBJECT

public:
    explicit AddContactDialog(AccountsModel* accountModel, QWidget *parent = 0);
    virtual ~AddContactDialog();
    Tp::AccountPtr account() const;
    const QString screenName() const;

private:
    Ui::AddContactDialog *ui;
};

#endif // ADDCONTACTDIALOG_H
