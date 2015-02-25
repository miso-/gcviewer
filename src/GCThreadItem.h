#ifndef GCTHREADITEM_H
#define GCTHREADITEM_H

#include "GCTree/GCCommand.h"

#include <QGraphicsLineItem>

class GCThreadItem : public QGraphicsLineItem
{
public:
	GCThreadItem(const GCCommand &data, QGraphicsItem *parent = 0, QGraphicsScene *scene = 0);

	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);
	virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);
	void setColor(const QColor &color);
};

#endif // GCTHREADITEM_H
