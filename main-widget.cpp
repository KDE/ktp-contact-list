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

#include "main-widget.moc"

#include <QtGui/QSortFilterProxyModel>
#include <QtGui/QPainter>
#include <QtGui/QMenu>
#include <QtGui/QLabel>
#include <QtGui/QCheckBox>
#include <QtGui/QPushButton>
#include <QtGui/QToolButton>

#include <TelepathyQt4/PendingReady>
#include <TelepathyQt4/PendingChannelRequest>
#include <TelepathyQt4/ClientRegistrar>
#include <TelepathyQt4/Constants>

#include <KDebug>

#include "main-widget.h"
#include "ui_main-widget.h"
#include "accountbutton.h"
#include "contactoverlays.h"
#include "accounts-model.h"
#include "accountfiltermodel.h"
#include "contactdelegate.h"

#define PREFERRED_TEXTCHAT_HANDLER "org.freedesktop.Telepathy.Client.KDEChatHandler"

MainWidget::MainWidget(QWidget *parent)
 : QWidget(parent),
   m_model(0),
   m_modelFilter(0)
{
    Tp::registerTypes();

    setupUi(this);
    setWindowIcon(KIcon("telepathy"));
    m_actionAdd_contact->setIcon(KIcon("list-add-user"));
    m_actionAdd_contact->setText(QString());
    m_actionAdd_contact->setToolTip(i18n("Add new contacts.."));
    
    m_actionGroup_contacts->setIcon(KIcon("user-group-properties"));
    m_actionGroup_contacts->setText(QString());
    //TODO: Toggle the tooltip with the button? eg. once its Show, after click its Hide .. ?
    m_actionGroup_contacts->setToolTip(i18n("Show/Hide groups"));
    
    m_actionHide_offline->setIcon(KIcon("meeting-attending-tentative"));
    m_actionHide_offline->setText(QString());
    m_actionHide_offline->setToolTip(i18n("Show/Hide offline users"));
    
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
                                                                      << Tp::Contact::FeatureSimplePresence
                                                                      << Tp::Contact::FeatureCapabilities);
    
    Tp::ChannelFactoryPtr channelFactory = Tp::ChannelFactory::create(QDBusConnection::sessionBus());
    
    m_accountManager = Tp::AccountManager::create(QDBusConnection::sessionBus(), accountFactory, connectionFactory, channelFactory, contactFactory);
    
    connect(m_accountManager->becomeReady(),
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onAccountManagerReady(Tp::PendingOperation*)));
    
    // Initialize Telepathy
    //TelepathyBridge::instance()->init();
    //connect(TelepathyBridge::instance(),
    //        SIGNAL(ready(bool)),
    //        SLOT(onHandlerReady(bool)));
    
    m_delegate = new ContactDelegate(this);
    
    m_contactsListView->header()->hide();
    m_contactsListView->setRootIsDecorated(false);
    m_contactsListView->setSortingEnabled(true);
    m_contactsListView->setContextMenuPolicy(Qt::CustomContextMenu);
    m_contactsListView->setItemDelegate(m_delegate);
    m_contactsListView->setIndentation(0);
    m_contactsListView->setMouseTracking(true);
    m_contactsListView->setExpandsOnDoubleClick(false); //the expanding/collapsing is handled manually
    
    addOverlayButtons();
    
    
    connect(m_contactsListView, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(onCustomContextMenuRequested(QPoint)));
    
    connect(m_contactsListView, SIGNAL(doubleClicked(QModelIndex)),
            this, SLOT(onContactListDoubleClick(QModelIndex)));
    
    connect(m_delegate, SIGNAL(repaintItem(QModelIndex)),
            m_contactsListView->viewport(), SLOT(repaint())); //update(QModelIndex)
    
    connect(m_actionAdd_contact, SIGNAL(triggered(bool)),
            this, SLOT(onAddContactRequest(bool)));
    
    connect(m_actionGroup_contacts, SIGNAL(triggered(bool)),
            this, SLOT(onGroupContacts(bool)));
}

MainWidget::~MainWidget()
{
}

void MainWidget::onAccountManagerReady(Tp::PendingOperation* op)
{
    if (op->isError()) {
        
        kDebug() << op->errorName();
        kDebug() << op->errorMessage();
    }
    
    m_model = new AccountsModel(m_accountManager, this);
    m_modelFilter = new AccountFilterModel(this);
    m_modelFilter->setSourceModel(m_model);
    m_modelFilter->setDynamicSortFilter(true);
    m_modelFilter->filterOfflineUsers(true);
    m_modelFilter->setSortRole(Qt::DisplayRole);
    m_contactsListView->setModel(m_modelFilter);
    m_contactsListView->setSortingEnabled(true);
    m_contactsListView->sortByColumn(0, Qt::AscendingOrder);

    connect(m_actionHide_offline, SIGNAL(toggled(bool)),
            m_modelFilter, SLOT(filterOfflineUsers(bool)));
    
    
    QList<Tp::AccountPtr> accounts = m_accountManager->allAccounts();
    foreach (Tp::AccountPtr account, accounts) 
    {
        if(account->isEnabled()) 
        {
            account->becomeReady();
            
            connect(account.data(),
                    SIGNAL(connectionChanged(Tp::ConnectionPtr)),
                    this, SLOT(onConnectionChanged(Tp::ConnectionPtr)));
            
            connect(account.data(),
                    SIGNAL(connectionStatusChanged(Tp::ConnectionStatus)), 
                    this, SLOT(onAccountConnectionStatusChanged(Tp::ConnectionStatus)));

            AccountButton *bt = new AccountButton(account, this);

            m_accountButtonsLayout->addWidget(bt);         
        }
    }
    m_contactsListView->expandAll();

    m_accountButtonsLayout->insertStretch(-1);
    
//     QPushButton *bInfo = new QPushButton(this);
//     bInfo->setText("Info");
//     bInfo->setObjectName("infoBt");
//     
//     QPushButton *bErr = new QPushButton(this);
//     bErr->setText("Err");
//     bErr->setObjectName("errBt");
//     
//     connect(bInfo, SIGNAL(clicked(bool)), this, SLOT(systemMessageTest()));
//     connect(bErr, SIGNAL(clicked(bool)), this, SLOT(systemMessageTest()));
//     
//     m_accountButtonsLayout->addWidget(bInfo);
//     m_accountButtonsLayout->addWidget(bErr);
}

void MainWidget::systemMessageTest()
{
    if(sender()->objectName() == "infoBt")
        showMessageToUser("Info message...", MainWidget::SystemMessageInfo);
    else showMessageToUser("Error message...", MainWidget::SystemMessageError);
    
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
    //TODO: Add some 'working' indicator
    kDebug() << "Connection status is" << status;
    switch (status) {
    case Tp::ConnectionStatusConnecting:
         showMessageToUser(i18n("Connecting..."), MainWidget::SystemMessageInfo);
        break;
    case Tp::ConnectionStatusConnected:
        showMessageToUser(i18n("Connected!"), MainWidget::SystemMessageInfo);
        break;
    case Tp::ConnectionStatusDisconnected:
        showMessageToUser(i18n("Disconnected!"), MainWidget::SystemMessageInfo);
        break;
    default:
        break;
    }
}

void MainWidget::onConnectionChanged(const Tp::ConnectionPtr& connection)
{
    Q_UNUSED(connection);
//    m_contactsListView->expandAll();
    //Tp::AccountPtr account(qobject_cast<Tp::Account*>(sender()));
    kDebug();
}

void MainWidget::onContactListDoubleClick(const QModelIndex& index)
{
    if(!index.isValid()) {
        return;
    }
    
    if(index.data(AccountsModel::AliasRole).toString().isEmpty()) {
        if(m_contactsListView->isExpanded(index))
            m_contactsListView->collapse(index);
        else m_contactsListView->expand(index);
    }
    else {
        kDebug() << "Text chat requested for index" << index;
        startTextChannel(index);    
    }
}

void MainWidget::startTextChannel(const QModelIndex &index)
{
    if (! index.isValid()) {
        return;
    }
    
    QModelIndex realIndex = m_modelFilter->mapToSource(index);
    Tp::ContactPtr contact = m_model->contactForIndex(realIndex);
    kDebug() << "Requesting chat for contact" << contact->alias();
    
    Tp::AccountPtr account = m_model->accountForContactIndex(realIndex);
    
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

void MainWidget::showMessageToUser(const QString& text, const MainWidget::SystemMessageType type)
{
    //kDebug() << m_contactsListView->size() << m_contactsListView->viewport()->size();
    QFrame *msgFrame = new QFrame(m_contactsListView);
    msgFrame->setAttribute(Qt::WA_DeleteOnClose);
    msgFrame->setMinimumSize(QSize(m_contactsListView->viewport()->width(), 55));
    msgFrame->setFrameShape(QFrame::Box);
    msgFrame->setFrameShadow(QFrame::Plain);
    msgFrame->setAutoFillBackground(true);
    msgFrame->setLineWidth(1);
    
    if(type == MainWidget::SystemMessageError) {
        msgFrame->setStyleSheet("background-color: #FFCBCB; color: #FF2222;");
    }
    else if(type == MainWidget::SystemMessageInfo) {
        msgFrame->setStyleSheet("color: #2222FF;");
    }
    
    QHBoxLayout *layout = new QHBoxLayout(msgFrame);
    QVBoxLayout *closeBtLayout = new QVBoxLayout(msgFrame);
    
    QLabel *message = new QLabel(text, msgFrame);
    message->setAlignment(Qt::AlignVCenter);
    
    QToolButton *closeButton = new QToolButton(msgFrame);
    closeButton->setText("x");
    closeButton->setAutoRaise(true);
    closeButton->setMaximumSize(QSize(16,16));
    
    connect(closeButton, SIGNAL(clicked(bool)), msgFrame, SLOT(close()));
    
    closeBtLayout->addWidget(closeButton);
    closeBtLayout->addStretch(-1);
    
    layout->addWidget(message);
    layout->addLayout(closeBtLayout);
    
    msgFrame->show();
    
    QPropertyAnimation *a = new QPropertyAnimation(msgFrame, "pos");
    a->setParent(msgFrame);
    a->setDuration(4000);
    a->setEasingCurve(QEasingCurve::OutExpo);
    a->setStartValue(QPointF(m_contactsListView->viewport()->pos().x(), 
                             m_contactsListView->viewport()->pos().y()+m_contactsListView->viewport()->height()));
    
    a->setEndValue(QPointF(m_contactsListView->viewport()->pos().x(), 
                           m_contactsListView->viewport()->pos().y()+m_contactsListView->viewport()->height()-50));
    a->start();
    
    if(type == MainWidget::SystemMessageInfo) {
        QTimer::singleShot(4500, msgFrame, SLOT(close()));
    }
    
}

void MainWidget::addOverlayButtons()
{
        TextChannelContactOverlay*  textOverlay = new TextChannelContactOverlay(this);
        AudioChannelContactOverlay* audioOverlay = new AudioChannelContactOverlay(this);
        VideoChannelContactOverlay* videoOverlay = new VideoChannelContactOverlay(this);
        
        FileTransferContactOverlay* fileOverlay = new FileTransferContactOverlay(this);
        
        m_delegate->installOverlay(textOverlay);
        m_delegate->installOverlay(audioOverlay);
        m_delegate->installOverlay(videoOverlay);
        m_delegate->installOverlay(fileOverlay);
        
        textOverlay->setView(m_contactsListView);
        textOverlay->setActive(true);
        
        audioOverlay->setView(m_contactsListView);
        audioOverlay->setActive(true);
        
        videoOverlay->setView(m_contactsListView);
        videoOverlay->setActive(true);
        
        fileOverlay->setView(m_contactsListView);
        fileOverlay->setActive(true);
        
        connect(textOverlay, SIGNAL(overlayActivated(QModelIndex)),
                m_delegate, SLOT(hideStatusMessageSlot(QModelIndex)));
        
        connect(textOverlay, SIGNAL(overlayHidden()),
                m_delegate, SLOT(reshowStatusMessageSlot()));
        
        connect(textOverlay, SIGNAL(activated(QModelIndex)),
                this, SLOT(startTextChannel(QModelIndex)));
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
//     QModelIndex index = m_currentModel->mapToSource(proxyIdx);
//     if (!index.isValid()) {
//         kDebug() << "Could not map to source";
//         // Flee
//         return;
//     }
// 
//     // Ok, now let's guess
//     AbstractTreeItem *abstractItem = static_cast<AbstractTreeItem*>(index.internalPointer());
//     kDebug() << index << index.internalPointer() << abstractItem;
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
//     QModelIndex index = m_currentModel->mapToSource(m_contactsListView->currentIndex());
//     if (!index.isValid()) {
//         // Flee
//         kDebug() << "Invalid index";
//         return;
//     }
// 
//     // Ok, what is it?
//     AbstractTreeItem *abstractItem = static_cast<AbstractTreeItem*>(index.internalPointer());
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
//     QModelIndex index = m_currentModel->mapToSource(m_contactsListView->currentIndex());
//     if (!index.isValid()) {
//         // Flee
//         kDebug() << "Invalid index";
//         return;
//     }
// 
//     // Ok, what is it?
//     AbstractTreeItem *abstractItem = static_cast<AbstractTreeItem*>(index.internalPointer());
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
//     QModelIndex index = m_currentModel->mapToSource(m_contactsListView->currentIndex());
//     if (!index.isValid()) {
//         // Flee
//         kDebug() << "Invalid index";
//         return;
//     }
// 
//     // Ok, what is it?
//     AbstractTreeItem *abstractItem = static_cast<AbstractTreeItem*>(index.internalPointer());
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
//     QModelIndex index = m_currentModel->mapToSource(m_contactsListView->currentIndex());
//     if (!index.isValid()) {
//         // Flee
//         kDebug() << "Invalid index";
//         return;
//     }
// 
//     // Ok, what is it?
//     AbstractTreeItem *abstractItem = static_cast<AbstractTreeItem*>(index.internalPointer());
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
//     QModelIndex index = m_currentModel->mapToSource(m_contactsListView->currentIndex());
//     if (!index.isValid()) {
//         // Flee
//         kDebug() << "Invalid index";
//         return;
//     }
// 
//     // Ok, what is it?
//     AbstractTreeItem *abstractItem = static_cast<AbstractTreeItem*>(index.internalPointer());
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
//     QModelIndex index = m_currentModel->mapToSource(m_contactsListView->currentIndex());
//     if (!index.isValid()) {
//         // Flee
//         kDebug() << "Invalid index";
//         return;
//     }
// 
//     RequestTextChatJob* job;
//     // Ok, what is it?
//     AbstractTreeItem *abstractItem = static_cast<AbstractTreeItem*>(index.internalPointer());
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
