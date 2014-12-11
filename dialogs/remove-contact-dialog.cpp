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

#include <KLocalizedString>

#include <QCheckBox>
#include <QLabel>
#include <QDialogButtonBox>

#include <TelepathyQt/AvatarData>
#include <TelepathyQt/Contact>
#include <TelepathyQt/ContactManager>



RemoveContactDialog::RemoveContactDialog(Tp::ContactPtr contact, QWidget* parent)
    : QDialog(parent, Qt::Dialog)
    , ui(new Ui::RemoveContactDialog)
{
    QWidget *removeDialog = new QWidget(this);

    ui->setupUi(removeDialog);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(removeDialog);
    mainLayout->addWidget(buttonBox);


    ui->textLabel->setText(i18n("Remove the selected contact?"));
    ui->contactAliasLabel->setText(contact->alias());

    ui->blockCheckbox->setEnabled(contact->manager()->canBlockContacts());

    // load contact avatar
    if (contact->avatarData().fileName.isEmpty()) {
        ui->contactAvatarLabel->setPixmap(QIcon::fromTheme("im-user").pixmap(QSize(90, 90)));
    } else {
        ui->contactAvatarLabel->setPixmap(QPixmap(contact->avatarData().fileName).scaled(QSize(128, 128), Qt::KeepAspectRatio));
    }
}

bool RemoveContactDialog::blockContact() const
{
    return ui->blockCheckbox->isChecked();
}
