#include "GCTreeItem.h"

#include <QtAlgorithms>

GCTreeNodeItem *GCTreeItem::parent() const
{
	return m_parent;
}

int GCTreeItem::childNumber() const
{
	if (parent()) {
		return parent()->indexOf(this);
	}

	return -1;
}

int GCTreeNodeItem::childCount() const
{
	return m_items.size();
}

GCTreeItem *GCTreeNodeItem::child(int n) const
{
	if (n < m_items.size()) {
		return m_items[n];
	}

	return 0;
}

GCTreeNodeItem::~GCTreeNodeItem()
{
	qDeleteAll(m_items);
}

int GCTreeNodeItem::addChild(GCTreeItem *child)
{
	if (child && indexOf(child) < 0) {
		m_items.push_back(child);
		child->m_parent = this;

		return 0;
	}

	return -1;
}

int GCTreeNodeItem::indexOf(const GCTreeItem *child) const
{
	return m_items.indexOf(const_cast<GCTreeItem *>(child));
}
