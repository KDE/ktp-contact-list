#include "add-contact-dialog.h"
#include "ui_add-contact-dialog.h"

#include "accounts-model.h"
#include "accounts-model-item.h"

#include <QObject>
#include <QSortFilterProxyModel>
#include <QDebug>


#include <TelepathyQt4/Account>
#include <TelepathyQt4/Connection>
#include <TelepathyQt4/ContactManager>

/** A filter which only lists connections which accept adding contacts*/
class SubscribableAccountsModel : public QSortFilterProxyModel
{
public:
    SubscribableAccountsModel(QObject *parent);
    virtual bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;
};

SubscribableAccountsModel::SubscribableAccountsModel(QObject *parent)
 : QSortFilterProxyModel(parent)
{
}

bool SubscribableAccountsModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    AccountsModelItem* item = sourceModel()->index(source_row, 0, source_parent).data(AccountsModel::ItemRole).value<AccountsModelItem*>();

    if (item) {
        Tp::AccountPtr account = item->account();

        //if there's no connection we can't add contacts as we have no contactmanager
        if (! account->isOnline()) {
            return false;
        }

        //only show items which can add a contact (i.e hide accounts like IRC which are online but there's no point listing)
        if (! account->connection()->contactManager()->canRequestPresenceSubscription()){
            return false;
        }
    }
    return true;
}


AddContactDialog::AddContactDialog(AccountsModel *accountModel, QWidget *parent) :
    KDialog(parent),
    ui(new Ui::AddContactDialog)
{
    QWidget *widget = new QWidget(this);
    ui->setupUi(widget);
    setMainWidget(widget);

    SubscribableAccountsModel *filteredModel = new SubscribableAccountsModel(this);
    filteredModel->setSourceModel(accountModel);
    ui->accountCombo->setModel(filteredModel);
}

AddContactDialog::~AddContactDialog()
{
    delete ui;
}

Tp::AccountPtr AddContactDialog::account() const
{
    QVariant itemData = ui->accountCombo->itemData(ui->accountCombo->currentIndex(),AccountsModel::ItemRole);
    AccountsModelItem* item = itemData.value<AccountsModelItem*>();
    if (item) {
        return item->account();
    } else {
        return Tp::AccountPtr();
    }
}

const QString AddContactDialog::screenName() const
{
    return ui->screenNameLineEdit->text();
}
