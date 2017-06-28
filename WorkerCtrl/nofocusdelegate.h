#ifndef NOFOCUSDELEGATE_H
#define NOFOCUSDELEGATE_H

#include <QItemDelegate>
#include <QtGui>
#include <QtCore>

class NoFocusDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    explicit NoFocusDelegate(QObject *parent = 0);

protected:
    void drawFocus(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect) const;
};

#endif // NOFOCUSDELEGATE_H
