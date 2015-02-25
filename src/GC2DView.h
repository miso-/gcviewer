#ifndef GC2DVIEW_H
#define GC2DVIEW_H

#include "GCAbstractView.h"

class GCModel;
class QGraphicsItem;
class GCGraphicsView;
class QRadioButton;

class GC2DView : public GCAbstractView
{
	Q_OBJECT
	Q_DISABLE_COPY(GC2DView)

public:
	explicit GC2DView(QWidget *parent = 0);

	void setGridDimensions(const QRectF &dimensions);
	const QRectF &gridDimensions() const;

public slots:
	void on_gridRBtn_toggled();
	void currentChanged(const QModelIndex &current, const QModelIndex &previous);
	void reset();
	void selection();

private:
	void addItem(const QModelIndex &index);
	bool removeItem(const QModelIndex &index);
	void highlightItem(const QModelIndex &index, const QColor &color);
	void clear();

	GCGraphicsView *m_gcGraphicsView;

	QRadioButton *m_offRBtn, *m_foregroundRBtn, *m_backgroundRBtn;

	QMap<QModelIndex, QGraphicsItem *> m_indexToItem;
	QMap<QGraphicsItem *, QModelIndex> m_itemToIndex;
};

#endif // GC2DVIEW_H
