#include "GCThreadItem.h"

#include <QPen>
#include <QStyleOptionGraphicsItem>

GCThreadItem::GCThreadItem(const GCCommand &data, QGraphicsItem *parent, QGraphicsScene *scene)
	: QGraphicsLineItem(data.thread, parent, scene)
{
	setFlag(QGraphicsItem::ItemIsSelectable, true);
	QPen p = pen();
	p.setWidthF(data.threadWidth);
	p.setCapStyle(Qt::RoundCap);
	setPen(p);
}

void GCThreadItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	// Disable painting of dashed rectangle around selected items.
	QStyleOptionGraphicsItem opt = *option;
	opt.state &= !QStyle::State_Selected;

	QGraphicsLineItem::paint(painter, &opt, widget);
}

QVariant GCThreadItem::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
	if (change == QGraphicsItem::ItemSelectedChange) {
		setZValue(value.toBool() ? 1 : 0);
	}

	return QGraphicsItem::itemChange(change, value);
}

void GCThreadItem::setColor(const QColor &color)
{
	QPen p = pen();
	p.setColor(color);
	setPen(p);

	update();
}
