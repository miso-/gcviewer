#ifndef GCLAYER_H
#define GCLAYER_H

#include "GCTreeItem.h"

#include <QVector>

class GCCommand;

class GCLayer : public GCTreeNodeItem
{
public:
	GCLayer(double z, GCTreeNodeItem *parent = 0);

	int addChild(GCTreeItem *child);
	virtual TYPE type() {return GC_LAYER;}

	virtual QVariant data(int role, int column) const;

private:
	double m_z;
};

#endif // GCLAYER_H
