/*
 * This file is part of Telepathy Contact List
 *
 * Copyright (C) 2009-2010 Collabora Ltd. <info@collabora.co.uk>
 *   @Author George Goldberg <george.goldberg@collabora.co.uk>
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

#include "main-widget.h"

#include <KAboutData>
#include <KCmdLineArgs>
#include <KDebug>
#include <KUniqueApplication>

#include <TelepathyQt/Types>
#include <TelepathyQt/Debug>

#include "contact-list-application.h"
#include "version.h"

int main(int argc, char *argv[])
{
    KAboutData aboutData("ktp-contactlist", 0, ki18n("KDE Telepathy Contact List"), KTP_CONTACT_LIST_VERSION,
                         ki18n("KDE Telepathy Contact List"), KAboutData::License_GPL,
                         ki18n("(C) 2011, Martin Klapetek"));

    aboutData.addAuthor(ki18nc("@info:credit", "Martin Klapetek"), ki18n("Developer"),
                        "martin.klapetek@gmail.com");
    aboutData.setProductName("telepathy/contactlist"); //set the correct name for bug reporting
    aboutData.setProgramIconName("telepathy-kde");

    KCmdLineArgs::init(argc, argv, &aboutData);

    // Add --debug as commandline option
    KCmdLineOptions options;
    options.add("debug", ki18n("Show Telepathy debugging information"));
    KCmdLineArgs::addCmdLineOptions(options);

    ContactListApplication app;

    Tp::registerTypes();
    Tp::enableDebug(KCmdLineArgs::parsedArgs()->isSet("debug"));
    Tp::enableWarnings(true);

    // Create the main widget and show it.
    MainWidget *mainWidget = new MainWidget(0);
    mainWidget->show();

    // Start event loop.
    app.exec();
}

