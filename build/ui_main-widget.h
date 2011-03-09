#include <kdialog.h>
#include <klocale.h>

/********************************************************************************
** Form generated from reading UI file 'main-widget.ui'
**
** Created: Wed Mar 9 17:43:35 2011
**      by: Qt User Interface Compiler version 4.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAIN_2D_WIDGET_H
#define UI_MAIN_2D_WIDGET_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QToolBar>
#include <QtGui/QTreeView>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWidget
{
public:
    QAction *m_actionAdd_contact;
    QAction *m_actionGroup_contacts;
    QAction *m_actionHide_offline;
    QVBoxLayout *verticalLayout;
    QToolBar *m_toolBar;
    QTreeView *m_contactsListView;
    QHBoxLayout *m_accountButtonsLayout;

    void setupUi(QWidget *MainWidget)
    {
        if (MainWidget->objectName().isEmpty())
            MainWidget->setObjectName(QString::fromUtf8("MainWidget"));
        MainWidget->resize(324, 618);
        m_actionAdd_contact = new QAction(MainWidget);
        m_actionAdd_contact->setObjectName(QString::fromUtf8("m_actionAdd_contact"));
        m_actionGroup_contacts = new QAction(MainWidget);
        m_actionGroup_contacts->setObjectName(QString::fromUtf8("m_actionGroup_contacts"));
        m_actionGroup_contacts->setCheckable(true);
        m_actionGroup_contacts->setChecked(true);
        m_actionHide_offline = new QAction(MainWidget);
        m_actionHide_offline->setObjectName(QString::fromUtf8("m_actionHide_offline"));
        m_actionHide_offline->setCheckable(true);
        m_actionHide_offline->setChecked(true);
        verticalLayout = new QVBoxLayout(MainWidget);
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        m_toolBar = new QToolBar(MainWidget);
        m_toolBar->setObjectName(QString::fromUtf8("m_toolBar"));
        m_toolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

        verticalLayout->addWidget(m_toolBar);

        m_contactsListView = new QTreeView(MainWidget);
        m_contactsListView->setObjectName(QString::fromUtf8("m_contactsListView"));

        verticalLayout->addWidget(m_contactsListView);

        m_accountButtonsLayout = new QHBoxLayout();
        m_accountButtonsLayout->setObjectName(QString::fromUtf8("m_accountButtonsLayout"));

        verticalLayout->addLayout(m_accountButtonsLayout);


        m_toolBar->addAction(m_actionAdd_contact);
        m_toolBar->addAction(m_actionGroup_contacts);
        m_toolBar->addAction(m_actionHide_offline);

        retranslateUi(MainWidget);

        QMetaObject::connectSlotsByName(MainWidget);
    } // setupUi

    void retranslateUi(QWidget *MainWidget)
    {
        MainWidget->setWindowTitle(tr2i18n("Telepathy Contact List Prototype", 0));
        m_actionAdd_contact->setText(tr2i18n("Add contact", 0));
        m_actionGroup_contacts->setText(tr2i18n("Group contacts", 0));
        m_actionHide_offline->setText(tr2i18n("Hide offline", 0));
    } // retranslateUi

};

namespace Ui {
    class MainWidget: public Ui_MainWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // MAIN_2D_WIDGET_H

