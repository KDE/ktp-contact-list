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

#include "contacttooltip.h"

#include "ui_contacttooltip.h"
#include "ktooltip.h"

#include <KTp/types.h>
#include <KTp/text-parser.h>
#include <KTp/presence.h>

#include <QDesktopServices>
#include <QTextDocument>

#include <KToolInvocation>
#include <KDebug>

ContactToolTip::ContactToolTip(const QModelIndex &index) :
    QWidget(0),
    ui(new Ui::ContactToolTip)
{
    ui->setupUi(this);
    ui->nameLabel->setText(index.data(Qt::DisplayRole).toString());
    ui->idLabel->setText(index.data(KTp::IdRole).toString());
    ui->avatarLabel->setScaledContents(false);
    ui->avatarLabel->setAlignment(Qt::AlignCenter);

    QString avatar = index.data(KTp::ContactAvatarPathRole).toUrl().toLocalFile();
    if (avatar.isEmpty()) {
        ui->avatarLabel->setPixmap(KIconLoader::global()->loadIcon("im-user", KIconLoader::NoGroup, 96));
    } else {
        QPixmap avatarPixmap(avatar);
        ui->avatarLabel->setPixmap(avatarPixmap.scaled(ui->avatarLabel->size(), Qt::KeepAspectRatio));
    }

    QString presenceMessage = index.data(KTp::ContactPresenceMessageRole).toString();
    QString presenceIconPath = index.data(KTp::ContactPresenceIconRole).toString();
    QString presenceText = index.data(KTp::ContactPresenceNameRole).toString();

    if (index.data(KTp::ContactPresenceTypeRole).toInt() == Tp::ConnectionPresenceTypeError) {
        presenceIconPath = KIconLoader::global()->iconPath("task-attention", 1);
        presenceText = i18nc("This is an IM user status", "Error Getting Presence");

        /** if the presence is error, the message might containt server's error,
        *   so let's print it out and unset it so it won't be displayed to user
        */
        kDebug() << presenceMessage;
        presenceMessage.clear();
    }

    ui->presenceIcon->setPixmap(QPixmap(presenceIconPath));
    ui->presenceLabel->setText(presenceText);
    ui->presenceMessageLabel->setText(presenceMessage);
    ui->blockedLabel->setShown(index.data(KTp::ContactIsBlockedRole).toBool());

    const Tp::AccountPtr account = index.data(KTp::AccountRole).value<Tp::AccountPtr>();
    if (!account.isNull()) {
        ui->accountLabel->setText(i18n("Account: %1").arg(account->displayName()));
    }

    connect(ui->presenceMessageLabel, SIGNAL(linkActivated(QString)), this, SLOT(openLink(QString)));
}

ContactToolTip::~ContactToolTip()
{
    delete ui;
}

void ContactToolTip::openLink(QString url)
{
    KToolInvocation::invokeBrowser(url);
    KToolTip::hideTip();
}

QString ContactToolTip::getTextWithHyperlinks(QString text)
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
