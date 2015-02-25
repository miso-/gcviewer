#include "GCGLView.h"

#include <QtOpenGL/QGLShader>
#include <QVector4D>

#include <cmath>
#include <clocale>

// TODO: Error checking, overflows.

GCGLView::GCGLView(QWidget *parent)
	: QGLWidget(QGLFormat(QGL::SampleBuffers | QGL::AlphaChannel), parent),
	  m_bedGrid(), m_bedPlane(),
	  m_lastPos(),
	  m_nearPlane(), m_farPlane(),
	  m_cameraZoom(1),
	  m_viewPortAspectR(static_cast<qreal>(width()) / (height() ? height() : 1)),
	  m_projectionMatrix(), m_viewMatrix(),
	  m_hideUpperLayers(false),
	  m_shaderProgram(0),
	  m_printBedVBO(0), m_threadVerticesVBO(0), m_threadIndicesVBO(0),
	  m_indicesSize(0),
	  m_thinLinessRange(), m_thickLinesRange(), m_colorRanges()
{
	m_shaderProgram = new QGLShaderProgram(context(), this);
}

void GCGLView::setGridDimensions(const QRectF &dimensions)
{

	if (m_bedGrid == dimensions) {
		return;
	}

	m_bedGrid = dimensions;

	// Create 5% extra padding at each side.
	qreal widthPadding = dimensions.width() * 0.1;
	qreal heightPadding = dimensions.height() * 0.1;

	m_bedPlane = dimensions.adjusted(- widthPadding, - heightPadding, widthPadding, heightPadding);

	createPrintBed();
	updateZPlanes();

	updateGL();
}

const QRectF &GCGLView::gridDimensions() const
{
	return m_bedGrid;
}

void GCGLView::resetView()
{
	if (m_viewPortAspectR == 0) {
		return;
	}

	QPointF center = m_bedPlane.center();
	m_viewMatrix.setToIdentity();
	m_viewMatrix.translate(-center.x(), -center.y());

	qreal bedPlaneApectR = m_bedPlane.width() / (m_bedPlane.height() ? m_bedPlane.height() : 1.0);

	if (m_viewPortAspectR > bedPlaneApectR) {
		m_cameraZoom = m_bedPlane.height();
	} else {
		m_cameraZoom = m_bedPlane.width() / m_viewPortAspectR;
	}

}

void GCGLView::hideUpperLayers(int hide)
{
	if (m_hideUpperLayers != static_cast<bool>(hide)) {
		m_hideUpperLayers = hide;
		updateGL();
	}

}

void GCGLView::bufferGCData(const std::vector<Vertex> &vertices, const std::vector<GLuint> &indices)
{
	glBindBuffer(GL_ARRAY_BUFFER, m_threadVerticesVBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_threadIndicesVBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

	m_indicesSize = indices.size();
	
	m_colorRanges.clear();
}

void GCGLView::changeColorRanges(const std::vector<QPair<size_t, QColor> > &colorRanges)
{
	m_colorRanges = colorRanges;

	updateGL();
}

void GCGLView::initializeGL()
{
	glClearColor(0, 0, 0, 0);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	glGenBuffers(1, &m_printBedVBO);
	glGenBuffers(1, &m_threadVerticesVBO);
	glGenBuffers(1, &m_threadIndicesVBO);

	initializeShaders();

	createPrintBed();
	resetView();
}

void GCGLView::paintGL()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	qreal w = m_cameraZoom * m_viewPortAspectR;
	qreal h = m_cameraZoom;

	m_projectionMatrix.setToIdentity();
	m_projectionMatrix.ortho(-w / 2, w / 2, -h / 2, h / 2, -m_nearPlane, -m_farPlane);

	m_shaderProgram->setUniformValue("proj_view_matrix", m_projectionMatrix * m_viewMatrix);
	m_shaderProgram->setUniformValue("normal_matrix", m_viewMatrix.normalMatrix());

	paintPrintBed();
	paintThreads();

	glFlush();
}

void GCGLView::resizeGL(int w, int h)
{

	glViewport(0, 0, static_cast<GLint>(w), static_cast<GLint>(h));
	m_viewPortAspectR = static_cast<qreal>(w) / (h ? h : 1);

	updateGL();
}

void GCGLView::mousePressEvent(QMouseEvent *event)
{
	m_lastPos = event->pos();
}

void GCGLView::mouseMoveEvent(QMouseEvent *event)
{
	int dx = event->x() - m_lastPos.x();
	int dy = event->y() - m_lastPos.y();

	QMatrix4x4 transform;

	if (event->buttons() & Qt::LeftButton) {
		transform.translate(dx * m_cameraZoom / 450, - dy * m_cameraZoom / 450);
	} else if (event->buttons() & Qt::RightButton) {
		transform.rotate(dy / 4.0, 1.0, 0.0, 0.0);
		transform.rotate(dx / 4.0, 0.0, 1.0, 0.0);
	} else if (event->buttons() & Qt::MiddleButton) {
		transform.rotate(dy / 4.0, 1.0, 0.0, 0.0);
		transform.rotate(dx / 4.0, 0.0, 0.0, 1.0);
	}

	m_lastPos = event->pos();
	m_viewMatrix = transform * m_viewMatrix;
	updateZPlanes();

	updateGL();
}

void GCGLView::wheelEvent(QWheelEvent *event)
{
	m_cameraZoom -= static_cast<double>(event->delta()) / 30;

	if (m_cameraZoom < 0.001) {
		m_cameraZoom = 0.001;
	}

	updateGL();
}

void GCGLView::createPrintBed()
{
	const int X = 0;
	const int Y = 1;

	const GLfloat bedPlaneLeft = static_cast<GLfloat>(m_bedPlane.left());
	const GLfloat bedPlaneRight = static_cast<GLfloat>(m_bedPlane.right());
	const GLfloat bedPlaneTop = static_cast<GLfloat>(m_bedPlane.top());
	const GLfloat bedPlaneBottom = static_cast<GLfloat>(m_bedPlane.bottom());

	std::vector<Vertex> vert;

	Vertex planeVert = {{0.0, 0.0, 0.0}, {0.0, 0.0, 1.0}};
	planeVert.position[X] = bedPlaneLeft;
	planeVert.position[Y] = bedPlaneTop;
	vert.push_back(planeVert);
	planeVert.position[X] = bedPlaneRight;
	planeVert.position[Y] = bedPlaneTop;
	vert.push_back(planeVert);
	planeVert.position[X] = bedPlaneRight;
	planeVert.position[Y] = bedPlaneBottom;
	vert.push_back(planeVert);
	planeVert.position[X] = bedPlaneLeft;
	planeVert.position[Y] = bedPlaneBottom;
	vert.push_back(planeVert);


	double gridDensity = 10;

	std::vector<Vertex> thinLines;
	std::vector<Vertex> thickLines;

	std::vector<Vertex> *target;

	Vertex gridVert = {{0.0F, 0.0F, 0.01F}, {0.0F, 0.0F, 1.0F}};
	int lineNo = static_cast<int>(m_bedGrid.left() / gridDensity);
	if (m_bedGrid.left() > 0) {
		++lineNo;
	}

	for (double x = lineNo * gridDensity; x <= m_bedGrid.right(); x += gridDensity, ++lineNo) {
		if (lineNo % 5 == 0) {
			target = &thickLines;
		} else {
			target = &thinLines;
		}

		gridVert.position[X] = static_cast<GLfloat>(x);
		gridVert.position[Y] = static_cast<GLfloat>(m_bedGrid.bottom());
		target->push_back(gridVert);


		gridVert.position[X] = static_cast<GLfloat>(x);
		gridVert.position[Y] = static_cast<GLfloat>(m_bedGrid.top());
		target->push_back(gridVert);
	}

	lineNo = static_cast<int>(m_bedGrid.top() / gridDensity);
	if (m_bedGrid.top() > 0) {
		++lineNo;
	}

	for (double y = lineNo * gridDensity; y <= m_bedGrid.bottom(); y += gridDensity, ++lineNo) {
		if (lineNo % 5 == 0) {
			target = &thickLines;
		} else {
			target = &thinLines;
		}

		gridVert.position[X] = static_cast<GLfloat>(m_bedGrid.left());
		gridVert.position[Y] = static_cast<GLfloat>(y);
		target->push_back(gridVert);

		gridVert.position[X] = static_cast<GLfloat>(m_bedGrid.right());
		gridVert.position[Y] = static_cast<GLfloat>(y);
		target->push_back(gridVert);
	}

	std::vector<Vertex> result;

	result.insert(result.end(), vert.begin(), vert.end());
	m_thinLinessRange = QPair<size_t, size_t>(result.size(), (result.size() + thinLines.size()));
	result.insert(result.end(), thinLines.begin(), thinLines.end());
	m_thickLinesRange = QPair<size_t, size_t>(result.size(), (result.size() + thickLines.size()));
	result.insert(result.end(), thickLines.begin(), thickLines.end());

	glBindBuffer(GL_ARRAY_BUFFER, m_printBedVBO);
	glBufferData(GL_ARRAY_BUFFER, result.size() * sizeof(Vertex), &result[0], GL_STATIC_DRAW);

}

void GCGLView::paintPrintBed()
{
	glBindBuffer(GL_ARRAY_BUFFER, m_printBedVBO);
	m_shaderProgram->enableAttributeArray("position");
	m_shaderProgram->setAttributeBuffer("position", GL_FLOAT, 0, 3, static_cast<int>(sizeof(GLfloat) * 6));
	m_shaderProgram->enableAttributeArray("normal");
	m_shaderProgram->setAttributeBuffer("normal", GL_FLOAT, static_cast<int>(3 * sizeof(GLfloat)), 3, static_cast<int>(sizeof(GLfloat) * 6));

	m_shaderProgram->setUniformValue("global_color", 0.5, 0.5, 0.5, 1.0);
	glDrawArrays(GL_QUADS, 0, 4);

	glLineWidth(1);
	m_shaderProgram->setUniformValue("global_color", 0.0, 0.0, 0.0, 1.0);
	glDrawArrays(GL_LINES, m_thinLinessRange.first, m_thinLinessRange.second - m_thinLinessRange.first);

	glLineWidth(3);
	m_shaderProgram->setUniformValue("global_color", 0.0, 0.0, 0.0, 1.0);
	glDrawArrays(GL_LINES, m_thickLinesRange.first, m_thickLinesRange.second - m_thickLinesRange.first);
}

void GCGLView::paintThreads()
{
	glBindBuffer(GL_ARRAY_BUFFER, m_threadVerticesVBO);
	m_shaderProgram->enableAttributeArray("position");
	m_shaderProgram->setAttributeBuffer("position", GL_FLOAT, 0, 3, static_cast<int>(sizeof(Vertex)));
	m_shaderProgram->enableAttributeArray("normal");
	m_shaderProgram->setAttributeBuffer("normal", GL_FLOAT, static_cast<int>(3 * sizeof(GLfloat)), 3, static_cast<int>(sizeof(Vertex)));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_threadIndicesVBO);

	GLsizei start = 0;
	for (size_t i = 0; i < m_colorRanges.size(); ++i) {
		if (i == m_colorRanges.size() - 1 && m_hideUpperLayers) {
			break;
		}

		GLsizei end = static_cast<GLsizei>(m_colorRanges[i].first);

		if (end <= start) {
			continue;
		}

		GLsizei count = end - start;
		QColor &color = m_colorRanges[i].second;

		m_shaderProgram->setUniformValue("global_color", color);
		glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, reinterpret_cast<GLvoid *>(start * sizeof(GLuint)));

		start = end;
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void GCGLView::updateZPlanes()
{
	qreal height = 100;

	qreal left = m_bedPlane.left();
	qreal right = m_bedPlane.right();
	qreal top = m_bedPlane.top();
	qreal bottom = m_bedPlane.bottom();

	QVector4D zTransform =  m_viewMatrix.row(2);

	qreal near;
	qreal far;
	qreal z;
	near = far = QVector4D::dotProduct(zTransform, QVector4D(left, top, 0, 1));
	z = QVector4D::dotProduct(zTransform, QVector4D(left, top, height, 1));
	near = qMax(near, z);
	far = qMin(far, z);

	z = QVector4D::dotProduct(zTransform, QVector4D(left, bottom, 0, 1));
	near = qMax(near, z);
	far = qMin(far, z);
	z = QVector4D::dotProduct(zTransform, QVector4D(left, bottom, height, 1));
	near = qMax(near, z);
	far = qMin(far, z);

	z = QVector4D::dotProduct(zTransform, QVector4D(right, top, 0, 1));
	near = qMax(near, z);
	far = qMin(far, z);
	z = QVector4D::dotProduct(zTransform, QVector4D(right, top, height, 1));
	near = qMax(near, z);
	far = qMin(far, z);

	z = QVector4D::dotProduct(zTransform, QVector4D(right, bottom, 0, 1));
	near = qMax(near, z);
	far = qMin(far, z);
	z = QVector4D::dotProduct(zTransform, QVector4D(right, bottom, height, 1));
	near = qMax(near, z);
	far = qMin(far, z);

	m_nearPlane = near + 0.1;
	m_farPlane = far - 0.1;
}

void GCGLView::initializeShaders()
{
	setlocale(LC_NUMERIC, "C");

	m_shaderProgram->addShaderFromSourceFile(QGLShader::Vertex, ":/vertex.glsl");
	m_shaderProgram->addShaderFromSourceFile(QGLShader::Fragment, ":/fragment.glsl");
	m_shaderProgram->link();
	m_shaderProgram->bind();

	setlocale(LC_ALL, "");
}
