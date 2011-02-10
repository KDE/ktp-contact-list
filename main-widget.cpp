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

#include <QtGui/QSortFilterProxyModel>
#include <QtGui/QPainter>
#include <QtGui/QMenu>
#include <QtGui/QLabel>
#include <QtGui/QCheckBox>
#include <QtGui/QPushButton>
#include <QtGui/QToolButton>

#include <KDebug>
#include <KJob>
#include <KLineEdit>
#include <KComboBox>
#include <KMessageBox>
#include <KMenu>
#include <KSelectAction>

#include <TelepathyQt4/PendingReady>
#include <TelepathyQt4/PendingChannelRequest>
#include <TelepathyQt4/ClientRegistrar>

#include <Nepomuk/ResourceManager>
#include <Nepomuk/Variant>
#include <Nepomuk/Query/QueryServiceClient>
#include <Nepomuk/Query/ComparisonTerm>
#include <Nepomuk/Query/ResourceTerm>
#include <Nepomuk/Query/ResourceTypeTerm>
#include <Nepomuk/Query/LiteralTerm>
#include <Nepomuk/Query/AndTerm>
#include <Nepomuk/Query/Result>

#include <TelepathyQt4/Constants>

#include "main-widget.h"
#include "ui_main-widget.h"
#include "account-item.h"
#include "contactsmodelfilter.h"
#include "accountbutton.h"

#define PREFERRED_TEXTCHAT_HANDLER "org.freedesktop.Telepathy.Client.KDEChatHandler"

const int SPACING = 4;
const int AVATAR_SIZE = 32;

// using KTelepathy::ContactsListModel;
// using KTelepathy::GroupedContactsProxyModel;
// using KTelepathy::TelepathyBridge;
// using KTelepathy::AbstractTreeItem;
// using KTelepathy::ContactItem;
// using KTelepathy::MetaContactItem;
// using KTelepathy::RequestTextChatJob;

ContactDelegate::ContactDelegate(QObject * parent)
  : QStyledItemDelegate(parent)
{
}

ContactDelegate::~ContactDelegate()
{
}

void ContactDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & idx) const
{
    QStyleOptionViewItemV4 optV4 = option;
    initStyleOption(&optV4, idx);

    painter->save();

    painter->setClipRect(optV4.rect);

    QStyle *style = QApplication::style();
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter);

    if(idx.data(ModelRoles::IsContact).toBool())
    {
        
        QRect iconRect = optV4.rect;
        iconRect.setSize(QSize(32, 32));
        iconRect.moveTo(QPoint(iconRect.x() + SPACING, iconRect.y() + SPACING));

        const QPixmap pixmap = idx.data(ModelRoles::UserAvatarRole).value<QPixmap>();
        if (!pixmap.isNull()) {
            painter->drawPixmap(iconRect, idx.data(ModelRoles::UserAvatarRole).value<QPixmap>());
        }
        
        QPixmap icon;
        
        switch(idx.data(ModelRoles::UserStatusRole).toInt())
        {
            case Tp::ConnectionPresenceTypeAvailable:
                icon = SmallIcon("user-online", KIconLoader::SizeSmallMedium);
                break;
            case Tp::ConnectionPresenceTypeAway:
                icon = SmallIcon("user-away", KIconLoader::SizeSmallMedium);
                break;
            case Tp::ConnectionPresenceTypeExtendedAway:
                icon = SmallIcon("user-away-extended", KIconLoader::SizeSmallMedium);
                break;
            case Tp::ConnectionPresenceTypeBusy:
                icon = SmallIcon("user-busy", KIconLoader::SizeSmallMedium);
                break;
            case Tp::ConnectionPresenceTypeOffline:
                icon = SmallIcon("user-offline", KIconLoader::SizeSmallMedium);
                break;
            default:
                icon = SmallIcon("user-online", KIconLoader::SizeSmallMedium);
                break;
        }

        QRect userNameRect = optV4.rect;
        userNameRect.setX(iconRect.x() + iconRect.width() + SPACING);
        userNameRect.setY(userNameRect.y() + 3);
        //userNameRect = painter->boundingRect(userNameRect, Qt::AlignLeft | Qt::AlignTop, optV4.text);
        
        QRect statusMsgRect = optV4.rect;
        statusMsgRect.setX(iconRect.x() + iconRect.width() + SPACING);
        statusMsgRect.setY(userNameRect.top() + 16);
        statusMsgRect.setWidth(option.rect.width());
        
        QRect statusIconRect = optV4.rect;
        statusIconRect.setSize(QSize(22,22));
        statusIconRect.moveTo(QPoint(optV4.rect.right() - 24, optV4.rect.top()+8));
        
        painter->drawPixmap(statusIconRect, icon);

        QFont nameFont = painter->font();
        nameFont.setPixelSize(12);
        nameFont.setWeight(QFont::Bold);
        
        painter->setFont(nameFont);
        painter->drawText(userNameRect, optV4.text);
        
        QFont statusFont = painter->font();
        statusFont.setWeight(QFont::Normal);
        statusFont.setPixelSize(10);
        
        painter->setFont(statusFont);
        painter->drawText(statusMsgRect, idx.data(ModelRoles::UserStatusMsgRole).toString());
    }
    else
    {
        QRect groupRect = optV4.rect;
        
        QRect accountGroupRect = groupRect;
        accountGroupRect.setSize(QSize(16,16));
        accountGroupRect.moveTo(QPoint(groupRect.left() + 2, groupRect.top() + 2));
        
        QRect groupLabelRect = groupRect;
        groupLabelRect.setLeft(20);
        //groupLabelRect.setBottom(groupRect.bottom());
        //groupLabelRect.setHeight(16);
        
        QFont groupFont = painter->font();
        groupFont.setWeight(QFont::Normal);
        groupFont.setPixelSize(10);
        
        QString counts = QString(" (%1/%2)").arg(idx.data(ModelRoles::AccountAvailContactsCountRole).toString(),
                                       idx.data(ModelRoles::AccountAllContactsCountRole).toString());
        

        painter->fillRect(groupRect, QColor(247, 251, 255));
        
        painter->drawPixmap(accountGroupRect, KIcon(idx.data(ModelRoles::AccountIconRole).toString()).pixmap(16,16));
        
        painter->setFont(groupFont);
        painter->drawText(groupLabelRect, Qt::AlignVCenter, idx.data(ModelRoles::AccountGroupRole).toString().append(counts));
        
        painter->setPen(QColor(220, 220, 220));
        painter->drawLine(groupRect.x(), groupRect.y(), groupRect.width(), groupRect.y());
        painter->drawLine(groupRect.x(), groupRect.bottom(), groupRect.width(), groupRect.bottom());
    }
//     QRect typeRect;
// 
//     typeRect = painter->boundingRect(optV4.rect, Qt::AlignLeft | Qt::AlignBottom, idx.data(51).toString());
//     typeRect.moveTo(QPoint(typeRect.x() + iconRect.x() + iconRect.width() + SPACING, typeRect.y() - SPACING));
//     painter->drawText(typeRect, idx.data(51).toString());
// 
//     QRect sizeRect = painter->boundingRect(optV4.rect, Qt::AlignRight | Qt::AlignTop, idx.data(50).toString());
//     sizeRect.moveTo(QPoint(sizeRect.x() - SPACING, sizeRect.y() + SPACING));
//     painter->drawText(sizeRect, idx.data(50).toString());

    painter->restore();
}

QSize ContactDelegate::sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    if(index.data(ModelRoles::IsContact).toBool())
    {
        return QSize(0, 32 + 2 * SPACING);
    }
    else return QSize(0,20);
}

MainWidget::MainWidget(QWidget *parent)
 : QWidget(parent),
   m_model(0),
//   m_groupedContactsProxyModel(0),
   m_sortFilterProxyModel(0)
{

    // Check if Nepomuk Query service client is up and running
//     if (!Nepomuk::Query::QueryServiceClient::serviceAvailable()) {
//         // That's a failure
//         KMessageBox::error(this, i18n("The Nepomuk Query Service client is not available on your system. "
//                                       "Contactlist requires the query service client to be available: please "
//                                       "check your system settings"));
//     }

    Tp::registerTypes();

    setupUi(this);
    setWindowIcon(KIcon("telepathy"));
    m_actionAdd_contact->setIcon(KIcon("list-add-user"));
    m_actionGroup_contacts->setIcon(KIcon("user-group-properties"));
    
    
    // Start setting up the Telepathy AccountManager.
    Tp::AccountFactoryPtr  accountFactory = Tp::AccountFactory::create(QDBusConnection::sessionBus(),
                                                                       Tp::Features() << Tp::Account::FeatureCore
                                                                       << Tp::Account::FeatureAvatar
                                                                       << Tp::Account::FeatureProtocolInfo
                                                                       << Tp::Account::FeatureProfile);
    
    Tp::ConnectionFactoryPtr connectionFactory = Tp::ConnectionFactory::create(QDBusConnection::sessionBus(),
                                                                               Tp::Features() << Tp::Connection::FeatureCore
                                                                               << Tp::Connection::FeatureRosterGroups
                                                                               << Tp::Connection::FeatureRoster
                                                                               << Tp::Connection::FeatureSelfContact);
    
    Tp::ContactFactoryPtr contactFactory = Tp::ContactFactory::create(Tp::Features()  << Tp::Contact::FeatureAlias
                                                                      << Tp::Contact::FeatureAvatarData
                                                                      << Tp::Contact::FeatureSimplePresence);
    
    Tp::ChannelFactoryPtr channelFactory = Tp::ChannelFactory::create(QDBusConnection::sessionBus());
    
    m_accountManager = Tp::AccountManager::create(QDBusConnection::sessionBus(), accountFactory, connectionFactory, channelFactory, contactFactory);

    m_accountsListModel = new AccountsListModel(this);
    
    connect(m_accountManager->becomeReady(),
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onAccountManagerReady(Tp::PendingOperation*)));
    
    // Initialize Telepathy
    //TelepathyBridge::instance()->init();
    //connect(TelepathyBridge::instance(),
    //        SIGNAL(ready(bool)),
    //        SLOT(onHandlerReady(bool)));

    m_model = new FakeContactsModel(this);
    m_sortFilterProxyModel = new QSortFilterProxyModel(this);
    m_sortFilterProxyModel->setSourceModel(m_model);
    m_sortFilterProxyModel->setDynamicSortFilter(true);
    m_sortFilterProxyModel->setFilterRole(ModelRoles::UserStatusRole);
    m_sortFilterProxyModel->setSortRole(ModelRoles::UserNameRole);

    //m_groupedContactsProxyModel = new GroupedContactsProxyModel(this);
    //m_groupedContactsProxyModel->setSourceModel(m_model);
    
    m_contactsListView->header()->hide();
    m_contactsListView->setRootIsDecorated(false);
    m_contactsListView->setSortingEnabled(true);
    m_contactsListView->setContextMenuPolicy(Qt::CustomContextMenu);
    m_contactsListView->setModel(m_model);
    m_contactsListView->setItemDelegate(new ContactDelegate(this));
    m_contactsListView->setIndentation(0);
    m_contactsListView->setExpandsOnDoubleClick(false); //the expanding/collapsing is handled manually
    
    connect(m_contactsListView, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(onCustomContextMenuRequested(QPoint)));
    
    connect(m_contactsListView, SIGNAL(doubleClicked(QModelIndex)),
            this, SLOT(onContactListDoubleClick(QModelIndex)));
    
    connect(m_actionAdd_contact, SIGNAL(triggered(bool)),
            this, SLOT(onAddContactRequest(bool)));
    
    connect(m_actionGroup_contacts, SIGNAL(triggered(bool)),
            this, SLOT(onGroupContacts(bool)));

   
    // Get 'me' as soon as possible
    // FIXME: Port to new OSCAF standard for accessing "me" as soon as it
    // becomes available.
//     Nepomuk::Thing me(QUrl::fromEncoded("nepomuk:/myself"));

    // Loop through all the grounding instances of this person
//     foreach (Nepomuk::InformationElement resource, me.groundingOccurrences()) {
//         // See if this grounding instance is of type nco:contact.
//         if (resource.hasType(Nepomuk::Vocabulary::NCO::PersonContact())) {
//             // FIXME: We are going to assume the first NCO::PersonContact is the
//             // right one. Can we improve this?
//             m_mePersonContact = resource;
//             break;
//         }
//     }
    
    
}

MainWidget::~MainWidget()
{
    kDebug();
    //setStatus(7);
}

void MainWidget::onAccountManagerReady(Tp::PendingOperation* op)
{
    if (op->isError()) {
        
        kDebug() << op->errorName();
        kDebug() << op->errorMessage();
    }
    
    QList<Tp::AccountPtr> accounts = m_accountManager->allAccounts();
    foreach (Tp::AccountPtr account, accounts) 
    {
        if(account->isEnabled()) 
        {
            account->becomeReady();
            m_accountsListModel->addAccount(account);
            connect(account.data(),
                    SIGNAL(connectionChanged(Tp::ConnectionPtr)),
                    this, SLOT(onConnectionChanged(Tp::ConnectionPtr)));
            
            connect(account.data(),
                    SIGNAL(connectionStatusChanged(Tp::ConnectionStatus)), 
                    this, SLOT(onAccountConnectionStatusChanged(Tp::ConnectionStatus)));
            
            if(account->connectionStatus() == Tp::ConnectionStatusConnected && account->connection())
            {
                loadContactsFromAccount(account);
            }

            AccountButton *bt = new AccountButton(account, this);

            m_accountButtonsLayout->addWidget(bt);
        }

    }
    
    m_accountButtonsLayout->insertStretch(-1);
}

void MainWidget::onAccountReady(Tp::PendingOperation* op)
{
    if (op->isError()) {
        qWarning() << "Account cannot become ready";
        return;
    }
    
    Tp::PendingReady *pendingReady = qobject_cast<Tp::PendingReady*>(op);
    Q_ASSERT(pendingReady);   
}

void MainWidget::onAccountConnectionStatusChanged(Tp::ConnectionStatus status)
{   
    //TODO: Add some handling
    kDebug() << "Connection status is" << status;
    if(status == Tp::ConnectionStatusConnecting)
        showMessageToUser(i18n("Connecting..."));
}

void MainWidget::onConnectionChanged(const Tp::ConnectionPtr& connection)
{
    Tp::AccountPtr account(qobject_cast<Tp::Account*>(sender()));
    loadContactsFromAccount(account);
}

void MainWidget::loadContactsFromAccount(const Tp::AccountPtr& account)
{
    m_model->addAccountContacts(account);
    //m_currentAccountButtonPressed = -1;
    m_sortFilterProxyModel->sort(0, Qt::AscendingOrder);
    m_contactsListView->expandAll();
}

void MainWidget::onContactListDoubleClick(const QModelIndex& index)
{
    if(!index.isValid()) {
        return;
    }
    kDebug() << index;   
    ContactItem *item = static_cast<ContactItem*>(index.internalPointer());
    if(item->isContact())
    {
        kDebug() << "Text chat requested";
        startTextChannel(index);
    }
    else
    {
        if(m_contactsListView->isExpanded(index))
            m_contactsListView->collapse(index);
        else m_contactsListView->expand(index);
        
        kDebug() << index.data(ModelRoles::AccountGroupRole);
    }
}

void MainWidget::startTextChannel(const QModelIndex &index)
{
    //QModelIndex index = m_contactsListView->currentIndex();
    if (! index.isValid()) {
        return;
    }
    
    Tp::ContactPtr contact = m_model->contact(index);
    kDebug() << "Requesting chat for contact" << contact->alias();
    
    Tp::AccountPtr account = m_model->account(index);
    kDebug() << account->displayName();
    
    Tp::PendingChannelRequest* channelRequest = account->ensureTextChat(contact);
    connect(channelRequest, SIGNAL(finished(Tp::PendingOperation*)), SLOT(onChannelJoined(Tp::PendingOperation*)));
}

void MainWidget::onChannelJoined(Tp::PendingOperation* op)
{
    if (op->isError()) {
        kDebug() << op->errorName();
        kDebug() << op->errorMessage();
    }
}


void MainWidget::onHandlerReady(bool ready)
{
    if (!ready) {
        kWarning() << "Telepathy handler could not become ready!";
    } else {
        kDebug() << "Telepathy handler ready";
    }
}

void MainWidget::showMessageToUser(const QString& text)
{
    QFrame *msgFrame = new QFrame(m_contactsListView);
    msgFrame->setAttribute(Qt::WA_DeleteOnClose);
    msgFrame->setMinimumSize(QSize(m_contactsListView->width(), 150));
    msgFrame->setFrameShape(QFrame::StyledPanel);
    msgFrame->setFrameShadow(QFrame::Plain);
    msgFrame->setAutoFillBackground(true);
    
    QLabel *message = new QLabel(text, msgFrame);
    
    msgFrame->show();
    
    //QTimeLine *tl = new QTimeLine(4000);
    
    QPropertyAnimation *a = new QPropertyAnimation(msgFrame, "pos");
    a->setParent(msgFrame);
    a->setDuration(4000);
    a->setEasingCurve(QEasingCurve::OutExpo);
    a->setStartValue(QPointF(m_contactsListView->pos().x(), m_contactsListView->pos().y()+m_contactsListView->height()));
    a->setEndValue(QPointF(m_contactsListView->pos().x(), m_contactsListView->pos().y()+m_contactsListView->height()-100));
    a->start();
    
//     m_anim = new Animation(msgFrame, "pos");
//     m_anim->setEasingCurve(QEasingCurve::OutExpo);
    
//     m_anim->setStartValue(QPointF(m_contactsListView->pos().x(), m_contactsListView->pos().y()+m_contactsListView->height()));
//     m_anim->setEndValue(QPointF(m_contactsListView->pos().x(), m_contactsListView->pos().y()+m_contactsListView->height()-100));
//     m_anim->setDuration(4000);
//     m_anim->setLoopCount(1);
//     m_anim->start();
    
    QTimer::singleShot(4500, msgFrame, SLOT(close()));
    
}

void MainWidget::onCustomContextMenuRequested(const QPoint& point)
{
//     QModelIndex proxyIdx = m_contactsListView->indexAt(point);
//     if (!proxyIdx.isValid()) {
//         kDebug() << "Invalid index provided";
//         // Flee
//         return;
//     }
// 
//     // Map the index to the real model
//     QModelIndex idx = m_currentModel->mapToSource(proxyIdx);
//     if (!idx.isValid()) {
//         kDebug() << "Could not map to source";
//         // Flee
//         return;
//     }
// 
//     // Ok, now let's guess
//     AbstractTreeItem *abstractItem = static_cast<AbstractTreeItem*>(idx.internalPointer());
//     kDebug() << idx << idx.internalPointer() << abstractItem;
//     ContactItem *contactItem = dynamic_cast<ContactItem*>(abstractItem);
//     kDebug() << contactItem;
//     QMenu *menu = new QMenu;
// 
//     // Of course we want to chat!
//     QAction *chatAction = menu->addAction(i18n("Start Chat..."));
//     chatAction->setIcon( KIcon("mail-message-new") );
//     connect(chatAction, SIGNAL(triggered(bool)),
//             this, SLOT(onStartChat(bool)));
// 
//     menu->addSeparator();
// 
//     if (contactItem) {
//         kDebug() << "A contactitem";
//         // Ok, now let's see what we can do
//         if (!contactItem->groups().isEmpty()) {
//             QMenu *parentAction = new QMenu(i18n("Remove from group"));
//             menu->addMenu(parentAction);
//             foreach (const QString &group, contactItem->groups()) {
//                 QAction *action = parentAction->addAction(group);
//                 connect(action, SIGNAL(triggered(bool)),
//                         SLOT(onRequestRemoveFromGroup(bool)));
//             }
//         }
// 
//         // Ok, can we also add this contact to some other groups?
//         QStringList allGroups = TelepathyBridge::instance()->knownGroupsFor(contactItem->personContact());
//         kDebug() << "All groups: " << allGroups;
// 
//         QStringList nonJoinedGroups;
//         foreach (const QString &group, allGroups) {
//             if (!contactItem->groups().contains(group)) {
//                 nonJoinedGroups << group;
//             }
//         }
// 
//         // Ok, now let's see what we can do
//         if (!nonJoinedGroups.isEmpty()) {
//             QMenu *parentAction = new QMenu(i18n("Add to group"));
//             menu->addMenu(parentAction);
//             foreach (const QString &group, nonJoinedGroups) {
//                 QAction *action = parentAction->addAction(group);
//                 connect(action, SIGNAL(triggered(bool)),
//                         SLOT(onRequestAddToGroup(bool)));
//             }
//         }
// 
//         // Add/remove to metacontacts
//         // First of all: can it be added to a metacontact?
//         bool canAddToMetaContact = false;
//         MetaContactItem *metaContactItem = dynamic_cast<MetaContactItem*>(contactItem->parentItem());
//         if (metaContactItem) {
//             canAddToMetaContact = metaContactItem->type() == MetaContactItem::FakeMetaContact ? true : false;
//         } else {
//             canAddToMetaContact = true;
//         }
// 
//         if (canAddToMetaContact) {
//             // List available metacontacts
//             QList<Nepomuk::Query::Result> results;
//             {
//                 using namespace Nepomuk::Query;
// 
//                 Query query(ResourceTypeTerm(Nepomuk::Vocabulary::PIMO::Person()));
// 
//                 bool queryResult = true;
//                 results = QueryServiceClient::syncQuery(query, &queryResult);
// 
//                 if (!queryResult) {
//                     KMessageBox::error(0, i18n("It was not possible to query Nepomuk database. Please check your "
//                                                "installation and make sure Nepomuk is running."));
//                 }
//             }
// 
//             QMenu *parentAction = new QMenu(i18n("Add to metacontact"));
//             menu->addMenu(parentAction);
//             // Iterate over all the IMAccounts found.
//             foreach (const Nepomuk::Query::Result &result, results) {
//                 Nepomuk::Person foundPerson(result.resource());
//                 kDebug() << foundPerson;
//                 QAction *action = parentAction->addAction(foundPerson.genericLabel());
//                 connect(action, SIGNAL(triggered(bool)),
//                         this, SLOT(onAddToMetaContact(bool)));
//             }
// 
//             QAction *action = parentAction->addAction(i18nc("Adds a new metacontact", "Add new..."));
//             connect(action, SIGNAL(triggered(bool)),
//                     this, SLOT(onAddToMetaContact(bool)));
//         } else {
//             // We can remove it from a metacontact instead
//             QAction *action = menu->addAction(i18n("Remove from metacontact"));
//             connect(action, SIGNAL(triggered(bool)),
//                     this, SLOT(onRemoveFromMetacontact(bool)));
//         }
//     }
// 
//     MetaContactItem *metaContactItem = dynamic_cast<MetaContactItem*>(abstractItem);
//     kDebug() << metaContactItem;
//     if (metaContactItem) {
//         kDebug() << "A metacontactitem";
//         // Ok, now let's see what we can do
//         if (!metaContactItem->groups().isEmpty()) {
//             QMenu *parentAction = new QMenu(i18n("Remove from group"));
//             menu->addMenu(parentAction);
//             foreach (const QString &group, metaContactItem->groups()) {
//                 QAction *action = parentAction->addAction(group);
//                 connect(action, SIGNAL(triggered(bool)),
//                         SLOT(onRequestRemoveFromGroup(bool)));
//             }
//         }
// 
//         // Ok, can we also add this contact to some other groups?
//         QStringList allGroups = TelepathyBridge::instance()->knownGroupsFor(metaContactItem->pimoPerson());
//         kDebug() << "All groups: " << allGroups;
// 
//         QStringList nonJoinedGroups;
//         foreach (const QString &group, allGroups) {
//             if (!contactItem->groups().contains(group)) {
//                 nonJoinedGroups << group;
//             }
//         }
// 
//         // Ok, now let's see what we can do
//         if (!nonJoinedGroups.isEmpty()) {
//             QMenu *parentAction = new QMenu(i18n("Add to group"));
//             menu->addMenu(parentAction);
//             foreach (const QString &group, nonJoinedGroups) {
//                 QAction *action = parentAction->addAction(group);
//                 connect(action, SIGNAL(triggered(bool)),
//                         SLOT(onRequestAddToGroup(bool)));
//             }
//         }
//     }
// 
//     // And of course remove the contact
//     QAction *removeContact = menu->addAction(KIcon("list-remove"), i18n("Remove"));
//     QAction *blockContact = menu->addAction(KIcon("dialog-cancel"), i18n("Block"));
// 
//     connect(removeContact, SIGNAL(triggered(bool)),
//             SLOT(onContactRemovalRequest(bool)));
//     connect(blockContact, SIGNAL(triggered(bool)),
//             SLOT(onContactBlockRequest(bool)));
// 
//     menu->exec(m_contactsListView->mapToGlobal(point));
//     menu->deleteLater();
}

void MainWidget::onRequestRemoveFromGroup(bool )
{
//     QAction *action = qobject_cast< QAction* >(sender());
//     if (!action) {
//         kDebug() << "invalid";
//         return;
//     }
// 
//     kDebug() << "Request removal from group " << action->text();
// 
//     // Pick the current model index
//     QModelIndex idx = m_currentModel->mapToSource(m_contactsListView->currentIndex());
//     if (!idx.isValid()) {
//         // Flee
//         kDebug() << "Invalid index";
//         return;
//     }
// 
//     // Ok, what is it?
//     AbstractTreeItem *abstractItem = static_cast<AbstractTreeItem*>(idx.internalPointer());
//     ContactItem *contactItem = dynamic_cast<ContactItem*>(abstractItem);
// 
//     if (contactItem) {
//         // Remove the contact
//         KJob *job = TelepathyBridge::instance()->removeContactFromGroup(action->text(), contactItem->personContact());
//         QEventLoop e;
//         connect(job, SIGNAL(finished(KJob*)), &e, SLOT(quit()));
//         job->start();
//         kDebug() << "Running job...";
//         e.exec();
//         kDebug() << "Job run, "<< job->error();
//         kDebug() << "rm contacts";
//     }
// 
//     MetaContactItem *metaContactItem = dynamic_cast<MetaContactItem*>(abstractItem);
//     if (metaContactItem) {
//         // Remove the metacontact
//         TelepathyBridge::instance()->removeMetaContactFromGroup(action->text(), metaContactItem->pimoPerson());
//         kDebug() << "rm contacts";
//     }
}

void MainWidget::onRequestAddToGroup(bool )
{
//     QAction *action = qobject_cast< QAction* >(sender());
//     if (!action) {
//         kDebug() << "invalid";
//         return;
//     }
// 
//     kDebug() << "Request addition group " << action->text();
// 
//     // Pick the current model index
//     QModelIndex idx = m_currentModel->mapToSource(m_contactsListView->currentIndex());
//     if (!idx.isValid()) {
//         // Flee
//         kDebug() << "Invalid index";
//         return;
//     }
// 
//     // Ok, what is it?
//     AbstractTreeItem *abstractItem = static_cast<AbstractTreeItem*>(idx.internalPointer());
//     ContactItem *contactItem = dynamic_cast<ContactItem*>(abstractItem);
// 
//     if (contactItem) {
//         // Remove the contact
//         KJob *job = TelepathyBridge::instance()->addContactToGroup(action->text(), contactItem->personContact());
//         QEventLoop e;
//         connect(job, SIGNAL(finished(KJob*)), &e, SLOT(quit()));
//         job->start();
//         kDebug() << "Running job...";
//         e.exec();
//         kDebug() << "Job run, "<< job->error();
//     }
// 
//     MetaContactItem *metaContactItem = dynamic_cast<MetaContactItem*>(abstractItem);
//     if (metaContactItem) {
//         // Remove the metacontact
//         TelepathyBridge::instance()->removeMetaContactFromGroup(action->text(), metaContactItem->pimoPerson());
//     }
}

void MainWidget::onContactBlockRequest(bool )
{

}

void MainWidget::onRemoveFromMetacontact(bool )
{
//     // Pick the current model index
//     QModelIndex idx = m_currentModel->mapToSource(m_contactsListView->currentIndex());
//     if (!idx.isValid()) {
//         // Flee
//         kDebug() << "Invalid index";
//         return;
//     }
// 
//     // Ok, what is it?
//     AbstractTreeItem *abstractItem = static_cast<AbstractTreeItem*>(idx.internalPointer());
//     ContactItem *contactItem = dynamic_cast<ContactItem*>(abstractItem);
// 
//     Q_ASSERT(contactItem);
// 
//     KJob *job = TelepathyBridge::instance()->removeContact(contactItem->personContact(),
//                                                            TelepathyBridge::RemoveFromMetacontactMode);
// 
//     QEventLoop e;
//     connect(job, SIGNAL(finished(KJob*)), &e, SLOT(quit()));
//     job->start();
//     qDebug() << "Running job...";
//     e.exec();
//     qDebug() << "Job run, "<< job->error();
}

void MainWidget::onContactRemovalRequest(bool )
{
//     QAction *action = qobject_cast< QAction* >(sender());
//     if (!action) {
//         kDebug() << "invalid";
//         return;
//     }
// 
//     kDebug() << "Request addition group " << action->text();
// 
//     // Pick the current model index
//     QModelIndex idx = m_currentModel->mapToSource(m_contactsListView->currentIndex());
//     if (!idx.isValid()) {
//         // Flee
//         kDebug() << "Invalid index";
//         return;
//     }
// 
//     // Ok, what is it?
//     AbstractTreeItem *abstractItem = static_cast<AbstractTreeItem*>(idx.internalPointer());
//     ContactItem *contactItem = dynamic_cast<ContactItem*>(abstractItem);
// 
//     if (contactItem) {
//         // Build a dialog
//         KDialog *dial = new KDialog(this);
//         QWidget *w = new QWidget;
//         QLabel *l = new QLabel(i18n("Please select the removal means for this contact"));
//         QCheckBox *presence = 0;
//         QCheckBox *subscription = 0;
//         QCheckBox *block = 0;
//         QVBoxLayout *lay = new QVBoxLayout;
//         lay->addWidget(l);
// 
//         // What can we do?
//         TelepathyBridge::RemovalModes modes = TelepathyBridge::instance()->supportedRemovalModesFor(contactItem->personContact());
// 
//         if (modes & TelepathyBridge::RemovePublicationMode) {
//             presence = new QCheckBox(i18n("Don't show me in his buddy list anymore"));
//             // On by default
//             presence->setCheckState(Qt::Checked);
//             lay->addWidget(presence);
//         }
//         if (modes & TelepathyBridge::RemoveSubscriptionMode) {
//             subscription = new QCheckBox(i18n("Don't show him in my buddy list anymore"));
//             // On by default
//             subscription->setCheckState(Qt::Checked);
//             lay->addWidget(subscription);
//         }
//         if (modes & TelepathyBridge::BlockMode) {
//             block = new QCheckBox(i18n("Block him"));
//             // Off by default
//             block->setCheckState(Qt::Unchecked);
//             lay->addWidget(block);
//         }
// 
//         w->setLayout(lay);
//         dial->setMainWidget(w);
// 
//         if (dial->exec() == KDialog::Accepted) {
//             TelepathyBridge::RemovalModes execModes = 0;
//             if (presence) {
//                 if (presence->isChecked()) {
//                     if (execModes == 0) {
//                         execModes = TelepathyBridge::RemovePublicationMode;
//                     } else {
//                         execModes |= TelepathyBridge::RemovePublicationMode;
//                     }
//                 }
//             }
//             if (subscription) {
//                 if (subscription->isChecked()) {
//                     if (execModes == 0) {
//                         execModes = TelepathyBridge::RemoveSubscriptionMode;
//                     } else {
//                         execModes |= TelepathyBridge::RemoveSubscriptionMode;
//                     }
//                 }
//             }
//             if (block) {
//                 if (block->isChecked()) {
//                     if (execModes == 0) {
//                         execModes = TelepathyBridge::BlockMode;
//                     } else {
//                         execModes |= TelepathyBridge::BlockMode;
//                     }
//                 }
//             }
// 
//             if (execModes == 0) {
//                 qDebug() << "Nothing to do!";
//                 return;
//             }
// 
//             // Remove the contact
//             KJob *job = TelepathyBridge::instance()->removeContact(contactItem->personContact(), execModes);
//             QEventLoop e;
//             connect(job, SIGNAL(finished(KJob*)), &e, SLOT(quit()));
//             job->start();
//             kDebug() << "Running job...";
//             e.exec();
//             kDebug() << "Job run, "<< job->error();
//         }
//     }
}

void MainWidget::onAddContactRequest(bool )
{
//     // Let's build a dialog
//     KDialog *dial = new KDialog(this);
//     QWidget *w = new QWidget;
//     QLabel *l = new QLabel(i18n("Please enter the ID of the contact in question"));
//     KComboBox *account = new KComboBox();
//     KLineEdit *contactId = new KLineEdit();
//     QVBoxLayout *lay = new QVBoxLayout;
//     lay->addWidget(l);
//     lay->addWidget(account);
//     lay->addWidget(contactId);
// 
//     // Get all valid Telepathy accounts
//     QList<Nepomuk::Query::Result> results;
//     {
//         using namespace Nepomuk::Query;
// 
//         // me must have an IMAccount
//         ComparisonTerm imterm(Nepomuk::Vocabulary::NCO::hasIMAccount(),
//                               ResourceTerm(m_mePersonContact));
//         imterm.setInverted(true);
// 
//         // Which must be an IMAccount of course
//         Query query(AndTerm(ResourceTypeTerm(Nepomuk::Vocabulary::NCO::IMAccount()),
//                             imterm));
// 
//         bool queryResult = true;
//         results = QueryServiceClient::syncQuery(query, &queryResult);
// 
//         if (!queryResult) {
//             KMessageBox::error(0, i18n("It was not possible to query Nepomuk database. Please check your "
//                                         "installation and make sure Nepomuk is running."));
//         }
//     }
// 
//     // Iterate over all the IMAccounts/PersonContacts found.
//     foreach (const Nepomuk::Query::Result &result, results) {
//         Nepomuk::IMAccount foundIMAccount(result.resource());
//         uint statusType = foundIMAccount.statusTypes().first();
//         if( statusType != Tp::ConnectionPresenceTypeUnset   &&
//             statusType != Tp::ConnectionPresenceTypeOffline &&
//             statusType != Tp::ConnectionPresenceTypeUnknown &&
//             statusType != Tp::ConnectionPresenceTypeError)
//         {
//             foreach (const QString &id, foundIMAccount.imIDs()) {
//                 account->addItem(id, foundIMAccount.resourceUri());
//             }
//         }
//     }
// 
//     w->setLayout(lay);
//     dial->setMainWidget(w);
// 
//     if (dial->exec() == KDialog::Accepted) {
//         // Add the contact
//         Nepomuk::IMAccount toAddAccount(account->itemData(account->currentIndex()).toUrl());
//         KJob *job = TelepathyBridge::instance()->addContact(toAddAccount, contactId->text());
//         QEventLoop e;
//         connect(job, SIGNAL(finished(KJob*)), &e, SLOT(quit()));
//         job->start();
//         qDebug() << "Running job...";
//         e.exec();
//         qDebug() << "Job run, "<< job->error();
//     }
}

void MainWidget::onGroupContacts(bool grouped)
{
//     if (grouped && m_currentModel != m_groupedContactsProxyModel) {
//         m_currentModel = m_groupedContactsProxyModel;
//         m_contactsListView->setModel(m_groupedContactsProxyModel);
//     }
//     else if(!grouped && m_currentModel != m_sortFilterProxyModel) {
//         m_currentModel = m_sortFilterProxyModel;
//         m_contactsListView->setModel(m_sortFilterProxyModel);
//     }

}

void MainWidget::onAddToMetaContact(bool )
{
//     QAction *action = qobject_cast< QAction* >(sender());
//     if (!action) {
//         kDebug() << "invalid";
//         return;
//     }
// 
//     QString metaContactName = action->text();
//     kDebug() << "Request adding to metacontact " << metaContactName;
// 
//     // Pick the current model index
//     QModelIndex idx = m_currentModel->mapToSource(m_contactsListView->currentIndex());
//     if (!idx.isValid()) {
//         // Flee
//         kDebug() << "Invalid index";
//         return;
//     }
// 
//     // Ok, what is it?
//     AbstractTreeItem *abstractItem = static_cast<AbstractTreeItem*>(idx.internalPointer());
//     ContactItem *contactItem = dynamic_cast<ContactItem*>(abstractItem);
// 
//     Q_ASSERT(contactItem);
// 
//     if (metaContactName == i18nc("Adds a new metacontact", "Add new...")) {
//         // Prompt to create a new metacontact
//         // Let's build a dialog
//         KDialog *dial = new KDialog(this);
//         QWidget *w = new QWidget;
//         QLabel *l = new QLabel(i18n("Please enter a name for this new metacontact"));
//         KLineEdit *contactId = new KLineEdit();
//         QVBoxLayout *lay = new QVBoxLayout;
//         lay->addWidget(l);
//         lay->addWidget(contactId);
// 
//         w->setLayout(lay);
//         dial->setMainWidget(w);
// 
//         if (dial->exec() == KDialog::Accepted) {
//             // Add the contact
//             metaContactName = contactId->text();
//             KJob *job = TelepathyBridge::instance()->addMetaContact(contactId->text(),
//                                                                     QList< Nepomuk::PersonContact >() <<
//                                                                     contactItem->personContact());
//             QEventLoop e;
//             connect(job, SIGNAL(finished(KJob*)), &e, SLOT(quit()));
//             job->start();
//             qDebug() << "Running job...";
//             e.exec();
//             qDebug() << "Job run, "<< job->error();
//         }
// 
//         return;
//     }
// 
//     // Ok, now let's add the contact
//     QList< Nepomuk::Query::Result > results;
//     {
//         using namespace Nepomuk::Query;
//         using namespace Nepomuk::Vocabulary;
// 
//         ResourceTypeTerm rtterm(PIMO::Person());
//         ComparisonTerm cmpterm(NAO::prefLabel(), LiteralTerm(metaContactName));
// 
//         Query query(AndTerm(cmpterm, rtterm));
// 
//         bool queryResult = true;
//         results = QueryServiceClient::syncQuery(query, &queryResult);
// 
//         if (!queryResult) {
//             KMessageBox::error(0, i18n("It was not possible to query Nepomuk database. Please check your "
//                                        "installation and make sure Nepomuk is running."));
//         }
//     }
// 
//     // Iterate over all the IMAccounts found.
//     foreach (const Nepomuk::Query::Result &result, results) {
//         Nepomuk::Person foundPerson(result.resource());
//         foundPerson.addGroundingOccurrence(contactItem->personContact());
//     }
}

void MainWidget::onStartChat(bool)
{
//     QAction *action = qobject_cast< QAction* >(sender());
//     if (!action) {
//         kDebug() << "invalid";
//         return;
//     }
// 
//     // Pick the current model index
//     QModelIndex idx = m_currentModel->mapToSource(m_contactsListView->currentIndex());
//     if (!idx.isValid()) {
//         // Flee
//         kDebug() << "Invalid index";
//         return;
//     }
// 
//     RequestTextChatJob* job;
//     // Ok, what is it?
//     AbstractTreeItem *abstractItem = static_cast<AbstractTreeItem*>(idx.internalPointer());
//     ContactItem *contactItem = dynamic_cast<ContactItem*>(abstractItem);
//     MetaContactItem *metacontactItem = dynamic_cast<MetaContactItem*>(abstractItem);
// 
//     if (contactItem) {
//         kDebug() << "Request chat to contact";
//         Nepomuk::PersonContact contact = contactItem->personContact();
//         kDebug() << contact.resourceUri() << contact.genericLabel();
//         job = KTelepathy::requestTextChat(contact, PREFERRED_TEXTCHAT_HANDLER, this);
//     } else if (metacontactItem && metacontactItem->type() == MetaContactItem::RealMetaContact) {
//         kDebug() << "Request chat to REAL metacontact";
//         Nepomuk::Person metacontact = metacontactItem->pimoPerson();
//         kDebug() << metacontact.resourceUri() << metacontact.genericLabel();
//         job = KTelepathy::requestTextChat(metacontact, PREFERRED_TEXTCHAT_HANDLER, this);
//     } else if (metacontactItem && metacontactItem->type() == MetaContactItem::FakeMetaContact) {
//         kDebug() << "Request chat to FAKE metacontact";
//         QList<AbstractTreeItem*> childList = metacontactItem->childItems();
//         AbstractTreeItem *childItem = childList.first(); //It should just have one
//         Q_ASSERT(childItem);
//         contactItem = dynamic_cast<ContactItem*>(childItem);
//         if (!contactItem) {
//             KMessageBox::error(0, i18n("An error occurred????"));
//             kWarning() << "Cannot dynamic cast child item!";
//         } else {
//             Nepomuk::PersonContact contact = contactItem->personContact();
//             kDebug() << contact.resourceUri() << contact.genericLabel();
//             job = KTelepathy::requestTextChat(contact, PREFERRED_TEXTCHAT_HANDLER, this);
//         }
//     } else {
//         KMessageBox::error(0, i18n("An error occurred????"));
//         kWarning() << "This is not a contact or a metacontact!";
//     }
//     if ( !job->exec() ) {
//         // An error occurred
//         KMessageBox::error(0, i18n("Impossible to start a chat"));
//         kWarning() << "Impossible to start a chat";
//     } else {
//         kDebug() << "This should be a success.";
//     }
}


#include "main-widget.moc"

