#ifndef GCPATH_H
#define GCPATH_H

#include "GCTreeItem.h"
#include "GCCommand.h"

class GCPath : public GCTreeNodeItem
{
public:
	GCPath(bool isTravel, GCTreeNodeItem *parent = 0);

	int addChild(GCTreeItem *command);
	virtual TYPE type() {return GC_PATH;}

	virtual QVariant data(int role, int column) const;

private:
	bool m_isTravel;
};

#endif // GCPATH_H
