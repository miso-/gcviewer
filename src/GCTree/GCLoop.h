#ifndef GCLOOP_H
#define GCLOOP_H

#include "GCTreeItem.h"

class GCLoop : public GCTreeNodeItem
{
	virtual TYPE type() {return GC_LOOP;}
};

#endif // GCLOOP_H
