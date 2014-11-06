#ifndef GCABSTRACTVIEW_H
#define GCABSTRACTVIEW_H

#include <QAbstractItemView>

class GCModel;
class QItemSelectionModel;
class QModelIndex;

class GCAbstractView : public QAbstractItemView
{
	Q_OBJECT

public:
	explicit GCAbstractView(QWidget *parent = 0);

	void setModel(GCModel *model);
	GCModel *model() const;

	virtual void setGridDimensions(const QRectF &dimensions) = 0;
	virtual const QRectF &gridDimensions() const = 0;

	virtual QModelIndex indexAt(const QPoint &point) const;
	virtual void scrollTo(const QModelIndex &index, QAbstractItemView::ScrollHint hint);
	virtual QRect visualRect(const QModelIndex &index) const;

public slots:
	void currentChanged(const QModelIndex &current, const QModelIndex &previous);

protected:
	virtual QRegion visualRegionForSelection(const QItemSelection &selection) const;
	virtual void setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command);
	virtual bool isIndexHidden(const QModelIndex &index) const;
	virtual int verticalOffset() const;
	virtual int horizontalOffset() const;
	virtual QModelIndex moveCursor(QAbstractItemView::CursorAction cursorAction, Qt::KeyboardModifiers modifiers);
	virtual bool viewportEvent(QEvent *event);
};

#endif // GCABSTRACTVIEW_H
