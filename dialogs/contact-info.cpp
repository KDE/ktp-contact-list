#include "contact-info.h"
#include "ui_contact-info.h"

#include <TelepathyQt4/AvatarData>
#include <TelepathyQt4/Presence>

#include <QtGui/QPixmap>

#include <KProtocolInfo>

ContactInfo::ContactInfo(Tp::ContactPtr contact, QWidget *parent) :
    KDialog(parent),
    ui(new Ui::ContactInfo)
{
    QWidget* widget = new QWidget(this);
    setMainWidget(widget);
    ui->setupUi(widget);

    setWindowTitle(contact->alias());

    setButtons(KDialog::Close);

    QPixmap avatar(contact->avatarData().fileName);

    ui->avatarLabel->setPixmap(avatar.scaled(ui->avatarLabel->maximumSize(), Qt::KeepAspectRatio));

    ui->idLabel->setText(contact->id());
    ui->nameLabel->setText(contact->alias());

    QString presenceMessage = contact->presence().statusMessage();

    //find links in presence message
    QRegExp linkRegExp("\\b(\\w+)://[^ \t\n\r\f\v]+");
    int index = 0;
    while ((index = linkRegExp.indexIn(presenceMessage, index)) != -1) {
        QString realUrl = linkRegExp.cap(0);
        QString protocol = linkRegExp.cap(1);
        if (KProtocolInfo::protocols().contains(protocol, Qt::CaseInsensitive)) {
            QString link = "<a href='" + realUrl + "'>" + realUrl + "</a>";
            presenceMessage.replace(index, realUrl.length(), link);
            index += link.length();
        }
        else {
            index += realUrl.length();
        }
    }

    ui->presenceLabel->setTextFormat(Qt::RichText);
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
