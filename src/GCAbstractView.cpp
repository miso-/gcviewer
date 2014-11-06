#include "GCAbstractView.h"
#include "GCModel.h"

#include <QScrollBar>

GCAbstractView::GCAbstractView(QWidget *parent)
	: QAbstractItemView(parent)
{
	setFrameShape(QFrame::NoFrame);

	horizontalScrollBar()->setRange(0, 0);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	verticalScrollBar()->setRange(0, 0);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void GCAbstractView::setModel(GCModel *model)
{
	QAbstractItemView::setModel(model);
}


GCModel *GCAbstractView::model() const
{
	return static_cast<GCModel *>(QAbstractItemView::model());
}

QModelIndex GCAbstractView::indexAt(const QPoint &point) const
{
	Q_UNUSED(point)

	return QModelIndex();
}

void GCAbstractView::scrollTo(const QModelIndex &index, QAbstractItemView::ScrollHint hint)
{
	Q_UNUSED(index)
	Q_UNUSED(hint)
}

QRect GCAbstractView::visualRect(const QModelIndex &index) const
{
	Q_UNUSED(index)

	return QRect();
}

void GCAbstractView::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
	QAbstractItemView::currentChanged(current, previous);
}

QRegion GCAbstractView::visualRegionForSelection(const QItemSelection &selection) const
{
	Q_UNUSED(selection)

	return QRegion();
}

void GCAbstractView::setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command)
{
	Q_UNUSED(rect)
	Q_UNUSED(command)
}

bool GCAbstractView::isIndexHidden(const QModelIndex &index) const
{
	Q_UNUSED(index)

	return true;
}

int GCAbstractView::verticalOffset() const
{
	return 0;
}

int GCAbstractView::horizontalOffset() const
{
	return 0;
}

QModelIndex GCAbstractView::moveCursor(QAbstractItemView::CursorAction cursorAction, Qt::KeyboardModifiers modifiers)
{
	Q_UNUSED(cursorAction)
	Q_UNUSED(modifiers)

	return QModelIndex();
}

bool GCAbstractView::viewportEvent(QEvent *event)
{
	Q_UNUSED(event);

	return false;
}
