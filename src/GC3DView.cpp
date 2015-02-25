#include "GC3DView.h"
#include "GCGLView.h"
#include "GCTree/GCPath.h"
#include "GCTree/GCCommand.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QCheckBox>

#include <vector>
#include <cmath>

const QColor objectColor(127, 0, 0);
const QColor layerColor(0, 127, 0);
const QColor pathColor(127, 127, 0);
const QColor commandColor(0, 0, 127);

GC3DView::GC3DView(QWidget *parent)
	: GCAbstractView(parent),
	  m_GCGLView(0),
	  m_vertices(), m_indices(),
	  m_itemRanges(),
	  m_halfFacePoints(0),
	  m_sinTable(), m_cosTable()
{
	QWidget *mainWidget = new QWidget();
	QVBoxLayout *vLayout = new QVBoxLayout();
	mainWidget->setLayout(vLayout);

	QHBoxLayout *hLayout = new QHBoxLayout();
	QCheckBox *hideLayersChkB = new QCheckBox(tr("Hide &upper layers"));
	hLayout->addSpacerItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
	hLayout->addWidget(hideLayersChkB);

	m_GCGLView = new GCGLView(this);

	vLayout->addWidget(m_GCGLView);
	vLayout->addLayout(hLayout);

	connect(hideLayersChkB, SIGNAL(stateChanged(int)), this, SLOT(hideUpperLayers(int)));

	setViewport(mainWidget);

	setLOD(3);
}

void GC3DView::setGridDimensions(const QRectF &dimensions)
{
	m_GCGLView->setGridDimensions(dimensions);
}

const QRectF &GC3DView::gridDimensions() const
{
	return m_GCGLView->gridDimensions();
}

void GC3DView::setLOD(unsigned char LOD)
{
	if (LOD == this->LOD() && m_halfFacePoints != 0) {
		return;
	}

	m_halfFacePoints = LOD + 2;

	// Recalculate goniometric tables.
	double angle = 0;
	double angleStep = M_PI / (m_halfFacePoints - 1);

	m_sinTable = std::vector<double>();
	m_cosTable = std::vector<double>();
	for (GLuint i = 0; i < m_halfFacePoints * 2; i++, angle += angleStep) {
		if (i == m_halfFacePoints) {
			angle -= angleStep;
		}

		m_sinTable.push_back(sin(angle));
		m_cosTable.push_back(cos(angle));
	}

	// Reload data.
	loadGCData();
	m_GCGLView->bufferGCData(m_vertices, m_indices);

	// Reselect selected item.
	if (selectionModel()) {
		currentChanged(selectionModel()->currentIndex(), QModelIndex());
	}
}

unsigned char GC3DView::LOD() const
{
	return static_cast<unsigned char>(m_halfFacePoints - 2);
}

void GC3DView::addThreadHullIndices()
{
	GLuint numFacePoints = m_halfFacePoints * 2;

	if (m_vertices.size() < numFacePoints * 2) {
		return;
	}

	GLuint threadStartVertexIndex = static_cast<GLuint>(m_vertices.size()) - numFacePoints * 2;

	for (GLuint pointNo = 0; pointNo < numFacePoints; pointNo++) {
		m_indices.push_back(threadStartVertexIndex + pointNo);
		m_indices.push_back(threadStartVertexIndex + (pointNo + 1) % numFacePoints);
		m_indices.push_back(threadStartVertexIndex + pointNo + numFacePoints);

		m_indices.push_back(threadStartVertexIndex + (pointNo + 1) % numFacePoints);
		m_indices.push_back(threadStartVertexIndex + (pointNo + 1) % numFacePoints + numFacePoints);
		m_indices.push_back(threadStartVertexIndex + pointNo + numFacePoints);
	}
}

void GC3DView::addThreadFaceIndices(bool start)
{
	GLuint numFacePoints = m_halfFacePoints * 2;

	if (m_vertices.size() < numFacePoints + 1) {
		return;
	}

	GLuint faceStart = static_cast<GLuint>(m_vertices.size()) - (numFacePoints + 1);

	if (start) {

		for (GLuint pointNo = 0; pointNo < numFacePoints; pointNo++) {
			m_indices.push_back(faceStart + numFacePoints); // center.
			m_indices.push_back(faceStart + (pointNo + 1) % numFacePoints);
			m_indices.push_back(faceStart + pointNo);
		}
	} else {

		for (GLuint pointNo = 0; pointNo < numFacePoints; pointNo++) {
			m_indices.push_back(faceStart + pointNo);
			m_indices.push_back(faceStart + (pointNo + 1) % numFacePoints);
			m_indices.push_back(faceStart + numFacePoints); // center.
		}
	}
}

std::vector<GCGLView::Vertex> GC3DView::getThreadVertices(QLineF thread, double width, double height, double z)
{
	std::vector<GCGLView::Vertex> vertices;
	vertices.reserve(m_halfFacePoints * 2);

	if (width < height) {
		width = height;
	}

	double radius = height / 2;
	double centerOffset = width / 2 - radius;

	if (m_halfFacePoints % 2 == 0) {
		double missingHalfWidth = radius - radius * m_sinTable[m_halfFacePoints / 2];
		centerOffset += missingHalfWidth;
	}

	QVector2D normal(thread.dy(), -thread.dx());
	normal.normalize();
	QVector2D centerVect = normal * centerOffset;
	double centerZ = z - radius;

	GCGLView::Vertex vertex;

	for (GLuint pointNo = 0; pointNo < m_halfFacePoints * 2; pointNo++) {

		if (pointNo == m_halfFacePoints) {
			centerVect *= -1;
		}

		vertex.position[0] = static_cast<GLfloat>(m_sinTable[pointNo] * normal.x() * radius + centerVect.x() + thread.p1().x());
		vertex.position[1] = static_cast<GLfloat>(m_sinTable[pointNo] * normal.y() * radius + centerVect.y() + thread.p1().y());
		vertex.position[2] = static_cast<GLfloat>(centerZ + m_cosTable[pointNo] * radius);

		vertex.normal[0] = static_cast<GLfloat>(m_sinTable[pointNo] * normal.x());
		vertex.normal[1] = static_cast<GLfloat>(m_sinTable[pointNo] * normal.y());
		vertex.normal[2] = static_cast<GLfloat>(m_cosTable[pointNo]);

		vertices.push_back(vertex);
	}

	return vertices;
}

void GC3DView::terminatePath(const GCCommand *path)
{
	if (!path || m_vertices.size() < m_halfFacePoints * 2) {
		return;
	}

	QLineF normal = path->thread.unitVector();
	GLfloat normalX = static_cast<GLfloat>(normal.dx());
	GLfloat normalY = static_cast<GLfloat>(normal.dy());

	size_t start = m_vertices.size() - m_halfFacePoints * 2;
	size_t end = m_vertices.size();

	GCGLView::Vertex vertex;

	for (; start < end; start++) {
		vertex = m_vertices[start];
		vertex.normal[0] = normalX;
		vertex.normal[1] = normalY;
		vertex.normal[2] = 0;

		m_vertices.push_back(vertex);
	}

	vertex.position[0] = static_cast<GLfloat>(path->thread.p2().x());
	vertex.position[1] = static_cast<GLfloat>(path->thread.p2().y());
	vertex.position[2] = static_cast<GLfloat>(path->z - (path->threadHeight / 2));

	m_vertices.push_back(vertex);

	addThreadFaceIndices(false);
}

void GC3DView::addThread(const GCCommand *thread, const GCCommand *prevThread)
{
	if (!thread) {
		return;
	}

	std::vector<GCGLView::Vertex> vertices = getThreadVertices(thread->thread, thread->threadWidth,
			thread->threadHeight, thread->z);
	GCGLView::Vertex vertex;

	if (!prevThread) {
		QLineF normal = thread->thread.unitVector();
		GLfloat normalX = static_cast<GLfloat>(-normal.dx());
		GLfloat normalY = static_cast<GLfloat>(-normal.dy());

		for (std::vector<GCGLView::Vertex>::const_iterator i = vertices.begin(); i != vertices.end(); ++i) {
			vertex = *i;
			vertex.normal[0] = normalX;
			vertex.normal[1] = normalY;
			vertex.normal[2] = 0;

			m_vertices.push_back(vertex);
		}

		vertex.position[0] = static_cast<GLfloat>(thread->thread.p1().x());
		vertex.position[1] = static_cast<GLfloat>(thread->thread.p1().y());
		vertex.position[2] = static_cast<GLfloat>(thread->z - thread->threadHeight / 2);
		vertex.normal[0] = normalX;
		vertex.normal[1] = normalY;
		vertex.normal[2] = 0;

		m_vertices.push_back(vertex);

		addThreadFaceIndices(true);
	} else if (m_vertices.size() >= m_halfFacePoints * 2) {
		// Create interconnection segment.

		double  deltaAngle = thread->thread.angleTo(prevThread->thread) * (M_PI / 180.0) / 2.0;

		if (deltaAngle > M_PI / 2.0) {
			deltaAngle -= M_PI;
		}

		// Copy last inserted vertices and adjust normals.
		GCGLView::Vertex vertex;

		size_t start = m_vertices.size() - m_halfFacePoints * 2 - 1;
		size_t end = m_vertices.size();

		GLfloat cosDAngle = static_cast<GLfloat>(cos(deltaAngle));
		GLfloat sinDAngle = static_cast<GLfloat>(sin(deltaAngle));

		for (size_t i = start; i < end; i++) {
			vertex = m_vertices[i];
			GLfloat nX = vertex.normal[0];
			GLfloat nY = vertex.normal[1];

			vertex.normal[0] = nX * cosDAngle - nY * sinDAngle;
			vertex.normal[1] = nX * sinDAngle + nY * cosDAngle;

			m_vertices.push_back(vertex);
		}

		// Insert this segment vertices with adjusted normals.
		cosDAngle = static_cast<GLfloat>(cos(-deltaAngle));
		sinDAngle = static_cast<GLfloat>(sin(-deltaAngle));

		for (std::vector<GCGLView::Vertex>::const_iterator i = vertices.begin(); i != vertices.end(); ++i) {
			vertex = *i;
			GLfloat nX = vertex.normal[0];
			GLfloat nY = vertex.normal[1];

			vertex.normal[0] = nX * cosDAngle - nY * sinDAngle;
			vertex.normal[1] = nX * sinDAngle + nY * cosDAngle;

			m_vertices.push_back(vertex);
		}

		addThreadHullIndices();
	}

	m_vertices.insert(m_vertices.end(), vertices.begin(), vertices.end());
	GLfloat dX = static_cast<GLfloat>(thread->thread.dx());
	GLfloat dY = static_cast<GLfloat>(thread->thread.dy());

	for (std::vector<GCGLView::Vertex>::const_iterator i = vertices.begin(); i != vertices.end(); ++i) {
		vertex = *i;
		vertex.position[0] += dX;
		vertex.position[1] += dY;

		m_vertices.push_back(vertex);
	}

	addThreadHullIndices();
}

bool GC3DView::addItem(const QModelIndex &index, QModelIndex &previous)
{
	if (!model() || !index.isValid()) {
		return false;
	}

	size_t startIndex = m_indices.size();

	const GCCommand *gcCommand = dynamic_cast<const GCCommand *>(static_cast<GCTreeItem *>(index.internalPointer()));
	const GCCommand *prevCommand = dynamic_cast<const GCCommand *>(static_cast<GCTreeItem *>(previous.internalPointer()));

	if (gcCommand) {
		if (!gcCommand->thread.isNull()) {

			if (gcCommand->threadWidth != 0.0) {
				addThread(gcCommand, prevCommand);
				previous = index;
			} else {
				// Travel move, break path.
				terminatePath(prevCommand);
				m_itemRanges[previous].second = m_indices.size();
				previous = QModelIndex();
				return false;
			}

			m_itemRanges.insert(index, QPair<size_t, size_t>(startIndex, m_indices.size()));
			return true;
		}
	} else {
		int numItems = model()->rowCount(index);

		for (int itemNo = 0; itemNo < numItems; ++itemNo) {
			addItem(model()->index(itemNo, 0, index), previous);
		}

		if (previous != QModelIndex()) {
			const GCCommand *prevCommand = dynamic_cast<const GCCommand *>(static_cast<GCTreeItem *>(previous.internalPointer()));
			terminatePath(prevCommand);
			m_itemRanges[previous].second = m_indices.size();
			previous = QModelIndex();
		}

		m_itemRanges.insert(index, QPair<size_t, size_t>(startIndex, m_indices.size()));
		return true;
	}

	return false;
}

QPair<size_t, size_t> GC3DView::getHgltRange(const QModelIndex &index) const
{
	if (index.isValid() && m_itemRanges.contains(index)) {
		return m_itemRanges[index];
	} else {
		return QPair<size_t, size_t>(0, 0);
	}
}

void GC3DView::loadGCData()
{
	if (!model()) {
		return;
	}

	m_vertices = std::vector<GCGLView::Vertex>();
	m_indices = std::vector<GLuint>();

	int numItems = model()->rowCount();

	QModelIndex previous;

	for (int item = 0; item < numItems; ++item) {
		addItem(model()->index(item, 0), previous);
	}

}

void GC3DView::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
	Q_UNUSED(previous)

	QModelIndex cmdIndex = GCModel::getCommandIndex(current);
	QModelIndex pathIndex = GCModel::type(current) == GCTreeItem::GC_PATH ? current : cmdIndex.parent();

	QPair<size_t, size_t> hgltLayerRange = getHgltRange(GCModel::getLayerIndex(current));
	QPair<size_t, size_t> hgltPathRange = getHgltRange(pathIndex);
	QPair<size_t, size_t> hgltCmdRange = getHgltRange(cmdIndex);

	std::vector<QPair<size_t, QColor> > colorRanges;
	colorRanges.push_back(QPair<size_t, QColor>(hgltLayerRange.first, objectColor));
	colorRanges.push_back(QPair<size_t, QColor>(hgltPathRange.first, layerColor));
	colorRanges.push_back(QPair<size_t, QColor>(hgltCmdRange.first, pathColor));
	colorRanges.push_back(QPair<size_t, QColor>(hgltCmdRange.second, commandColor));
	colorRanges.push_back(QPair<size_t, QColor>(hgltPathRange.second, pathColor));
	colorRanges.push_back(QPair<size_t, QColor>(hgltLayerRange.second, layerColor));
	colorRanges.push_back(QPair<size_t, QColor>(m_indices.size(), objectColor));

	m_GCGLView->changeColorRanges(colorRanges);
}

void GC3DView::reset()
{
	QAbstractItemView::reset();
	m_itemRanges = QMap<QModelIndex, QPair<size_t, size_t> >();

	loadGCData();
	m_GCGLView->bufferGCData(m_vertices, m_indices);

	std::vector<QPair<size_t, QColor> > colorRanges;
	colorRanges.push_back(QPair<size_t, QColor>(m_indices.size(), objectColor));
	m_GCGLView->changeColorRanges(colorRanges);
}

void GC3DView::hideUpperLayers(int hide)
{
	m_GCGLView->hideUpperLayers(hide);
}

void GC3DView::resetView()
{
	m_GCGLView->resetView();
}
