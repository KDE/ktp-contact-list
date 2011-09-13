/*
 * Dialog for showing contact info
 *
 * Copyright (C) 2011 David Edmundson <kde@davidedmundson.co.uk>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "contact-info.h"
#include "ui_contact-info.h"

#include <TelepathyQt4/AvatarData>
#include <TelepathyQt4/Presence>

#include <QtGui/QPixmap>

#include <KProtocolInfo>

ContactInfo::ContactInfo(const Tp::ContactPtr &contact, QWidget *parent) :
    KDialog(parent),
    ui(new Ui::ContactInfo)
{
    QWidget *widget = new QWidget(this);
    setMainWidget(widget);
    ui->setupUi(widget);

    setWindowTitle(contact->alias());

    setButtons(KDialog::Close);

    QPixmap avatar(contact->avatarData().fileName);
    if (avatar.isNull()) {
        avatar = KIconLoader::global()->loadIcon("im-user", KIconLoader::Desktop, 128);
    }

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
        } else {
            index += realUrl.length();
        }
    }

    ui->presenceLabel->setTextFormat(Qt::RichText);
    ui->presenceLabel->setText(presenceMessage);

    KIcon blockedIcon;
    if (contact->isBlocked()) {
        blockedIcon = KIcon("task-complete");
    } else {
        blockedIcon = KIcon("task-reject");
    }
    ui->blockedLabel->setPixmap(blockedIcon.pixmap(16));

    ui->subscriptionStateLabel->setPixmap(iconForPresenceState(contact->subscriptionState()).pixmap(16));
    ui->publishStateLabel->setPixmap(iconForPresenceState(contact->publishState()).pixmap(16));
}

ContactInfo::~ContactInfo()
{
    delete ui;
}

KIcon ContactInfo::iconForPresenceState(Tp::Contact::PresenceState state) const
{
    switch (state) {
    case Tp::Contact::PresenceStateYes:
        return KIcon("task-complete");
    case Tp::Contact::PresenceStateNo:
        return KIcon("task-reject");
    case Tp::Contact::PresenceStateAsk:
        /* Drop Through*/
    default:
        return KIcon("task-attempt");
    }
}
