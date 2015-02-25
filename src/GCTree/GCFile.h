#ifndef GCFILE_H
#define GCFILE_H

#include <GCTree/GCTreeItem.h>

class GCFile : public GCTreeNodeItem
{
public:
	int addChild(GCTreeItem *child);
	virtual TYPE type() {return GC_FILE;}

	virtual QVariant data(int role, int column) const;
};

#endif // GCFILE_H
