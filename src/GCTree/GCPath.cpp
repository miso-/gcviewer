#include "GCPath.h"

GCPath::GCPath(bool isTravel, GCTreeNodeItem *parent)
	: GCTreeNodeItem(parent),
	  m_isTravel(isTravel)
{

}

int GCPath::addChild(GCTreeItem *command)
{
	if (command->type() == GC_COMMAND) {
		return GCTreeNodeItem::addChild(command);
	}

	return -1;
}

QVariant GCPath::data(int role, int column) const
{
	if (column != 0) {
		return QVariant();
	}

	QString type = (m_isTravel ? "Travel " : "Extrusion ");

	switch (role) {

	case Qt::DisplayRole:
		return QString(type + "path:");
		break;

	case Qt::ToolTipRole:
		return QString(type + "path:");
		break;

	default:
		return QVariant();
	}
}
