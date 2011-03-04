#include "contactdelegate.h"

#include <QtGui/QPainter>
#include <QApplication>

#include <KIconLoader>
#include <KIcon>
#include <KDebug>

#include "accounts-model.h"

const int SPACING = 4;
const int AVATAR_SIZE = 32;

ContactDelegate::ContactDelegate(QObject * parent)
  : QStyledItemDelegate(parent), ContactDelegateOverlayContainer()
{
}

ContactDelegate::~ContactDelegate()
{
}

void ContactDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    QStyleOptionViewItemV4 optV4 = option;
    initStyleOption(&optV4, index);

    painter->save();

    painter->setClipRect(optV4.rect);

    QStyle *style = QApplication::style();
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter);

//     kDebug() << index.data(AccountsModel::PresenceTypeRole);

    bool isContact = !index.data(AccountsModel::AliasRole).toString().isEmpty();

    if(isContact)
    {

        QRect iconRect = optV4.rect;
        iconRect.setSize(QSize(AVATAR_SIZE, AVATAR_SIZE));
        iconRect.moveTo(QPoint(iconRect.x() + SPACING, iconRect.y() + SPACING));

        QPixmap avatar = QPixmap::fromImage(QImage(index.data(AccountsModel::AvatarRole).toString()));

        if(avatar.isNull()) {
            avatar = SmallIcon("im-user", KIconLoader::SizeMedium);
        }

        painter->drawPixmap(iconRect, avatar);

        QPixmap icon;

        switch(index.data(AccountsModel::PresenceTypeRole).toInt())
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
                icon = SmallIcon("task-attention", KIconLoader::SizeSmallMedium);
                break;
        }

        QRect statusIconRect = optV4.rect;
        statusIconRect.setSize(QSize(22, 22));
        statusIconRect.moveTo(QPoint(optV4.rect.right() - 24, optV4.rect.top() + 8));

        painter->drawPixmap(statusIconRect, icon);

        QRect userNameRect = optV4.rect;
        userNameRect.setX(iconRect.x() + iconRect.width() + SPACING);
        userNameRect.setY(userNameRect.y() + 3);
        userNameRect.setWidth(userNameRect.width() - 28);

        QFont nameFont = painter->font();
        nameFont.setPixelSize(12);
        nameFont.setWeight(QFont::Bold);

        const QFontMetrics nameFontMetrics(nameFont);

        painter->setFont(nameFont);
        painter->drawText(userNameRect, 
                          nameFontMetrics.elidedText(optV4.text, Qt::ElideRight, userNameRect.width()));
        
        QRect statusMsgRect = optV4.rect;
        statusMsgRect.setX(iconRect.x() + iconRect.width() + SPACING);
        statusMsgRect.setY(userNameRect.top() + 16);
        statusMsgRect.setWidth(statusMsgRect.width() - 28);

        QFont statusFont = painter->font();
        statusFont.setWeight(QFont::Normal);
        statusFont.setPixelSize(10);

        const QFontMetrics statusFontMetrics(statusFont);

        if (index == m_indexForHiding) {
            painter->setPen(QColor(0, 0, 0, m_fadingValue)); // TODO: Change to theme colour.
        }

        painter->setFont(statusFont);
        painter->drawText(statusMsgRect,
                          statusFontMetrics.elidedText(index.data(AccountsModel::PresenceMessageRole).toString(),
                                                       Qt::ElideRight, statusMsgRect.width()));

    }
    else
    {
        QRect groupRect = optV4.rect;

        QRect accountGroupRect = groupRect;
        accountGroupRect.setSize(QSize(16,16));
        accountGroupRect.moveTo(QPoint(groupRect.left() + 2, groupRect.top() + 2));

        QRect groupLabelRect = groupRect;
        groupLabelRect.setLeft(20);
        
        QRect expandSignRect = groupLabelRect;
        expandSignRect.setLeft(groupLabelRect.right()-20);

        QFont groupFont = painter->font();
        groupFont.setWeight(QFont::Normal);
        groupFont.setPixelSize(10);

        QString counts;// = QString(" (%1/%2)").arg(index.data(AccountsModel::).toString(),
                        //               index.data(ModelRoles::AccountAllContactsCountRole).toString());


        painter->fillRect(groupRect, QColor(247, 251, 255));

        painter->drawPixmap(accountGroupRect, KIcon(index.data(AccountsModel::IconRole).toString()).pixmap(16,16));

        painter->setFont(groupFont);
        painter->drawText(groupLabelRect, Qt::AlignVCenter, index.data(AccountsModel::DisplayNameRole).toString().append(counts));

        painter->setPen(QColor(220, 220, 220));
        painter->drawLine(groupRect.x(), groupRect.y(), groupRect.width(), groupRect.y());
        painter->drawLine(groupRect.x(), groupRect.bottom(), groupRect.width(), groupRect.bottom());

        painter->setPen(QColor(0, 0, 0));
        
        if(option.state & QStyle::State_Open) {
            painter->drawText(expandSignRect, Qt::AlignVCenter, QString("-"));
        }
        else {
            painter->drawText(expandSignRect, Qt::AlignVCenter, QString("+"));
        }
    }

    painter->restore();
}

QSize ContactDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
//     if(option.state & QStyle::State_Selected)
//         kDebug() << index.data(ModelRoles::UserNameRole).toString();

    if(!index.data(AccountsModel::AliasRole).toString().isEmpty()) {
        return QSize(0, 32 + 4 * SPACING);
    }
    else return QSize(0,20);
}

void ContactDelegate::hideStatusMessageSlot(const QModelIndex& index)
{
    m_indexForHiding = index;
    fadeOutStatusMessageSlot();
}

void ContactDelegate::reshowStatusMessageSlot()
{
    m_fadingValue = 255;
    m_indexForHiding = QModelIndex();
    emit repaintItem(m_indexForHiding);
}

void ContactDelegate::fadeOutStatusMessageSlot()
{
    QPropertyAnimation *a = new QPropertyAnimation(this, "m_fadingValue");
    a->setParent(this);
    a->setDuration(100);
    a->setEasingCurve(QEasingCurve::OutExpo);
    a->setStartValue(255);
    a->setEndValue(0);
    a->start();

    connect(a, SIGNAL(valueChanged(QVariant)),
        this, SLOT(triggerRepaint()));
}

int ContactDelegate::fadingValue() const
{
    return m_fadingValue;
}

void ContactDelegate::setFadingValue(int value)
{
    m_fadingValue = value;
}

void ContactDelegate::triggerRepaint()
{
    emit repaintItem(m_indexForHiding);
}
