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

#include <Nepomuk/ResourceManager>

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
    KAboutData aboutData("telepathy-contactslist-prototype", 0, ki18n("Telepathy Nepomuk Enabled Contact List Prototype"), "0.1");

    KCmdLineArgs::init(argc, argv, &aboutData);
    KApplication app;

    Nepomuk::ResourceManager::instance()->init();

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

