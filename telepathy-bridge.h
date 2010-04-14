/*
 * This file is part of telepathy-contactslist-prototype
 *
 * Copyright (C) 2009-2010 Collabora Ltd. <info@collabora.co.uk>
 *   @Author Dario Freddi <dario.freddi@collabora.co.uk>
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

#ifndef TELEPATHY_BRIDGE_H
#define TELEPATHY_BRIDGE_H

#include <QtCore/QObject>

namespace Nepomuk {
class PersonContact;
class Person;
class IMAccount;
}

namespace Tp {
class Contact;
class PendingOperation;
}

class KJob;

class QUrl;
class QStringList;

class TelepathyBridgePrivate;
/**
 * @class TelepathyBridge telepathy-bridge.h
 * @brief Wrapper class providing an easy to use bridge between Nepomuk and Telepathy
 *
 * When creating a KDE based telepathy client, you will be mostly using Nepomuk resources to
 * access information about accounts, contacts and such. However, when having to deal with local
 * changes (a contact being removed/added from a group, etc...), you will need to access Telepathy directly.
 *
 * \b TelepathyBridge is a wrapper class which providing a very high level interface to most common
 * Telepathy actions based upon Nepomuk resources. Under the hood, TelepathyBridge matches the correct
 * Telepathy object to the provided Nepomuk resource(s), and performs the requested action.
 *
 * \note This class provides a higher level interface compared to Telepathy-qt4, since it has no notion of accounts,
 * connections or such. Everything is handled under the hood and retrieved by querying Nepomuk.
 */
class TelepathyBridge : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(TelepathyBridge)
    Q_DISABLE_COPY(TelepathyBridge)
public:
    /**
     * This enum/flag defines how the removal should be handled in \c removeContact() and friends
     *
     * \see removeContact
     * \see removeMetaContact
     */
    enum RemovalMode {
        /** Removes the subscription to the specified contact's presence, and removes it from the buddy list */
        RemoveSubscriptionMode = 1,
        /** Revokes the contact's subscription to your presence */
        RemovePublicationMode = 2,
        /** Blocks the contact */
        BlockMode = 4,
        /** Removes the metacontact only - leaves unaltered the underlying real contacts */
        RemoveMetaContactMode = 8
    };
    Q_DECLARE_FLAGS(RemovalModes, RemovalMode)

    /**
     * This enum defines the error code for a job spawned by one of the functions inside \c TelepathyBridge
     */
    enum JobError {
        /** Taken from KJob. No errors occurred */
        NoError = 0,
        /** Taken from KJob. The job was killed */
        KilledJobError = 1,
        /** The operation supplied is invalid. This, most of the times, represents an error internal to TelepathyBridge */
        InvalidOperationError = 101,
        /**
         * This error means that there has been one or more errors while mapping Nepomuk resources to Telepathy contacts.
         * For specific operations, this means that:
         * <ul>
         * <li>If the operation is being done upon a contact, the Nepomuk resource provided could not be
         *     mapped to an existing Tp::Contact.
         * </li>
         * <li>If the operation is being done upon a metacontact, none of the \c PersonContact belonging to the metacontact
         *     could be mapped to an existing Tp::Contact.
         * </li>
         * </ul>
         */
        NoContactsFoundError = 102,
        /** None of the specified \c RemovalModes were available for the contact(s) in question */
        NoRemovalModesAvailableError = 103,
        /** No valid Telepathy accounts were found to match the specified contacts and/or resources */
        NoAccountsFoundError = 104,
        /** The protocol of the account on which the request is being done is not capable to carry on the requested action */
        ProtocolNotCapableError = 105,
        /** The operation requested the account to be online, but it is not */
        AccountNotOnlineError = 106,
        /** Telepathy triggered an error */
        TelepathyErrorError = 200
    };

    /**
     * \returns The singleton instance for TelepathyBridge
     *
     * \note Always use this method for accessing TelepathyBridge
     */
    static TelepathyBridge *instance();
    /**
     * Standard destructor
     */
    virtual ~TelepathyBridge();

    /**
     * Always call this method before doing anything else. This method will take care of setting up Telepathy
     * and making TelepathyBridge ready to be used.
     *
     * This function is asynchronous: to monitor TelepathyBridge's status, connect to \b ready() signal, which will be
     * emitted upon a successful initialization or an error.
     */
    void init();

    /**
     * \brief Removes a contact from a group
     *
     * This function attempts to remove \p contact from group \p group. It will return a valid KJob ready to be started.
     * Upon error, the KJob will return an error code which belongs to \c JobError.
     *
     * \param group The name of the group
     * \param contact The resource of the contact which should be removed from the group
     *
     * \returns A valid KJob ready to be started
     *
     * \note This function mostly acts as a proxy to Tp::ContactManager::removeContactsFromGroup, although it abstracts
     *       its functionality over multiple accounts.
     *
     * \note Please remember that KJob has to be explicitly started using KJob::start(),
     *       differently from Tp::PendingOperation.
     *
     * \see JobError
     */
    KJob *removeContactFromGroup(const QString &group, const Nepomuk::PersonContact &contact);
    /**
     * \brief Removes a metacontact from a group
     *
     * This function attempts to remove \p contact from group \p group. It will return a valid KJob ready to be started.
     * Upon error, the KJob will return an error code which belongs to \c JobError.
     * It is the equivalent of removeContactFromGroup() for metacontacts.
     *
     * \param group The name of the group
     * \param contact The resource of the metacontact which should be removed from the group
     *
     * \returns A valid KJob ready to be started
     *
     * \note This function acts on every contact associated to the metacontact in question.
     *
     * \see removeContactFromGroup
     */
    KJob *removeMetaContactFromGroup(const QString &group, const Nepomuk::Person &contact);
    /**
     * \brief Removes a list of contacts from a group
     *
     * This function attempts to remove \p contacts from group \p group.
     * It will return a valid KJob ready to be started.
     *
     * \see removeContactFromGroup
     */
    KJob *removeContactsFromGroup(const QString &group, const QList<Nepomuk::PersonContact> &contacts);
    /**
     * \brief Removes a list of metacontacts from a group
     *
     * This function attempts to remove \p contacts from group \p group.
     * It will return a valid KJob ready to be started.
     *
     * \see removeMetaContactFromGroup
     */
    KJob *removeMetaContactsFromGroup(const QString &group, const QList<Nepomuk::Person> &contacts);


    /**
     * \brief Adds a contact to a group
     *
     * This function attempts to add \p contact to group \p group. It will return a valid KJob ready to be started.
     * Upon error, the KJob will return an error code which belongs to \c JobError.
     *
     * \param group The name of the group
     * \param contact The resource of the contact which should be added to the group
     *
     * \returns A valid KJob ready to be started
     *
     * \note This function acts as a proxy to Tp::ContactManager::addContactsToGroup, although it abstracts
     *       its functionality over multiple accounts.
     *
     * \note Please remember that KJob has to be explicitly started using KJob::start(),
     *       differently from Tp::PendingOperation.
     *
     * \see JobError
     */
    KJob *addContactToGroup(const QString &group, const Nepomuk::PersonContact &contact);
    /**
     * \brief Adds a metacontact to a group
     *
     * This function attempts to add \p contact to group \p group. It will return a valid KJob ready to be started.
     * Upon error, the KJob will return an error code which belongs to \c JobError.
     * It is the equivalent of addContactToGroup() for metacontacts.
     *
     * \param group The name of the group
     * \param contact The resource of the metacontact which should be added to the group
     *
     * \returns A valid KJob ready to be started
     *
     * \note This function acts on every contact associated to the metacontact in question.
     *
     * \see addContactToGroup
     */
    KJob *addMetaContactToGroup(const QString &group, const Nepomuk::Person &contact);
    /**
     * \brief Adds a list of contacts to a group
     *
     * This function attempts to add \p contacts to group \p group. It will return a valid KJob ready to be started.
     *
     * \see addContactToGroup
     */
    KJob *addContactsToGroup(const QString &group, const QList<Nepomuk::PersonContact> &contacts);
    /**
     * \brief Adds a list of metacontacts to a group
     *
     * This function attempts to add \p contacts to group \p group. It will return a valid KJob ready to be started.
     *
     * \see addMetaContactToGroup
     */
    KJob *addMetaContactsToGroup(const QString &group, const QList<Nepomuk::Person> &contacts);

    /**
     * \brief Gets all known groups for an account
     *
     * This function returns all known groups for the specified \c account resource.
     *
     * \note the Nepomuk::IMAccount passed to this function must represent a valid local IM Account, and not an
     *       IMAccount associated to a contact.
     *
     * \param accountURI a valid IMAccount representing a valid Telepathy account.
     *
     * \returns A list carrying the names of known groups
     */
    QStringList knownGroupsFor(const Nepomuk::IMAccount &account) const;
    /**
     * \brief Gets all known groups for a contact
     *
     * This function returns all known groups for the specified \c metacontact.
     * This includes all the valid groups \c metacontact can join.
     *
     * \param contact The resource of the metacontact in question
     *
     * \returns A list carrying the names of known groups
     */
    QStringList knownGroupsFor(const Nepomuk::Person &metacontact) const;
    /**
     * \brief Gets all known groups for a metacontact
     *
     * This function returns all known groups for the specified \c contact.
     * This includes all the valid groups \c contact can join.
     *
     * \param contact The resource of the contact in question
     *
     * \returns A list carrying the names of known groups
     */
    QStringList knownGroupsFor(const Nepomuk::PersonContact &contact) const;

    /**
     * \returns The supported \c RemovalModes for the specified \p contact.
     *
     * \note This function will never return \c RemoveMetaContactMode.
     *
     * \see RemovalModes
     */
    RemovalModes supportedRemovalModesFor(const Nepomuk::PersonContact &contact) const;
    /**
     * \returns The supported \c RemovalModes for the specified \p metacontact.
     *
     * \note This function will return all \c RemovalModes supported by <b>at least one</b> of the contacts belonging
     *       to the metacontact in question. So it is not guaranteed that each contact belonging to the metacontact
     *       in question will support all the \c RemovalModes returned by this function.
     *
     * \see RemovalModes
     */
    RemovalModes supportedRemovalModesFor(const Nepomuk::Person &metacontact) const;


    /**
     * \brief Removes a contact from your buddy list
     *
     * This function attempts to remove a contact from your buddy list in the specified \c modes. It will return a valid
     * KJob ready to be started.
     *
     * \param contact The contact to be removed
     * \param modes How the removal should be handled
     *
     * \returns A valid KJob ready to be started
     *
     * \note The protocol providing a contact might not support one or more of the RemovalModes specified. If that's the
     *       case, the mode in question will be simply ignored. \c NoRemovalModesAvailableError will be triggered
     *       only if all of the specified modes are not available for the contact in question.
     *       In any case, it might be a good idea to check \c supportedRemovalModesFor() before calling this function.
     *
     * \note This function does not support \c RemoveMetaContactMode.
     *
     * \note Please remember that KJob has to be explicitly started using KJob::start(),
     *       differently from Tp::PendingOperation.
     *
     * \see RemovalModes
     * \see JobError
     */
    KJob *removeContact(const Nepomuk::PersonContact &contact, RemovalModes modes);
    /**
     * \brief Removes a contact from your buddy list
     *
     * This function attempts to remove a metacontact from your buddy list in the specified \c modes. It will return a valid
     * KJob ready to be started. It is the equivalent of removeContact() for metacontacts.
     *
     * \param metacontact The metacontact to be removed
     * \param modes How the removal should be handled
     *
     * \returns A valid KJob ready to be started
     *
     * \note The removal (unless only \c RemoveMetaContactMode has been specified) will affect every contact belonging to
     *       the metacontact being removed.
     *
     * \see removeContact
     * \see RemovalModes
     * \see JobError
     */
    KJob *removeMetaContact(const Nepomuk::Person &metacontact, RemovalModes modes);

    /**
     * \returns whether the contact identified by \p contactId can be added to \p account.
     *
     * \see addContact
     */
    bool canAddContact(const Nepomuk::IMAccount &account, const QString &contactId);

    /**
     * \brief Adds a contact to an account's buddy list
     *
     * This function attempts to add a contact to your buddy list. It will return a valid KJob ready to be started.
     * To prevent early failures, it might be a good idea to call \c canAddContact() before this function.
     *
     * \param account The account the new contact should be added to
     * \param contactId The ID of the contact to be added
     *
     * \returns A valid KJob ready to be started
     *
     * \note Please remember that KJob has to be explicitly started using KJob::start(),
     *       differently from Tp::PendingOperation.
     */
    KJob *addContact(const Nepomuk::IMAccount &account, const QString &contactId);
    /**
     * \brief Adds a contact to an account's buddy list
     *
     * This function attempts to add a contact to your buddy list. It will return a valid KJob ready to be started.
     * This overloads lets you specify a petname which will be used for the added contact.
     *
     * \param account The account the new contact should be added to
     * \param contactId The ID of the contact to be added
     * \param petName The pet name which will be associated to the contact
     *
     * \returns A valid KJob ready to be started
     *
     * \note Please remember that KJob has to be explicitly started using KJob::start(),
     *       differently from Tp::PendingOperation.
     */
    KJob *addContact(const Nepomuk::IMAccount &account, const QString &contactId, const QString &petName);
    /**
     * \brief Adds a contact to an account's buddy list
     *
     * This function attempts to add a contact to your buddy list. It will return a valid KJob ready to be started.
     * This overloads lets you specify a petname which will be used for the added contact, and a metacontact
     * the new contact will be grouped into.
     *
     * \param account The account the new contact should be added to
     * \param contactId The ID of the contact to be added
     * \param petName The pet name which will be associated to the contact
     * \param metacontact A metacontact the new contact will be grouped into
     *
     * \returns A valid KJob ready to be started
     *
     * \note Please remember that KJob has to be explicitly started using KJob::start(),
     *       differently from Tp::PendingOperation.
     */
    KJob *addContact(const Nepomuk::IMAccount &account, const QString &contactId,
                     const QString &petName, const Nepomuk::Person &metacontact);

    /**
     * \returns whether TelepathyBridge is ready to be used or not. This function will return true after a successful \c init().
     *
     * \see init
     */
    bool isReady() const;

Q_SIGNALS:
    /**
     * This signal is emitted upon a successful or failed \c init()
     *
     * \param success \c true if the initialization was successful, \c false if the initialization failed.
     *
     * \see init
     */
    void ready(bool success);

private:
    TelepathyBridge(QObject* parent = 0);

    TelepathyBridgePrivate * const d_ptr;

    Q_PRIVATE_SLOT(d_func(), void __k__onAccountManagerReady(Tp::PendingOperation*))
    Q_PRIVATE_SLOT(d_func(), void __k__onAccountCreated(const QString &path))

    friend class RemoveContactsFromGroupJobPrivate;
    friend class AddContactsToGroupJobPrivate;
    friend class RemoveContactsJobPrivate;
    friend class AddContactJobPrivate;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(TelepathyBridge::RemovalModes)

#endif // TELEPATHY_BRIDGE_P
