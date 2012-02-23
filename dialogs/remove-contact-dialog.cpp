/*
 * This file is part of telepathy-contactslist
 *
 * Copyright (C) 2011 by Francesco Nwokeka <francesco.nwokeka@gmail.com>
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

#include "remove-contact-dialog.h"
#include "ui_remove-contact-dialog.h"

#include <KIcon>
#include <KLocalizedString>

#include <QtGui/QCheckBox>
#include <QtGui/QLabel>

#include <TelepathyQt/AvatarData>

RemoveContactDialog::RemoveContactDialog(Tp::ContactPtr contact, QWidget* parent)
    : KDialog(parent, Qt::Dialog)
    , ui(new Ui::RemoveContactDialog)
{
    QWidget *removeDialog = new QWidget(this);

    ui->setupUi(removeDialog);
    setMainWidget(removeDialog);

    ui->textLabel->setText(i18n("Remove the selected contact?"));
    ui->contactAliasLabel->setText(contact->alias());

    // load contact avatar
    if (contact->avatarData().fileName.isEmpty()) {
        KIcon defaultIcon("im-user");       // load KIcon with the desired pixmap
        ui->contactAvatarLabel->setPixmap(defaultIcon.pixmap(QSize(90, 90)));
    } else {
        ui->contactAvatarLabel->setPixmap(QPixmap(contact->avatarData().fileName).scaled(QSize(128, 128), Qt::KeepAspectRatio));
    }
}

bool RemoveContactDialog::blockContact() const
{
    return ui->blockCheckbox->isChecked();
}

#include "remove-contact-dialog.moc"
