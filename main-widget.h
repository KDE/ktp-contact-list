/*
 * This file is part of telepathy-contact-list
 *
 * Copyright (C) 2009-2010 Collabora Ltd. <info@collabora.co.uk>
 *   @Author George Goldberg <george.goldberg@collabora.co.uk>
 * Copyright (C) 2011 Martin Klapetek <martin.klapetek@gmail.com>
 * Copyright (C) 2011 Keith Rusler <xzekecomax@gmail.com>
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

#include <QWidget>
#include <QStyledItemDelegate>

#include <TelepathyQt/Types>

#include <KXmlGuiWindow>
#include <QAction>
#include <KDualAction>
#include "ui_main-widget.h"

class ContextMenu;
class ContactDelegateCompact;
class GroupsModel;
class QMenu;
class KMenuBar;
class KSelectAction;
class AccountsModel;
class AccountsFilterModel;
class ContactDelegate;
class FilterBar;
class KJob;

class MainWidget : public KMainWindow, Ui::MainWindow
{
    Q_OBJECT
    //this is needed otherwise this class is exported as .ktp-contactlist interface
    //and dashes are not allowed in dbus interface names, so this interface would not work otherwise
    Q_CLASSINFO("D-Bus Interface", "org.kde.KTp.ContactList")
public:
    MainWidget(QWidget *parent = 0);
    ~MainWidget();

    bool isPresencePlasmoidPresent() const;

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
    void showMessageToUser(const QString &text, const SystemMessageType type);
    Q_INVOKABLE void toggleWindowVisibility();

private Q_SLOTS:
    void toggleSearchWidget(bool show);
    void clearSearch();
    void onAccountManagerReady(Tp::PendingOperation *op);
    void onAddContactRequest();
    void onStartChatRequest();
    void onJoinChatRoomRequested();                 /** join chat room action is triggered */
    void onMakeCallRequested();
    void onMergeContactsDialogRequested();
    void onCustomContextMenuRequested(const QPoint &point);
    void onGenericOperationFinished(Tp::PendingOperation *operation);   /** called when a Tp::PendingOperation finishes. Used to check for errors */
    void onContactSelectionChanged();
    void onMetacontactToggleTriggered();
    void onModelInitialized(bool success);

private:
    QStringList extractLinksFromIndex(const QModelIndex &index);    /** extract links from a QModelIndex pointing to a contact */
    ///Was moved to telepathy-kded-module
    //void handleConnectionError(const Tp::AccountPtr &account);      /** handle connection errors for given account. This method provides visual notification */
    void closeEvent(QCloseEvent *e);
    QAction *createAction(const QString &text, QObject *signalReceiver, const char *slot, const QIcon &icon);
    QAction *createAction(const QString &text, QObject *signalReceiver, const char *slot, bool isChecked, const QIcon &icon);
    void setupActions(const KConfigGroup&);
    void setupGlobalMenu();
    void setupToolBar();
    void setupTelepathy();

    QMenuBar               *m_globalMenu;
    QAction                *m_settingsDialog;
    QAction                *m_joinChatRoom;
    QAction                *m_makeCall;
    QAction                *m_mergeContacts;
    QActionGroup           *m_contactListTypeGroup;
    QActionGroup           *m_blockedFilterGroup;

    QMenu                  *m_accountMenu;
    KSelectAction          *m_setStatusAction;

    KDualAction            *m_metacontactToggleAction;
    QAction                *m_addContactAction;
    QActionGroup           *m_groupContactsActionGroup;
    KDualAction            *m_showOfflineAction;
    QAction                *m_searchContactAction;
    QAction                *m_startChatAction;
    KDualAction            *m_sortByPresenceAction;
    QAction                *m_quitAction;

    Tp::AccountManagerPtr  m_accountManager;

    ContextMenu            *m_contextMenu;
};


#endif // Header guard
