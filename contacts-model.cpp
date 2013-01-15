#include "contacts-model.h"

#include <KTp/Models/accounts-tree-proxy-model.h>
#include <KTp/Models/groups-tree-proxy-model.h>

ContactsModel2::ContactsModel2(QObject *parent)
    : ContactsFilterModel(parent),
      m_groupMode(NoGrouping),
      m_source(new KTp::ContactsListModel(this))
{

}

void ContactsModel2::setAccountManager(const Tp::AccountManagerPtr &accountManager)
{
    m_accountManager = accountManager;

    setGroupMode(m_groupMode); //reload the groups, as AccountGrouping now needs a real accountManager
    m_source->setAccountManager(accountManager);
}

void ContactsModel2::setGroupMode(ContactsModel2::GroupMode mode)
{

    m_groupMode = mode;

    if (!m_accountManager) {
        mode = NoGrouping; //don't waste time doing any grouping if account manager is not really ready.. as we will have nothing to group anyway
    }

    if (m_proxy) {
        m_proxy.data()->deleteLater();
    }

    switch (mode) {
    case NoGrouping:
        setSourceModel(m_source);
        break;
    case AccountGrouping:
        m_proxy = new KTp::AccountsTreeProxyModel(m_source, m_accountManager);
        setSourceModel(m_proxy.data());
        break;
    case GroupGrouping:
        m_proxy = new KTp::GroupsTreeProxyModel(m_source);
        setSourceModel(m_proxy.data());
        break;
    }
}

ContactsModel2::GroupMode ContactsModel2::groupMode() const
{
    return m_groupMode;
}
