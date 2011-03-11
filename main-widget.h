/*
 * This file is part of telepathy-contactslist-prototype
 *
 * Copyright (C) 2009-2010 Collabora Ltd. <info@collabora.co.uk>
 *   @Author George Goldberg <george.goldberg@collabora.co.uk>
 *   @Author Martin Klapetek <martin.klapetek@gmail.com>
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

#ifndef TELEPATHY_CONTACTSLIST_MAIN_WIDGET_H
#define TELEPATHY_CONTACTSLIST_MAIN_WIDGET_H

#include <QtGui/QWidget>
#include <QtGui/QStyledItemDelegate>

#include <TelepathyQt4/AccountManager>

#include "ui_main-widget.h"

class KMenu;
class KSelectAction;
class AccountsModel;
class AccountFilterModel;
class ContactDelegate;
class FilterBar;

class MainWidget : public QWidget, Ui::MainWidget
{
    Q_OBJECT
public:
    MainWidget(QWidget *parent = 0);
    ~MainWidget();

    enum SystemMessageType {
        /*
         * this will show a system message to the user
         * but it will fade after short timout,
         * thus it should be used for non-important messages
         * like "Connecting..." etc.
         */ 
        SystemMessageInfo,

        /*
         * message with this class will stay visible until user
         * closes it and will have light-red background
         */
        SystemMessageError
    };

public Q_SLOTS:
    void onAccountManagerReady(Tp::PendingOperation *op);
    void onChannelJoined(Tp::PendingOperation *op);
    void startTextChannel(const QModelIndex &index);
    void onContactListDoubleClick(const QModelIndex &index);
    void onConnectionChanged(const Tp::ConnectionPtr &connection);
    void onAccountReady(Tp::PendingOperation *op);
    void onAccountConnectionStatusChanged(Tp::ConnectionStatus status);
    void showMessageToUser(const QString &text, const SystemMessageType type);
    void addOverlayButtons();
    void onNewAccountAdded(const Tp::AccountPtr &account);
    void onAccountStateChanged(bool enabled); 
    void onAccountRemoved();
    void toggleSearchWidget(bool show);
    //    void startAudioChannel();
    //    void startVideoChannel();

    void onCustomContextMenuRequested(const QPoint &point);

private:
    AccountsModel          *m_model;
    AccountFilterModel     *m_modelFilter;
    Tp::AccountManagerPtr   m_accountManager;
    KMenu                  *m_accountMenu;
    KSelectAction          *m_setStatusAction;
    ContactDelegate        *m_delegate;
};


#endif // Header guard