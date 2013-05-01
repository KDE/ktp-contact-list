/*
 * Contact Tooltip
 *
 * Copyright (C) 2011 David Edmundson <kde@davidedmundson.co.uk>
 * Copyright (C) 2011 Geoffry Song <goffrie@gmail.com>
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

#include "persontooltip.h"

#include "ui_persontooltip.h"
#include "ktooltip.h"

#include <KTp/types.h>
#include <KTp/text-parser.h>
#include <KTp/presence.h>

#include <QDesktopServices>
#include <QTextDocument>

#include <KToolInvocation>
#include <KDebug>


bool contactLessThan(const QVariant &left, const QVariant &right)
{
    KTp::ContactPtr c1 = left.value<KTp::ContactPtr>();
    KTp::ContactPtr c2 = right.value<KTp::ContactPtr>();

    if (c1.isNull() && !c2.isNull()) {
        return true;
    }

    if (!c1.isNull() && c2.isNull()) {
        return false;
    }

    if (c1.isNull() && c2.isNull()) {
        return false;
    }

    if (!c1.isNull() && !c2.isNull()) {
        return c1->presence() < c2->presence();
    }

    return false;
}

//-----------------------------------------------------------------------------

PersonToolTip::PersonToolTip(const QModelIndex &index) :
    QWidget(0),
    ui(new Ui::PersonToolTip)
{
    ui->setupUi(this);
    ui->nameLabel->setText(index.data(Qt::DisplayRole).toString());
    ui->avatarLabel->setScaledContents(false);
    ui->avatarLabel->setAlignment(Qt::AlignCenter);
    ui->contactsLabel->setText(i18n("Contacts"));
    ui->contactsWidget->setLayout(new QGridLayout(ui->contactsWidget));
    qobject_cast<QGridLayout*>(ui->contactsWidget->layout())->setColumnStretch(1, 500);

    QVariantList avatarsList = index.data(KTp::ContactAvatarPathRole).toList();
    if (avatarsList.isEmpty()) {
        ui->avatarLabel->setPixmap(KIconLoader::global()->loadIcon("im-user", KIconLoader::NoGroup, 96));
    } else {
        QPixmap avatarPixmap(avatarsList.first().toUrl().toLocalFile());
        ui->avatarLabel->setPixmap(avatarPixmap.scaled(ui->avatarLabel->size(), Qt::KeepAspectRatio));
    }

    KTp::Presence presence(Tp::Presence((Tp::ConnectionPresenceType)index.data(KTp::ContactPresenceTypeRole).toUInt(),
                                        QString(), //the presence name is not needed, saves one call to the model
                                        index.data(KTp::ContactPresenceMessageRole).toString()));

    QString presenceMessage = presence.statusMessage();
    QString presenceText = presence.displayString();

    if (index.data(KTp::ContactPresenceTypeRole).toInt() == Tp::ConnectionPresenceTypeError) {
        presenceText = i18nc("This is an IM user status", "Error Getting Presence");

        /** if the presence is error, the message might containt server's error,
        *   so let's print it out and unset it so it won't be displayed to user
        */
        kDebug() << presenceMessage;
        presenceMessage.clear();
    }

    int smallIconSize = KIconLoader::global()->currentSize(KIconLoader::Small);

    ui->presenceIcon->setPixmap(presence.icon().pixmap(smallIconSize, smallIconSize));
    ui->presenceLabel->setText(presenceText);
    ui->presenceMessageLabel->setText(presenceMessage);

    int row = 0;

    QVariantList contacts = index.data(KTp::ContactRole).toList();

    qSort(contacts.begin(), contacts.end(), contactLessThan);

    Q_FOREACH (const QVariant &contactVariant, contacts) {
        const KTp::ContactPtr contact = contactVariant.value<KTp::ContactPtr>();

        if (!contact.isNull()) {
            QLabel *contactPresenceLabel = new QLabel(ui->contactsWidget);
            contactPresenceLabel->setPixmap(contact->presence().icon().pixmap(KIconLoader::SizeSmall, KIconLoader::SizeSmall));

            QLabel *contactIdLabel = new QLabel(ui->contactsWidget);
            contactIdLabel->setText(contact->id());

            qobject_cast<QGridLayout*>(ui->contactsWidget->layout())->addWidget(contactPresenceLabel, row, 0);
            qobject_cast<QGridLayout*>(ui->contactsWidget->layout())->addWidget(contactIdLabel, row, 1);

            row++;
        }
    }

    connect(ui->presenceMessageLabel, SIGNAL(linkActivated(QString)), this, SLOT(openLink(QString)));

    ui->blockedLabel->setShown(index.data(KTp::ContactIsBlockedRole).toBool());
}

PersonToolTip::~PersonToolTip()
{
    delete ui;
}

void PersonToolTip::openLink(QString url)
{
    KToolInvocation::invokeBrowser(url);
    KToolTip::hideTip();
}

QString PersonToolTip::getTextWithHyperlinks(QString text)
{
    KTp::TextUrlData urls = KTp::TextParser::instance()->extractUrlData(text);
    QString result;
    int position = 0;

    for (int i = 0; i < urls.fixedUrls.size(); ++i) {
        QPair<int, int> pair = urls.urlRanges[i];
        QString displayLink = text.mid(pair.first, pair.second);
        QString fixedLink = urls.fixedUrls[i];

        if (pair.first > position) {
            result += Qt::escape(text.mid(position, pair.first - position));
        }

        result += QString("<a href=\"%1\">%2</a>").arg(Qt::escape(fixedLink)).arg(Qt::escape(displayLink));
        position = pair.first + pair.second;
    }

    if (position < text.length()) {
        result += Qt::escape(text.mid(position));
    }

    return result;
}
