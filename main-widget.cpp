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
#include <KUser>

#include "main-widget.h"
#include "ui_main-widget.h"
#include "account-button.h"
#include "contact-overlays.h"
#include "accounts-model.h"
#include "account-filter-model.h"
#include "contact-delegate.h"
#include "contact-model-item.h"

#define PREFERRED_TEXTCHAT_HANDLER "org.freedesktop.Telepathy.Client.KDE.TextUi"

MainWidget::MainWidget(QWidget *parent)
    : KMainWindow(parent),
   m_model(0),
   m_modelFilter(0)
{
    Tp::registerTypes();
    KUser user;

    setupUi(this);
    m_filterBar->hide();
    setWindowIcon(KIcon("telepathy"));

    m_userAccountIconButton->setIcon(QIcon(QPixmap::fromImage(QImage(user.faceIconPath()))));
    m_userAccountNameLabel->setText(user.property(KUser::FullName).toString());
//     m_toolBar->hide();

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

    m_actionSearch_contact->setIcon(KIcon("edit-find-user"));
    m_actionSearch_contact->setText(QString());
    m_actionSearch_contact->setToolTip(i18n("Find contact"));

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

    m_accountManager = Tp::AccountManager::create(QDBusConnection::sessionBus(),
                                                  accountFactory,
                                                  connectionFactory,
                                                  channelFactory,
                                                  contactFactory);

    connect(m_accountManager->becomeReady(),
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onAccountManagerReady(Tp::PendingOperation*)));

    connect(m_accountManager.data(), SIGNAL(newAccount(Tp::AccountPtr)),
            this, SLOT(onNewAccountAdded(Tp::AccountPtr)));

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

    connect(m_actionSearch_contact, SIGNAL(triggered(bool)),
            this, SLOT(toggleSearchWidget(bool)));

    connect(m_presenceMessageEdit, SIGNAL(returnPressed(QString)),
            this, SLOT(setCustomPresenceMessage(QString)));
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
    m_modelFilter->clearFilterString();
    m_modelFilter->setSortRole(Qt::DisplayRole);
    m_contactsListView->setModel(m_modelFilter);
    m_contactsListView->setSortingEnabled(true);
    m_contactsListView->sortByColumn(0, Qt::AscendingOrder);

    connect(m_actionHide_offline, SIGNAL(toggled(bool)),
            m_modelFilter, SLOT(filterOfflineUsers(bool)));

    connect(m_filterBar, SIGNAL(filterChanged(QString)),
            m_modelFilter, SLOT(setFilterString(QString)));

    connect(m_filterBar, SIGNAL(closeRequest()),
            m_modelFilter, SLOT(clearFilterString()));

    connect(m_filterBar, SIGNAL(closeRequest()),
            m_filterBar, SLOT(hide()));

    connect(m_filterBar, SIGNAL(closeRequest()),
            m_actionSearch_contact, SLOT(toggle()));

    m_accountButtonsLayout->insertStretch(-1);

    QList<Tp::AccountPtr> accounts = m_accountManager->allAccounts();
    foreach (Tp::AccountPtr account, accounts) {
        onNewAccountAdded(account);
    }
    m_contactsListView->expandAll();
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
    kDebug() << "Connection status is" << status;
    switch (status) {
    case Tp::ConnectionStatusConnected:
        //FIXME: Get the account (sender()) index and expand only that index
        m_contactsListView->expandAll();
        break;
    case Tp::ConnectionStatusDisconnected:
        // This will make sure that the empty account groups will be hidden
        m_modelFilter->refilter();
        break;
    //Fall through
    case Tp::ConnectionStatusConnecting:
    default:
        break;
    }
}

void MainWidget::onNewAccountAdded(const Tp::AccountPtr& account)
{
    account->becomeReady();

    connect(account.data(),
            SIGNAL(connectionChanged(Tp::ConnectionPtr)),
            this, SLOT(onConnectionChanged(Tp::ConnectionPtr)));

    connect(account.data(),
            SIGNAL(connectionStatusChanged(Tp::ConnectionStatus)), 
            this, SLOT(onAccountConnectionStatusChanged(Tp::ConnectionStatus)));

    connect(account.data(), SIGNAL(stateChanged(bool)),
            this, SLOT(onAccountStateChanged(bool)));

    connect(account.data(),
            SIGNAL(removed()),
            this, SLOT(onAccountRemoved()));

    AccountButton *bt = new AccountButton(account, this);
    bt->setObjectName(account->uniqueIdentifier());
    bt->hide();

    m_accountButtonsLayout->insertWidget(m_accountButtonsLayout->count() - 1, bt);

    if(account->isEnabled()) {
        bt->show();
    }
}

void MainWidget::onAccountStateChanged(bool enabled)
{
    Tp::AccountPtr account(static_cast<Tp::Account*>(sender()));

    if(enabled) {
        findChild<AccountButton *>(account->uniqueIdentifier())->show();
    }
    else {
        findChild<AccountButton *>(account->uniqueIdentifier())->hide();
        showMessageToUser(i18n("Account %1 was disabled!").arg(account->displayName()), 
                          MainWidget::SystemMessageError);
    }
}

void MainWidget::onAccountRemoved()
{
    Tp::AccountPtr account(static_cast<Tp::Account*>(sender()));
    delete findChild<AccountButton *>(account->uniqueIdentifier());

    showMessageToUser(i18n("Account %1 was removed!").arg(account->displayName()), 
                      MainWidget::SystemMessageError);
}

void MainWidget::onConnectionChanged(const Tp::ConnectionPtr& connection)
{
    Q_UNUSED(connection);
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
    Tp::ContactPtr contact = static_cast<ContactModelItem*>(
        qVariantValue<QObject *>(m_model->data(realIndex, AccountsModel::ItemRole)))->contact();

    kDebug() << "Requesting chat for contact" << contact->alias();

    Tp::AccountPtr account = m_model->accountForContactIndex(realIndex);

    Tp::PendingChannelRequest* channelRequest = account->ensureTextChat(contact,
                                                                        QDateTime::currentDateTime(),
                                                                        PREFERRED_TEXTCHAT_HANDLER);
    connect(channelRequest, SIGNAL(finished(Tp::PendingOperation*)), SLOT(onChannelJoined(Tp::PendingOperation*)));
}

void MainWidget::onChannelJoined(Tp::PendingOperation* op)
{
    if (op->isError()) {
        kDebug() << op->errorName();
        kDebug() << op->errorMessage();
    }
}

void MainWidget::showMessageToUser(const QString& text, const MainWidget::SystemMessageType type)
{
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

void MainWidget::toggleSearchWidget(bool show)
{
    if(show) {
        m_filterBar->show();
    }
    else {
        m_modelFilter->clearFilterString();
        m_filterBar->clear();
        m_filterBar->hide();
    }
}

void MainWidget::onCustomContextMenuRequested(const QPoint& point)
{
    Q_UNUSED(point);
}

void MainWidget::setCustomPresenceMessage(const QString& message)
{
    for (int i = 0; i < m_accountButtonsLayout->count() - 1; i++) {
        qobject_cast<AccountButton*>(m_accountButtonsLayout->itemAt(i)->widget())->setCustomPresenceMessage(message);

    }

    m_presenceMessageEdit->clearFocus();
}