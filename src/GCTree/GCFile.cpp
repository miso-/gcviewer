#include "GCFile.h"

QVariant GCFile::data(int role, int column) const
{
	Q_UNUSED(role)
	Q_UNUSED(column)

	return QVariant();
}

int GCFile::addChild(GCTreeItem *child)
{
	if (child->type() != GC_FILE) {
		return GCTreeNodeItem::addChild(child);
	}

	return -1;
}
