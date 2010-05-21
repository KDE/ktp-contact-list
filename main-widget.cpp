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

#include "main-widget.h"

#include "contacts-list-model.h"
#include "grouped-contacts-proxy-model.h"
#include "contact-item.h"
#include "meta-contact-item.h"

#include "telepathy-bridge.h"

#include <nco.h>
#include <telepathy.h>
#include <informationelement.h>

#include <QtGui/QSortFilterProxyModel>
#include <QtGui/QPainter>
#include <QtGui/QMenu>
#include <QtGui/QLabel>
#include <QtGui/QCheckBox>

#include <KDebug>
#include <KJob>
#include <KLineEdit>
#include <KComboBox>
#include <KMessageBox>

#include <Nepomuk/ResourceManager>
#include <Nepomuk/Variant>
#include <Nepomuk/Query/QueryServiceClient>
#include <Nepomuk/Query/ComparisonTerm>
#include <Nepomuk/Query/ResourceTerm>
#include <Nepomuk/Query/ResourceTypeTerm>
#include <Nepomuk/Query/LiteralTerm>
#include <Nepomuk/Query/AndTerm>
#include <Nepomuk/Query/Result>

#include <pimo.h>
#include <nao.h>

#include <TelepathyQt4/Constants>

const int SPACING = 4;
const int AVATAR_SIZE = 32;

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

    QRect iconRect = optV4.rect;
    iconRect.setSize(QSize(32, 32));
    iconRect.moveTo(QPoint(iconRect.x() + SPACING, iconRect.y() + SPACING));

    const QPixmap pixmap = idx.data(ContactsListModel::AvatarRole).value<QPixmap>();
    if (!pixmap.isNull()) {
        painter->drawPixmap(iconRect, idx.data(ContactsListModel::AvatarRole).value<QPixmap>());
    }

    painter->save();

    QFont nameFont = painter->font();
    nameFont.setWeight(QFont::Bold);
    painter->setFont(nameFont);

    QRect textRect = optV4.rect;
    textRect.setX(iconRect.x() + iconRect.width() + SPACING);
    textRect = painter->boundingRect(textRect, Qt::AlignLeft | Qt::AlignTop, optV4.text);
    //textRect.setWidth(optV4.rect.width() / 2);

    painter->drawText(textRect, optV4.text);

    painter->restore();

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
    return QSize(0, 32 + 2 * SPACING);
}

MainWidget::MainWidget(QWidget *parent)
 : QWidget(parent),
   m_model(0),
   m_groupedContactsProxyModel(0),
   m_sortFilterProxyModel(0)
{
    kDebug();

    // Check if Nepomuk Query service client is up and running
    if (!Nepomuk::Query::QueryServiceClient::serviceAvailable()) {
        // That's a failure
        KMessageBox::error(this, i18n("The Nepomuk Query Service client is not available on your system. "
                                      "Contactlist requires the query service client to be available: please "
                                      "check your system settings"));
    }

    setupUi(this);

    // Initialize Telepathy
    TelepathyBridge::instance()->init();
    connect(TelepathyBridge::instance(),
            SIGNAL(ready(bool)),
            SLOT(onHandlerReady(bool)));

    m_model = new ContactsListModel(this);
    m_sortFilterProxyModel = new QSortFilterProxyModel(this);
    m_sortFilterProxyModel->setSourceModel(m_model);

    m_groupedContactsProxyModel = new GroupedContactsProxyModel(this);
    m_groupedContactsProxyModel->setSourceModel(m_model);

    m_contactsListView->setSortingEnabled(true);
    m_contactsListView->setContextMenuPolicy(Qt::CustomContextMenu);
    m_contactsListView->setModel(m_groupedContactsProxyModel);
    m_contactsListView->setItemDelegate(new ContactDelegate(this));
    connect(m_contactsListView, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(onCustomContextMenuRequested(QPoint)));
    connect(actionAdd_contact, SIGNAL(triggered(bool)),
            this, SLOT(onAddContactRequest(bool)));

    // Get 'me' as soon as possible
    // FIXME: Port to new OSCAF standard for accessing "me" as soon as it
    // becomes available.
    Nepomuk::Thing me(QUrl::fromEncoded("nepomuk:/myself"));

    // Loop through all the grounding instances of this person
    foreach (Nepomuk::InformationElement resource, me.groundingOccurrences()) {
        // See if this grounding instance is of type nco:contact.
        if (resource.hasType(Nepomuk::Vocabulary::NCO::PersonContact())) {
            // FIXME: We are going to assume the first NCO::PersonContact is the
            // right one. Can we improve this?
            m_mePersonContact = resource;
            break;
        }
    }
}

MainWidget::~MainWidget()
{
    kDebug();
}

void MainWidget::onHandlerReady(bool ready)
{
    if (!ready) {
        kWarning() << "Telepathy handler could not become ready!";
    } else {
        kDebug() << "Telepathy handler ready";
    }
}

void MainWidget::onCustomContextMenuRequested(const QPoint& point)
{
    QModelIndex proxyIdx = m_contactsListView->indexAt(point);
    if (!proxyIdx.isValid()) {
        kDebug() << "Invalid index provided";
        // Flee
        return;
    }

    // Map the index to the real model
    QModelIndex idx = m_groupedContactsProxyModel->mapToSource(proxyIdx);
    if (!idx.isValid()) {
        kDebug() << "Could not map to source";
        // Flee
        return;
    }

    // Ok, now let's guess
    AbstractTreeItem *abstractItem = static_cast<AbstractTreeItem*>(idx.internalPointer());
    kDebug() << idx << idx.internalPointer() << abstractItem;
    ContactItem *contactItem = dynamic_cast<ContactItem*>(abstractItem);
    kDebug() << contactItem;
    QMenu *menu = new QMenu;

    if (contactItem) {
        kDebug() << "A contactitem";
        // Ok, now let's see what we can do
        if (!contactItem->groups().isEmpty()) {
            QMenu *parentAction = new QMenu(i18n("Remove from group"));
            menu->addMenu(parentAction);
            foreach (const QString &group, contactItem->groups()) {
                QAction *action = parentAction->addAction(group);
                connect(action, SIGNAL(triggered(bool)),
                        SLOT(onRequestRemoveFromGroup(bool)));
            }
        }

        // Ok, can we also add this contact to some other groups?
        QStringList allGroups = TelepathyBridge::instance()->knownGroupsFor(contactItem->personContact());
        kDebug() << "All groups: " << allGroups;

        QStringList nonJoinedGroups;
        foreach (const QString &group, allGroups) {
            if (!contactItem->groups().contains(group)) {
                nonJoinedGroups << group;
            }
        }

        // Ok, now let's see what we can do
        if (!nonJoinedGroups.isEmpty()) {
            QMenu *parentAction = new QMenu(i18n("Add to group"));
            menu->addMenu(parentAction);
            foreach (const QString &group, nonJoinedGroups) {
                QAction *action = parentAction->addAction(group);
                connect(action, SIGNAL(triggered(bool)),
                        SLOT(onRequestAddToGroup(bool)));
            }
        }

        // Add/remove to metacontacts
        // First of all: can it be added to a metacontact?
        bool canAddToMetaContact = false;
        MetaContactItem *metaContactItem = dynamic_cast<MetaContactItem*>(contactItem->parentItem());
        if (metaContactItem) {
            canAddToMetaContact = metaContactItem->type() == MetaContactItem::FakeMetaContact ? true : false;
        } else {
            canAddToMetaContact = true;
        }

        if (canAddToMetaContact) {
            // List available metacontacts
            QList<Nepomuk::Query::Result> results;
            {
                using namespace Nepomuk::Query;

                Query query(ResourceTypeTerm(Nepomuk::Vocabulary::PIMO::Person()));

                bool queryResult = true;
                results = QueryServiceClient::syncQuery(query, &queryResult);

                if (!queryResult) {
                    KMessageBox::error(0, i18n("It was not possible to query Nepomuk database. Please check your "
                                               "installation and make sure Nepomuk is running."));
                }
            }

            QMenu *parentAction = new QMenu(i18n("Add to metacontact"));
            menu->addMenu(parentAction);
            // Iterate over all the IMAccounts found.
            foreach (const Nepomuk::Query::Result &result, results) {
                Nepomuk::Person foundPerson(result.resource());
                kDebug() << foundPerson;
                QAction *action = parentAction->addAction(foundPerson.genericLabel());
                connect(action, SIGNAL(triggered(bool)),
                        this, SLOT(onAddToMetaContact(bool)));
            }

            QAction *action = parentAction->addAction(i18nc("Adds a new metacontact", "Add new..."));
            connect(action, SIGNAL(triggered(bool)),
                    this, SLOT(onAddToMetaContact(bool)));
        } else {
            // We can remove it from a metacontact instead
            QAction *action = menu->addAction(i18n("Remove from metacontact"));
            connect(action, SIGNAL(triggered(bool)),
                    this, SLOT(onRemoveFromMetacontact(bool)));
        }
    }

    MetaContactItem *metaContactItem = dynamic_cast<MetaContactItem*>(abstractItem);
    kDebug() << metaContactItem;
    if (metaContactItem) {
        kDebug() << "A metacontactitem";
        // Ok, now let's see what we can do
        if (!metaContactItem->groups().isEmpty()) {
            QMenu *parentAction = new QMenu(i18n("Remove from group"));
            menu->addMenu(parentAction);
            foreach (const QString &group, metaContactItem->groups()) {
                QAction *action = parentAction->addAction(group);
                connect(action, SIGNAL(triggered(bool)),
                        SLOT(onRequestRemoveFromGroup(bool)));
            }
        }

        // Ok, can we also add this contact to some other groups?
        QStringList allGroups = TelepathyBridge::instance()->knownGroupsFor(metaContactItem->pimoPerson());
        kDebug() << "All groups: " << allGroups;

        QStringList nonJoinedGroups;
        foreach (const QString &group, allGroups) {
            if (!contactItem->groups().contains(group)) {
                nonJoinedGroups << group;
            }
        }

        // Ok, now let's see what we can do
        if (!nonJoinedGroups.isEmpty()) {
            QMenu *parentAction = new QMenu(i18n("Add to group"));
            menu->addMenu(parentAction);
            foreach (const QString &group, nonJoinedGroups) {
                QAction *action = parentAction->addAction(group);
                connect(action, SIGNAL(triggered(bool)),
                        SLOT(onRequestAddToGroup(bool)));
            }
        }
    }

    // And of course remove the contact
    QAction *removeContact = menu->addAction(KIcon("list-remove"), i18n("Remove"));
    QAction *blockContact = menu->addAction(KIcon("dialog-cancel"), i18n("Block"));

    connect(removeContact, SIGNAL(triggered(bool)),
            SLOT(onContactRemovalRequest(bool)));
    connect(blockContact, SIGNAL(triggered(bool)),
            SLOT(onContactBlockRequest(bool)));

    menu->exec(m_contactsListView->mapToGlobal(point));
    menu->deleteLater();
}

void MainWidget::onRequestRemoveFromGroup(bool )
{
    QAction *action = qobject_cast< QAction* >(sender());
    if (!action) {
        kDebug() << "invalid";
        return;
    }

    kDebug() << "Request removal from group " << action->text();

    // Pick the current model index
    QModelIndex idx = m_groupedContactsProxyModel->mapToSource(m_contactsListView->currentIndex());
    if (!idx.isValid()) {
        // Flee
        kDebug() << "Invalid index";
        return;
    }

    // Ok, what is it?
    AbstractTreeItem *abstractItem = static_cast<AbstractTreeItem*>(idx.internalPointer());
    ContactItem *contactItem = dynamic_cast<ContactItem*>(abstractItem);

    if (contactItem) {
        // Remove the contact
        KJob *job = TelepathyBridge::instance()->removeContactFromGroup(action->text(), contactItem->personContact());
        QEventLoop e;
        connect(job, SIGNAL(finished(KJob*)), &e, SLOT(quit()));
        job->start();
        kDebug() << "Running job...";
        e.exec();
        kDebug() << "Job run, "<< job->error();
        kDebug() << "rm contacts";
    }

    MetaContactItem *metaContactItem = dynamic_cast<MetaContactItem*>(abstractItem);
    if (metaContactItem) {
        // Remove the metacontact
        TelepathyBridge::instance()->removeMetaContactFromGroup(action->text(), metaContactItem->pimoPerson());
        kDebug() << "rm contacts";
    }
}

void MainWidget::onRequestAddToGroup(bool )
{
    QAction *action = qobject_cast< QAction* >(sender());
    if (!action) {
        kDebug() << "invalid";
        return;
    }

    kDebug() << "Request addition group " << action->text();

    // Pick the current model index
    QModelIndex idx = m_groupedContactsProxyModel->mapToSource(m_contactsListView->currentIndex());
    if (!idx.isValid()) {
        // Flee
        kDebug() << "Invalid index";
        return;
    }

    // Ok, what is it?
    AbstractTreeItem *abstractItem = static_cast<AbstractTreeItem*>(idx.internalPointer());
    ContactItem *contactItem = dynamic_cast<ContactItem*>(abstractItem);

    if (contactItem) {
        // Remove the contact
        KJob *job = TelepathyBridge::instance()->addContactToGroup(action->text(), contactItem->personContact());
        QEventLoop e;
        connect(job, SIGNAL(finished(KJob*)), &e, SLOT(quit()));
        job->start();
        kDebug() << "Running job...";
        e.exec();
        kDebug() << "Job run, "<< job->error();
    }

    MetaContactItem *metaContactItem = dynamic_cast<MetaContactItem*>(abstractItem);
    if (metaContactItem) {
        // Remove the metacontact
        TelepathyBridge::instance()->removeMetaContactFromGroup(action->text(), metaContactItem->pimoPerson());
    }
}

void MainWidget::onContactBlockRequest(bool )
{

}

void MainWidget::onRemoveFromMetacontact(bool )
{
    // Pick the current model index
    QModelIndex idx = m_groupedContactsProxyModel->mapToSource(m_contactsListView->currentIndex());
    if (!idx.isValid()) {
        // Flee
        kDebug() << "Invalid index";
        return;
    }

    // Ok, what is it?
    AbstractTreeItem *abstractItem = static_cast<AbstractTreeItem*>(idx.internalPointer());
    ContactItem *contactItem = dynamic_cast<ContactItem*>(abstractItem);

    Q_ASSERT(contactItem);

    KJob *job = TelepathyBridge::instance()->removeContact(contactItem->personContact(),
                                                           TelepathyBridge::RemoveFromMetacontactMode);

    QEventLoop e;
    connect(job, SIGNAL(finished(KJob*)), &e, SLOT(quit()));
    job->start();
    qDebug() << "Running job...";
    e.exec();
    qDebug() << "Job run, "<< job->error();
}

void MainWidget::onContactRemovalRequest(bool )
{
    QAction *action = qobject_cast< QAction* >(sender());
    if (!action) {
        kDebug() << "invalid";
        return;
    }

    kDebug() << "Request addition group " << action->text();

    // Pick the current model index
    QModelIndex idx = m_groupedContactsProxyModel->mapToSource(m_contactsListView->currentIndex());
    if (!idx.isValid()) {
        // Flee
        kDebug() << "Invalid index";
        return;
    }

    // Ok, what is it?
    AbstractTreeItem *abstractItem = static_cast<AbstractTreeItem*>(idx.internalPointer());
    ContactItem *contactItem = dynamic_cast<ContactItem*>(abstractItem);

    if (contactItem) {
        // Build a dialog
        KDialog *dial = new KDialog(this);
        QWidget *w = new QWidget;
        QLabel *l = new QLabel(i18n("Please select the removal means for this contact"));
        QCheckBox *presence = 0;
        QCheckBox *subscription = 0;
        QCheckBox *block = 0;
        QVBoxLayout *lay = new QVBoxLayout;
        lay->addWidget(l);

        // What can we do?
        TelepathyBridge::RemovalModes modes = TelepathyBridge::instance()->supportedRemovalModesFor(contactItem->personContact());

        if (modes & TelepathyBridge::RemovePublicationMode) {
            presence = new QCheckBox(i18n("Don't show me in his buddy list anymore"));
            // On by default
            presence->setCheckState(Qt::Checked);
            lay->addWidget(presence);
        }
        if (modes & TelepathyBridge::RemoveSubscriptionMode) {
            subscription = new QCheckBox(i18n("Don't show him in my buddy list anymore"));
            // On by default
            subscription->setCheckState(Qt::Checked);
            lay->addWidget(subscription);
        }
        if (modes & TelepathyBridge::BlockMode) {
            block = new QCheckBox(i18n("Block him"));
            // Off by default
            block->setCheckState(Qt::Unchecked);
            lay->addWidget(block);
        }

        w->setLayout(lay);
        dial->setMainWidget(w);

        if (dial->exec() == KDialog::Accepted) {
            TelepathyBridge::RemovalModes execModes = 0;
            if (presence) {
                if (presence->isChecked()) {
                    if (execModes == 0) {
                        execModes = TelepathyBridge::RemovePublicationMode;
                    } else {
                        execModes |= TelepathyBridge::RemovePublicationMode;
                    }
                }
            }
            if (subscription) {
                if (subscription->isChecked()) {
                    if (execModes == 0) {
                        execModes = TelepathyBridge::RemoveSubscriptionMode;
                    } else {
                        execModes |= TelepathyBridge::RemoveSubscriptionMode;
                    }
                }
            }
            if (block) {
                if (block->isChecked()) {
                    if (execModes == 0) {
                        execModes = TelepathyBridge::BlockMode;
                    } else {
                        execModes |= TelepathyBridge::BlockMode;
                    }
                }
            }

            if (execModes == 0) {
                qDebug() << "Nothing to do!";
                return;
            }

            // Remove the contact
            KJob *job = TelepathyBridge::instance()->removeContact(contactItem->personContact(), execModes);
            QEventLoop e;
            connect(job, SIGNAL(finished(KJob*)), &e, SLOT(quit()));
            job->start();
            kDebug() << "Running job...";
            e.exec();
            kDebug() << "Job run, "<< job->error();
        }
    }
}

void MainWidget::onAddContactRequest(bool )
{
    // Let's build a dialog
    KDialog *dial = new KDialog(this);
    QWidget *w = new QWidget;
    QLabel *l = new QLabel(i18n("Please enter the ID of the contact in question"));
    KComboBox *account = new KComboBox();
    KLineEdit *contactId = new KLineEdit();
    QVBoxLayout *lay = new QVBoxLayout;
    lay->addWidget(l);
    lay->addWidget(account);
    lay->addWidget(contactId);

    // Get all valid Telepathy accounts
    QList<Nepomuk::Query::Result> results;
    {
        using namespace Nepomuk::Query;

        // me must have an IMAccount
        ComparisonTerm imterm(Nepomuk::Vocabulary::NCO::hasIMAccount(),
                              ResourceTerm(m_mePersonContact));
        imterm.setInverted(true);

        // Which must be an IMAccount of course
        Query query(AndTerm(ResourceTypeTerm(Nepomuk::Vocabulary::NCO::IMAccount()),
                            imterm));

        bool queryResult = true;
        results = QueryServiceClient::syncQuery(query, &queryResult);

        if (!queryResult) {
            KMessageBox::error(0, i18n("It was not possible to query Nepomuk database. Please check your "
                                        "installation and make sure Nepomuk is running."));
        }
    }

    // Iterate over all the IMAccounts/PersonContacts found.
    foreach (const Nepomuk::Query::Result &result, results) {
        Nepomuk::IMAccount foundIMAccount(result.resource());
        uint statusType = foundIMAccount.statusTypes().first();
        if( statusType != Tp::ConnectionPresenceTypeUnset   &&
            statusType != Tp::ConnectionPresenceTypeOffline &&
            statusType != Tp::ConnectionPresenceTypeUnknown &&
            statusType != Tp::ConnectionPresenceTypeError)
        {
            foreach (const QString &id, foundIMAccount.imIDs()) {
                account->addItem(id, foundIMAccount.resourceUri());
            }
        }
    }

    w->setLayout(lay);
    dial->setMainWidget(w);

    if (dial->exec() == KDialog::Accepted) {
        // Add the contact
        Nepomuk::IMAccount toAddAccount(account->itemData(account->currentIndex()).toUrl());
        KJob *job = TelepathyBridge::instance()->addContact(toAddAccount, contactId->text());
        QEventLoop e;
        connect(job, SIGNAL(finished(KJob*)), &e, SLOT(quit()));
        job->start();
        qDebug() << "Running job...";
        e.exec();
        qDebug() << "Job run, "<< job->error();
    }
}

void MainWidget::onAddToMetaContact(bool )
{
    QAction *action = qobject_cast< QAction* >(sender());
    if (!action) {
        kDebug() << "invalid";
        return;
    }

    QString metaContactName = action->text();
    kDebug() << "Request adding to metacontact " << metaContactName;

    // Pick the current model index
    QModelIndex idx = m_groupedContactsProxyModel->mapToSource(m_contactsListView->currentIndex());
    if (!idx.isValid()) {
        // Flee
        kDebug() << "Invalid index";
        return;
    }

    // Ok, what is it?
    AbstractTreeItem *abstractItem = static_cast<AbstractTreeItem*>(idx.internalPointer());
    ContactItem *contactItem = dynamic_cast<ContactItem*>(abstractItem);

    Q_ASSERT(contactItem);

    if (metaContactName == i18nc("Adds a new metacontact", "Add new...")) {
        // Prompt to create a new metacontact
        // Let's build a dialog
        KDialog *dial = new KDialog(this);
        QWidget *w = new QWidget;
        QLabel *l = new QLabel(i18n("Please enter a name for this new metacontact"));
        KLineEdit *contactId = new KLineEdit();
        QVBoxLayout *lay = new QVBoxLayout;
        lay->addWidget(l);
        lay->addWidget(contactId);

        w->setLayout(lay);
        dial->setMainWidget(w);

        if (dial->exec() == KDialog::Accepted) {
            // Add the contact
            metaContactName = contactId->text();
            KJob *job = TelepathyBridge::instance()->addMetaContact(contactId->text(),
                                                                    QList< Nepomuk::PersonContact >() <<
                                                                    contactItem->personContact());
            QEventLoop e;
            connect(job, SIGNAL(finished(KJob*)), &e, SLOT(quit()));
            job->start();
            qDebug() << "Running job...";
            e.exec();
            qDebug() << "Job run, "<< job->error();
        }

        return;
    }

    // Ok, now let's add the contact
    QList< Nepomuk::Query::Result > results;
    {
        using namespace Nepomuk::Query;
        using namespace Nepomuk::Vocabulary;

        ResourceTypeTerm rtterm(PIMO::Person());
        ComparisonTerm cmpterm(NAO::prefLabel(), LiteralTerm(metaContactName));

        Query query(AndTerm(cmpterm, rtterm));

        bool queryResult = true;
        results = QueryServiceClient::syncQuery(query, &queryResult);

        if (!queryResult) {
            KMessageBox::error(0, i18n("It was not possible to query Nepomuk database. Please check your "
                                       "installation and make sure Nepomuk is running."));
        }
    }

    // Iterate over all the IMAccounts found.
    foreach (const Nepomuk::Query::Result &result, results) {
        Nepomuk::Person foundPerson(result.resource());
        foundPerson.addGroundingOccurrence(contactItem->personContact());
    }
}

#include "main-widget.moc"

