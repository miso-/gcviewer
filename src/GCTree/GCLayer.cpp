#include "GCLayer.h"

#include "GCCommand.h"
#include "GCPath.h"
#include "GCLoop.h"

GCLayer::GCLayer(double z, GCTreeNodeItem *parent)
	: GCTreeNodeItem(parent),
	  m_z(z)
{

}

int GCLayer::addChild(GCTreeItem *child)
{
	if (child->type() == GC_LOOP ||
		child->type() == GC_PATH) {
		return GCTreeNodeItem::addChild(child);
	}

	return -1;
}

QVariant GCLayer::data(int role, int column) const
{
	if (column != 0) {
		return QVariant();
	}

	switch (role) {

	case Qt::DisplayRole:
		return QString("Layer: %1").arg(m_z);
		break;

	case Qt::ToolTipRole:
		return QString("Layer: %1").arg(m_z);
		break;

	default:
		return QVariant();
	}
}
