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
#include <KLocalizedString>
#include <KDBusService>

#include <TelepathyQt/Types>
#include <TelepathyQt/Debug>

#include <QApplication>
#include <QCommandLineParser>

#include "version.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    KLocalizedString::setApplicationDomain("ktp-contactlist");

    KAboutData aboutData("ktpcontactlist", i18n("KDE Telepathy Contact List"), KTP_CONTACT_LIST_VERSION,
                         i18n("KDE Telepathy Contact List"), KAboutLicense::GPL,
                         i18n("(C) 2011, Martin Klapetek"));

    aboutData.addAuthor(i18nc("@info:credit", "Martin Klapetek"), i18n("Developer"),
                        "martin.klapetek@gmail.com");
    aboutData.setProductName("telepathy/contactlist"); //set the correct name for bug reporting
    QGuiApplication::setWindowIcon(QIcon::fromTheme("telepathy-kde"));
    KAboutData::setApplicationData(aboutData);

    KDBusService service(KDBusService::Unique);
    {
        QCommandLineParser parser;
        // Add --debug as commandline option
        parser.addOption(QCommandLineOption("debug", i18n("Show Telepathy debugging information")));

        aboutData.setupCommandLine(&parser);
        parser.addHelpOption();
        parser.addVersionOption();
        parser.process(app);
        aboutData.processCommandLine(&parser);

        Tp::registerTypes();
        Tp::enableDebug(parser.isSet("debug"));
        Tp::enableWarnings(true);
    }

    // Create the main widget and show it.
    MainWidget *mainWidget = new MainWidget(0);
    mainWidget->show();

    // Start event loop.
    app.exec();
}

