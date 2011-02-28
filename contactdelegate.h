#ifndef CONTACTDELEGATE_H
#define CONTACTDELEGATE_H

#include <QStyledItemDelegate>

#include "contactdelegateoverlay.h"

class ContactDelegate : public QStyledItemDelegate, public ContactDelegateOverlayContainer
{
    Q_OBJECT
    Q_PROPERTY(int m_fadingValue READ fadingValue WRITE setFadingValue);

public:
    ContactDelegate(QObject * parent = 0);
    ~ContactDelegate();

    void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const;
    QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const;

    int fadingValue() const;
    void setFadingValue(int value);

public Q_SLOTS:
    void hideStatusMessageSlot(const QModelIndex& index);
    void reshowStatusMessageSlot();
    void fadeOutStatusMessageSlot();
    void triggerRepaint();

Q_SIGNALS:
    void repaintItem(QModelIndex);

protected:
    /// Returns the delegate, typically, the derived class
    virtual QAbstractItemDelegate* asDelegate() { return this; }

private:
    QModelIndex m_indexForHiding;
    int         m_fadingValue;
};

#endif // CONTACTDELEGATE_H
