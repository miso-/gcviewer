#ifndef GCGRAPHICSVIEW_H
#define GCGRAPHICSVIEW_H

#include <QGraphicsView>

class QGraphicsScene;

class GCGraphicsView : public QGraphicsView
{
	Q_OBJECT
	Q_DISABLE_COPY(GCGraphicsView)

public:
	enum GridPosition {Off, Foreground, Background};

	GCGraphicsView(QWidget *parent = 0);

	void setGridDimensions(const QRectF &dimensions);
	const QRectF &gridDimensions() const;

	void setGridPosition(GridPosition gridPosition);

protected:
	virtual void drawForeground(QPainter *painter, const QRectF &rect);
	virtual void drawBackground(QPainter *painter, const QRectF &rect);

private:
	void drawGrid(QPainter *painter, const QRectF &rect);
	virtual void wheelEvent(QWheelEvent *event);

	int m_gridPosition;
	QGraphicsScene *m_scene;
	QRectF m_gridRect;
};

#endif // GCGRAPHICSVIEW_H
