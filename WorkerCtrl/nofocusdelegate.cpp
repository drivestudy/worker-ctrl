#include "nofocusdelegate.h"

NoFocusDelegate::NoFocusDelegate(QObject *parent) :
    QItemDelegate(parent)
{
}

void NoFocusDelegate::drawFocus(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect) const
{
//    if (option.state & QStyle::State_HasFocus)
//    {
//        QPen penVal(Qt::white);
//        penVal.setWidth(0);
//        painter->setPen(penVal);
//        painter->drawRect(rect);
//    }
}
