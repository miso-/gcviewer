#ifndef GC3DVIEW_H
#define GC3DVIEW_H

#include "GCAbstractView.h"
#include "GCGLView.h"

#include <QMap>
#include <QPair>
#include <vector>

class QVariant;

class GC3DView : public GCAbstractView
{
	Q_OBJECT
	Q_DISABLE_COPY(GC3DView)

public:
	explicit GC3DView(QWidget *parent = 0);

	virtual void setGridDimensions(const QRectF &dimensions);
	virtual const QRectF &gridDimensions() const;
	void setLOD(unsigned char LOD);
	unsigned char LOD() const;

public slots:
	void currentChanged(const QModelIndex &current, const QModelIndex &previous);
	void reset();
	void hideUpperLayers(int hide);
	void resetView();

private:
	void addThreadHullIndices();
	void addThreadFaceIndices(bool start);
	std::vector<GCGLView::Vertex> getThreadVertices(QLineF thread, double width, double height, double z);
	void terminatePath(const QVariant &path);
	void addThread(const QVariant &thread, const QVariant &previous);
	bool addItem(const QModelIndex &index, QModelIndex &previous);
	void loadGCData();

	GCGLView *m_GCGLView;

	std::vector<GCGLView::Vertex> m_vertices;
	std::vector<GLuint> m_indices;

	QMap<QModelIndex, QPair<size_t, size_t> > m_itemRanges;

	GLuint m_halfFacePoints;
	std::vector<double> m_sinTable;
	std::vector<double> m_cosTable;
};

#endif // GC3DVIEW_H
