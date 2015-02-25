#ifndef GCTREEITEM_H
#define GCTREEITEM_H

#include <QVariant>
#include <QVector>

class GCTreeNodeItem;

class GCTreeItem
{
	Q_DISABLE_COPY(GCTreeItem)

public:
	enum TYPE {INVAL, GC_TREE_ITEM, GC_TREE_NODE_ITEM, GC_COMMAND, GC_PATH, GC_LOOP, GC_LAYER, GC_FILE};

	GCTreeItem(GCTreeNodeItem *parent = 0)
		: m_parent(parent) {}

	virtual ~GCTreeItem() {}

	virtual QVariant data(int role, int column) const = 0;
	virtual int childCount() const {
		return 0;
	}
	virtual GCTreeItem *child(int n) const {
		Q_UNUSED(n);
		return 0;
	};

	GCTreeNodeItem *parent() const;
	int childNumber() const;
	virtual TYPE type() {return GC_TREE_ITEM;}

	friend class GCTreeNodeItem;

protected:
	GCTreeNodeItem *m_parent;
};

Q_DECLARE_METATYPE(const GCTreeItem *)

class GCTreeNodeItem : public GCTreeItem
{
public:
	GCTreeNodeItem(GCTreeNodeItem *parent = 0)
		: GCTreeItem(parent), m_items() {}
	virtual ~GCTreeNodeItem();

	virtual int childCount() const;
	virtual GCTreeItem *child(int n) const;
	virtual int addChild(GCTreeItem *child);
	int indexOf(const GCTreeItem *child) const;
	virtual TYPE type() {return GC_TREE_NODE_ITEM;}

protected:
	QVector<GCTreeItem *> m_items;
};

#endif // GCTREEITEM_H
