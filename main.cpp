/*
 * This file is part of telepathy-contactslist-prototype
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

extern "C"
{
#include <signal.h>
}

#include "main-widget.h"

#include <KAboutData>
#include <KCmdLineArgs>
#include <KDebug>
#include <KApplication>

#include <TelepathyQt4/Types>
#include <TelepathyQt4/Debug>

namespace
{
    static void signal_handler(int signal)
    {
        if ((signal == SIGTERM) || (signal == SIGINT)) {
            QCoreApplication * const app(QCoreApplication::instance());
            if (app != 0) {
                kDebug() << "Signal Handler Called. Exiting...";
                app->quit();
            }
        }
    }
}

int main(int argc, char *argv[])
{
    KAboutData aboutData("telepathy-kde-contactslist", 0, ki18n("Telepathy KDE Contact List"), "0.1",
                         ki18n("Telepathy KDE Contact List"), KAboutData::License_GPL,
                         ki18n("(C) 2011, Martin Klapetek"));

    aboutData.addAuthor(ki18nc("@info:credit", "Martin Klapetek"), KLocalizedString(),
                        "martin.klapetek@gmail.com");

    KCmdLineArgs::init(argc, argv, &aboutData);

    // Add --debug as commandline option
    KCmdLineOptions options;
    options.add("debug", ki18n("Show telepathy debbuging information"));
    KCmdLineArgs::addCmdLineOptions(options);

    KApplication app;

    Tp::registerTypes();
    Tp::enableDebug(KCmdLineArgs::parsedArgs()->isSet("debug"));
    Tp::enableWarnings(true);

    // Set up signal handlers.
    if (signal(SIGINT, signal_handler) == SIG_ERR) {
        kWarning() << "Setting up SIGINT signal handler failed.";
    }

    if (signal(SIGTERM, signal_handler) == SIG_ERR) {
        kWarning() << "Setting up SIGTERM signal handler failed.";
    }

    // Create the main widget and show it.
    MainWidget *mainWidget = new MainWidget(0);
    mainWidget->show();

    // Start event loop.
    app.exec();
}

