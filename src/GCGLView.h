#ifndef GCGLVIEW_H
#define GCGLVIEW_H

#include "GCModel.h"

#define GL_GLEXT_PROTOTYPES
#include <QtOpenGL/QtOpenGL>

#include <QLineF>
#include <QRectF>
#include <QPair>
#include <QMatrix4x4>
#include <QColor>

#include <vector>

class QGLShaderProgram;

class GCGLView : public QGLWidget
{
	Q_OBJECT
	Q_DISABLE_COPY(GCGLView)

public:
	struct Vertex {
		GLfloat position[3];
		GLfloat normal[3];
	};

	explicit GCGLView(QWidget *parent = 0);

	void setGridDimensions(const QRectF &dimensions);
	const QRectF &gridDimensions() const;

	void resetView();
	void hideUpperLayers(int hide);
	void bufferGCData(const std::vector<Vertex> &vertices, const std::vector<GLuint> &indices);
	void changeColorRanges(const std::vector<QPair<size_t, QColor> > &colorRanges);

protected:
	virtual void initializeGL();
	virtual void paintGL();
	virtual void resizeGL(int w, int h);
	virtual void mousePressEvent(QMouseEvent *event);
	virtual void mouseMoveEvent(QMouseEvent *event);
	virtual void wheelEvent(QWheelEvent *event);

private:
	void createPrintBed();

	void paintPrintBed();
	void paintThreads();

	void updateZPlanes();

	void initializeShaders();

	QRectF m_bedGrid;
	QRectF m_bedPlane;

	QPoint m_lastPos;

	qreal m_nearPlane;
	qreal m_farPlane;

	qreal m_cameraZoom;
	qreal m_viewPortAspectR;

	QMatrix4x4 m_projectionMatrix;
	QMatrix4x4 m_viewMatrix;

	bool m_hideUpperLayers;

	QGLShaderProgram *m_shaderProgram;

	GLuint m_printBedVBO;
	GLuint m_threadVerticesVBO;
	GLuint m_threadIndicesVBO;

	size_t m_indicesSize;
	QPair<size_t, size_t> m_thinLinessRange;
	QPair<size_t, size_t> m_thickLinesRange;
	std::vector<QPair<size_t, QColor> > m_colorRanges;
};

#endif // GCGLVIEW_H
