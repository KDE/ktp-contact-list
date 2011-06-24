#include "contact-info.h"
#include "ui_contact-info.h"

ContactInfo::ContactInfo(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ContactInfo)
{
    ui->setupUi(this);
}

ContactInfo::~ContactInfo()
{
    delete ui;
}
