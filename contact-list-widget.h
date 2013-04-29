/*
 *  Contact List Widget
 *  Copyright (C) 2011  Martin Klapetek <martin.klapetek@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef CONTACT_LIST_WIDGET_H
#define CONTACT_LIST_WIDGET_H

#include <QTreeView>
#include <TelepathyQt/Types>
#include <TelepathyQt/Connection>

class ContactsModel;
class ContactListWidgetPrivate;

namespace Tp {
    class PendingOperation;
}

class ContactListWidget : public QTreeView
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(ContactListWidget)
    Q_DISABLE_COPY(ContactListWidget)

public:

    /** This should cover all the possible selection cases */
    enum SelectedItemType {
        NoSelection,                /// Nothing is selected
        NonGroupedContact,          /// Normal contact that does not belong to any Person
        GroupedContact,             /// Contact being a grounding occurance to some Person
        Person,                     /// Person item
        PersonAndNonGroupedContact, /// Person selected together with normal contact
        PersonAndGroupedContact,    /// Person selected with contact belonging to other Person
        NonAndGroupedContact        /// Normal contact and contact belonging to a Person selected
    };

    explicit ContactListWidget(QWidget* parent);
    virtual ~ContactListWidget();

    Tp::AccountManagerPtr accountManager() const;

public Q_SLOTS:
    void toggleOfflineContacts(bool show);
    void toggleGroups(bool show);
    void toggleSortByPresence(bool sort);

    void setFilterString(const QString &string);
    void showSettingsKCM();

    void onSwitchToFullView();
    void onSwitchToCompactView();
    void onSwitchToMiniView();

    void onShowAllContacts();
    void onShowUnblockedContacts();
    void onShowBlockedContacts();

    void onGroupSelectedContacts();
    void onUngroupSelectedContacts();

protected Q_SLOTS:
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

private Q_SLOTS:
    void onContactListClicked(const QModelIndex &index);
    void onContactListDoubleClicked(const QModelIndex &index);
    void onCustomContextMenuRequested(const QPoint &pos);

    void addOverlayButtons();

    void startTextChannel(const Tp::AccountPtr &account, const Tp::ContactPtr &contact);
    void startFileTransferChannel(const Tp::AccountPtr &account, const Tp::ContactPtr &contact);
    void startAudioChannel(const Tp::AccountPtr &account, const Tp::ContactPtr &contact);
    void startVideoChannel(const Tp::AccountPtr &account, const Tp::ContactPtr &contact);
    void startDesktopSharing(const Tp::AccountPtr &account, const Tp::ContactPtr &contact);
    void startLogViewer(const Tp::AccountPtr &account, const Tp::ContactPtr &contact);

    void accountEnablingFinished(Tp::PendingOperation *op);

Q_SIGNALS:
    void enableOverlays(bool);
    void accountManagerReady(Tp::PendingOperation* op);
    void genericOperationFinished(Tp::PendingOperation* op);

    void listSelectionChanged(ContactListWidget::SelectedItemType selection);

protected:
    void setDropIndicatorRect(const QRect &rect);
//     virtual bool event(QEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void keyPressEvent(QKeyEvent *event);

//     virtual void mouseMoveEvent(QMouseEvent *event);
//     virtual void paintEvent(QPaintEvent *event);
//     virtual void dropEvent(QDropEvent *event);
//     virtual void dragEnterEvent(QDragEnterEvent *event);
//     virtual void dragMoveEvent(QDragMoveEvent *event);
//     virtual void dragLeaveEvent(QDragLeaveEvent *event);
    virtual void drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const;

private:
    void requestFileTransferChannels(const Tp::AccountPtr &account,
                                     const Tp::ContactPtr &contact,
                                     const QStringList &filenames);

    void loadGroupStatesFromConfig();

    friend class ContextMenu;
    ContactListWidgetPrivate * const d_ptr;
};

Q_DECLARE_METATYPE(QList<QAction *>)

#endif // CONTACT_LIST_WIDGET_H
