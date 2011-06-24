#include "contact-info.h"
#include "ui_contact-info.h"

#include <TelepathyQt4/AvatarData>
#include <TelepathyQt4/Presence>


ContactInfo::ContactInfo(Tp::ContactPtr contact, QWidget *parent) :
    KDialog(parent),
    ui(new Ui::ContactInfo)
{
//    QWidget* widget = new QWidget(this);
    ui->setupUi(this);
//    setMainWidget(widget);
    setButtons(KDialog::Close);

    ui->idLabel->setText(contact->id());
    ui->nameLabel->setText(contact->alias());

    QString presenceMessage = contact->presence().statusMessage();
    ui->presenceLabel->setText(presenceMessage);


    QString blockedText;
    if (contact->isBlocked()) {
        blockedText = i18n("Yes");
    }
    else {
        blockedText = i18n("No");
    }
    ui->blockedLabel->setText(blockedText);

    QString presenceSubscriptionText;
    if (contact->subscriptionState() == Tp::Contact::PresenceStateYes) {
        presenceSubscriptionText = i18n("Yes");
    }
    else if (contact->subscriptionState() == Tp::Contact::PresenceStateNo){
        presenceSubscriptionText = i18n("No");
    }
    else {
        presenceSubscriptionText = i18n("Unknown");
    }
    ui->subscriptionStateLabel->setText(presenceSubscriptionText);


    QString presencePublicationText;
    if (contact->publishState() == Tp::Contact::PresenceStateYes) {
        presencePublicationText = i18n("Yes");
    }
    else if (contact->publishState() == Tp::Contact::PresenceStateNo){
        presencePublicationText = i18n("No");
    }
    else {
        presencePublicationText = i18n("Unknown");
    }
    ui->publishStateLabel->setText(presencePublicationText);


}

ContactInfo::~ContactInfo()
{
    delete ui;
}
