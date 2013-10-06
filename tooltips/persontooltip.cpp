/*
 * Person Tooltip
 *
 * Copyright (C) 2011 David Edmundson <kde@davidedmundson.co.uk>
 * Copyright (C) 2013 Martin Klapetek <mklapetek@kde.org>
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

bool contactLessThan(const QVariant &left, const QVariant &right)
{
    QModelIndex i1 = left.value<QModelIndex>();
    QModelIndex i2 = right.value<QModelIndex>();

    return KTp::Presence::sortPriority((Tp::ConnectionPresenceType)i1.data(KTp::ContactPresenceTypeRole).toInt())
        < KTp::Presence::sortPriority((Tp::ConnectionPresenceType)i2.data(KTp::ContactPresenceTypeRole).toInt());
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

    QPixmap avatarPixmap = index.data(KTp::ContactAvatarPixmapRole).value<QPixmap>();
    ui->avatarLabel->setPixmap(avatarPixmap.scaled(ui->avatarLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));

    int smallIconSize = KIconLoader::global()->currentSize(KIconLoader::Small);

    ui->presenceIcon->setPixmap(KIcon(index.data(KTp::ContactPresenceIconRole).toString()).pixmap(smallIconSize, smallIconSize));
    ui->presenceLabel->setText(index.data(KTp::ContactPresenceNameRole).toString());
    ui->presenceMessageLabel->setText(getTextWithHyperlinks(index.data(KTp::ContactPresenceMessageRole).toString()));

    //collect child indexes for sorting
    QList<QVariant> indexes;
    for (int i = 0; i < index.model()->rowCount(index); i++) {
        indexes << QVariant::fromValue<QModelIndex>(index.child(i, 0));
    }

    //sort indexes by presence
    qSort(indexes.begin(), indexes.end(), contactLessThan);

    //QLabel row counter
    int row = 0;

    Q_FOREACH (const QVariant &v, indexes)
    {
        QModelIndex i = v.value<QModelIndex>();
        QLabel *contactPresenceLabel = new QLabel(ui->contactsWidget);
        KIcon presenceIcon = KIcon(i.data(KTp::ContactPresenceIconRole).toString());
        contactPresenceLabel->setPixmap(presenceIcon.pixmap(KIconLoader::SizeSmall, KIconLoader::SizeSmall));

        QLabel *contactIdLabel = new QLabel(ui->contactsWidget);
        contactIdLabel->setText(i.data(KTp::IdRole).toString());

        qobject_cast<QGridLayout*>(ui->contactsWidget->layout())->addWidget(contactPresenceLabel, row, 0);
        qobject_cast<QGridLayout*>(ui->contactsWidget->layout())->addWidget(contactIdLabel, row, 1);

        row++;
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
