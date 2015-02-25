#ifndef GCCOMMAND_H
#define GCCOMMAND_H

#include "GCTreeItem.h"

#include <QString>
#include <QLineF>

class GCCommand : public GCTreeItem
{
public:
	GCCommand(GCTreeNodeItem *parent = 0)
		: GCTreeItem(parent), z(0.0), commandText(), threadWidth(0.0),
		  threadHeight(0.0), thread() {}

	virtual TYPE type() {return GC_COMMAND;}

	virtual QVariant data(int role, int column) const;

	double z;
	QString commandText;			// G-commnad string.
	double threadWidth;
	double threadHeight;
	QLineF thread;					// 2D graphical representation
};

#endif // GCCOMMAND_H
