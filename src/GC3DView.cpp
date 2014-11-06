#include "GC3DView.h"
#include "GCGLView.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QCheckBox>

#include <vector>
#include <cmath>

static const unsigned halfFacePoints = 5;

GC3DView::GC3DView(QWidget *parent)
	: GCAbstractView(parent),
	  m_GCGLView(0),
	  m_vertices(), m_indices(),
	  m_itemRanges()
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
}

void GC3DView::setGridDimensions(const QRectF &dimensions)
{
	m_GCGLView->setGridDimensions(dimensions);
}

const QRectF &GC3DView::gridDimensions() const
{
	return m_GCGLView->gridDimensions();
}

void GC3DView::addThreadHullIndices()
{
	GLuint numFacePoints = halfFacePoints * 2;

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
	GLuint numFacePoints = halfFacePoints * 2;

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
	vertices.reserve(halfFacePoints * 2);

	if (width < height) {
		width = height;
	}

	double radius = height / 2;
	double centerOffset = width / 2 - radius;
	QVector2D normal(thread.dy(), -thread.dx());
	normal.normalize();
	QVector2D centerVect = normal * centerOffset;
	double centerZ = z - radius;

	GCGLView::Vertex vertex;

	double angleStep = M_PI / (halfFacePoints - 1);
	double angle = 0;

	for (unsigned pointNo = 0; pointNo < halfFacePoints; pointNo++) {
		double sinAngle = sin(angle);
		double cosAngle = cos(angle);

		vertex.position[0] = static_cast<GLfloat>(sinAngle * normal.x() * radius + centerVect.x() + thread.p1().x());
		vertex.position[1] = static_cast<GLfloat>(sinAngle * normal.y() * radius + centerVect.y() + thread.p1().y());
		vertex.position[2] = static_cast<GLfloat>(centerZ + cosAngle * radius);

		vertex.normal[0] = static_cast<GLfloat>(sinAngle * normal.x());
		vertex.normal[1] = static_cast<GLfloat>(sinAngle * normal.y());
		vertex.normal[2] = static_cast<GLfloat>(cosAngle);

		vertices.push_back(vertex);
		angle += angleStep;
	}


	angle = M_PI;

	for (unsigned pointNo = 0; pointNo < halfFacePoints; pointNo++) {
		double sinAngle = sin(angle);
		double cosAngle = cos(angle);

		vertex.position[0] = static_cast<GLfloat>(sinAngle * normal.x() * radius - centerVect.x() + thread.p1().x());
		vertex.position[1] = static_cast<GLfloat>(sinAngle * normal.y() * radius - centerVect.y() + thread.p1().y());
		vertex.position[2] = static_cast<GLfloat>(centerZ + cosAngle * radius);

		vertex.normal[0] = static_cast<GLfloat>(sinAngle * normal.x());
		vertex.normal[1] = static_cast<GLfloat>(sinAngle * normal.y());
		vertex.normal[2] = static_cast<GLfloat>(cosAngle);

		vertices.push_back(vertex);
		angle += angleStep;
	}

	return vertices;
}

void GC3DView::terminatePath()
{
	if (m_vertices.size() < halfFacePoints * 2) {
		return;
	}

	size_t normalI = m_vertices.size() - halfFacePoints * 2 + halfFacePoints / 2 - 1;

	GLfloat normalX = -m_vertices[normalI].normal[1];
	GLfloat normalY = m_vertices[normalI].normal[0];

	size_t start = m_vertices.size() - halfFacePoints * 2 - 1;
	size_t end = m_vertices.size();

	GCGLView::Vertex vertex;

	for (; start < end; start++) {
		vertex = m_vertices[start];
		vertex.normal[0] = normalX;
		vertex.normal[1] = normalY;
		vertex.normal[2] = 0;

		m_vertices.push_back(vertex);
	}

	GCGLView::Vertex v1 = m_vertices.back();
	GCGLView::Vertex v2 = m_vertices[m_vertices.size() - halfFacePoints];

	vertex.position[0] = (v1.position[0] + v2.position[0]) / 2;
	vertex.position[1] = (v1.position[1] + v2.position[1]) / 2;
	vertex.position[2] = (v1.position[2] + v2.position[2]) / 2;

	m_vertices.push_back(vertex);
	addThreadFaceIndices(false);
}

void GC3DView::addThread(QLineF thread, double width, double height, double z, bool connect)
{
	std::vector<GCGLView::Vertex> vertices = getThreadVertices(thread, width, height, z);
	GCGLView::Vertex vertex;

	if (!connect) {
		QLineF normal = thread.unitVector();
		GLfloat normalX = static_cast<GLfloat>(-normal.dx());
		GLfloat normalY = static_cast<GLfloat>(-normal.dy());

		for (std::vector<GCGLView::Vertex>::const_iterator i = vertices.begin(); i != vertices.end(); ++i) {
			vertex = *i;
			vertex.normal[0] = normalX;
			vertex.normal[1] = normalY;
			vertex.normal[2] = 0;

			m_vertices.push_back(vertex);
		}

		vertex.position[0] = static_cast<GLfloat>(thread.p1().x());
		vertex.position[1] = static_cast<GLfloat>(thread.p1().y());
		vertex.position[2] = static_cast<GLfloat>(z - height / 2);
		vertex.normal[0] = normalX;
		vertex.normal[1] = normalY;
		vertex.normal[2] = 0;
		m_vertices.push_back(vertex);
		addThreadFaceIndices(true);
	} else if (m_vertices.size() >= halfFacePoints * 2) {
		// Create interconnection segment.

		// Pick normal with large X, Y components.
		size_t normalI = halfFacePoints / 2;
		GLfloat *normalA = m_vertices[m_vertices.size() - halfFacePoints * 2 + normalI - 1].normal;
		GLfloat *normalB = vertices[normalI].normal;

		if ((normalA[1] != 0 || normalA[0] != 0) && (normalB[1] != 0 || normalB[0] != 0)) {

			double angleA = atan2(normalA[1], normalA[0]);
			double angleB = atan2(normalB[1], normalB[0]);
			double deltaAngle = (angleB - angleA) / 2;

			if (deltaAngle > M_PI / 2) {
				deltaAngle = -(M_PI - deltaAngle);
			}

			if (deltaAngle < -M_PI / 2) {
				deltaAngle = (M_PI + deltaAngle);
			}

			// Copy last inserted vertices and adjust normals.
			GCGLView::Vertex vertex;

			size_t start = m_vertices.size() - halfFacePoints * 2 - 1;
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
	}

	m_vertices.insert(m_vertices.end(), vertices.begin(), vertices.end());
	GLfloat dX = static_cast<GLfloat>(thread.dx());
	GLfloat dY = static_cast<GLfloat>(thread.dy());

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
	QModelIndex cmdIndex = GCModel::getCommandIndex(index);

	if (cmdIndex.isValid()) {
		QVariant data = model()->data(cmdIndex, Qt::UserRole);

		if (!data.isValid()) {
			terminatePath();
			m_itemRanges[previous].second = m_indices.size();
			previous = QModelIndex();		// Break path.
			return false;
		}

		if (!data.value<parsedGCData>().thread.isNull()) {

			parsedGCData indexData = data.value<parsedGCData>();

			if (indexData.threadWidth != 0.0) {
				addThread(indexData.thread, indexData.threadWidth, indexData.threadHeight, indexData.z, (previous != QModelIndex()));
				previous = index;
			} else {
				// Travel move, break path.
				terminatePath();
				m_itemRanges[previous].second = m_indices.size();
				previous = QModelIndex();
				return false;
			}

			m_itemRanges.insert(cmdIndex, QPair<size_t, size_t>(startIndex, m_indices.size()));
			return true;
		}
	} else {
		int numItems = model()->rowCount(index);

		for (int itemNo = 0; itemNo < numItems; ++itemNo) {
			addItem(model()->index(itemNo, 0, index), previous);
		}

		if (previous != QModelIndex()) {
			terminatePath();
			m_itemRanges[previous].second = m_indices.size();
			previous = QModelIndex();
		}

		m_itemRanges.insert(index, QPair<size_t, size_t>(startIndex, m_indices.size()));
		return true;
	}

	return false;
}

void GC3DView::loadGCData()
{
	if (!model()) {
		return;
	}

	int numItems = model()->rowCount();

	QModelIndex previous;

	for (int item = 0; item < numItems; ++item) {
		addItem(model()->index(item, 0), previous);
	}

}

void GC3DView::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
	Q_UNUSED(previous)

	QModelIndex layerIndex = GCModel::getLayerIndex(current);
	QModelIndex threadIndex = GCModel::getCommandIndex(current);

	QPair<size_t, size_t> hgltLayerRange;
	if (layerIndex.isValid() && m_itemRanges.contains(layerIndex)) {
		hgltLayerRange = m_itemRanges[layerIndex];
	} else {
		hgltLayerRange = QPair<size_t, size_t>(0, 0);
	}

	QPair<size_t, size_t> hgltThreadRange;
	if (threadIndex.isValid() && m_itemRanges.contains(threadIndex)) {
		hgltThreadRange = m_itemRanges[threadIndex];
	} else {
		hgltThreadRange = QPair<size_t, size_t>(0, 0);
	}

	m_GCGLView->changeHgltRanges(hgltLayerRange, hgltThreadRange);
}

void GC3DView::reset()
{
	QAbstractItemView::reset();
	m_itemRanges = QMap<QModelIndex, QPair<size_t, size_t> >();

	m_vertices = std::vector<GCGLView::Vertex>();
	m_indices = std::vector<GLuint>();

	loadGCData();
	m_GCGLView->bufferGCData(m_vertices, m_indices);
}

void GC3DView::hideUpperLayers(int hide)
{
	m_GCGLView->hideUpperLayers(hide);
}

void GC3DView::resetView()
{
	m_GCGLView->resetView();
}
