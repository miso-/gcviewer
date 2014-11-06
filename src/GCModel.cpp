#include "GCModel.h"

#include <QString>
#include <QTextStream>

#include <cmath>

#include <QDebug>

GCModel::GCModel(QObject *parent)
	: QAbstractItemModel(parent),
	  m_filamentXsectionArea(0.0),
	  m_layerStarts(),
	  m_data()
{

}

QVariant GCModel::data(const QModelIndex &index, int role) const
{
	if (index.column() != 0) {
		return QVariant();
	}

	switch (role) {
	case Qt::DisplayRole:
		if (!index.parent().isValid()) {
			// Layer.
			return QString("Layer: %1").arg(index.row());
		} else {
			// Command.
			QPair<int, int> layerRange = getLayerIndexRange(index.parent().row());

			if (layerRange.first + index.row() < layerRange.second) {
				return m_data[layerRange.first + index.row()].commandText;
			}

			return QVariant();
		}

		break;

	case Qt::ToolTipRole:
		if (!index.parent().isValid()) {
			// Layer.
			return QString("Layer: %1").arg(index.row());
		} else {
			// Command.
			QPair<int, int> layerRange = getLayerIndexRange(index.parent().row());

			if (layerRange.first + index.row() < layerRange.second) {
				return QString("Segment length: %1\nSegment width: %2")
					   .arg(QString::number(m_data[layerRange.first + index.row()].thread.length(), 'f', 2))
					   .arg(QString::number(m_data[layerRange.first + index.row()].threadWidth, 'f', 2));
			}

			return QVariant();
		}

		break;

	case Qt::UserRole:
		// Used by graphics views to get raw (parsedGCData) data.
		parsedGCData result;

		if (!index.parent().isValid()) {
			// Layer.
			result.commandText = QString("Layer: %1").arg(index.row());
		} else {
			// Command.
			QPair<int, int> layerRange = getLayerIndexRange(index.parent().row());

			if (layerRange.first + index.row() < layerRange.second) {
				result = m_data[layerRange.first + index.row()];
			}
		}

		return QVariant::fromValue<parsedGCData>(result);
		break;
	}

	return QVariant();
}

Qt::ItemFlags GCModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return 0;

	return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

int GCModel::rowCount(const QModelIndex &parent) const
{
	if (!parent.isValid()) {
		// Layers.
		return m_layerStarts.size();;
	} else if (parent.internalId() == quint32(-1)) {
		// Command.
		QPair<int, int> layerRange = getLayerIndexRange(parent.row());
		return layerRange.second - layerRange.first;
	}

	return 0;
}

int GCModel::columnCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent)

	return 1;
}

QModelIndex GCModel::index(int row, int column, const QModelIndex &parent) const
{
	if (column != 0) {
		return QModelIndex();
	}

	if (!parent.isValid()) {
		// Layer item.
		return createIndex(row, column, quint32(-1));
	} else {
		// Command item.
		return createIndex(row, column, quint32(parent.row()));
	}
}

QModelIndex GCModel::parent(const QModelIndex &index) const
{
	if (!index.isValid()) {
		return QModelIndex();
	}

	if (index.internalId() == quint32(-1)) {
		// Layer.
		return QModelIndex();
	} else {
		// Command, parent layer index is stored in its internalId().
		return createIndex(int(index.internalId()), 0, quint32(-1));
	}

}

bool GCModel::loadGCode(QTextStream &gcode, double filamentDiameter, double packingDensity)
{
	beginResetModel();

	m_data.clear();
	m_layerStarts.clear();

	m_filamentXsectionArea =  std::fabs((M_PI * filamentDiameter * filamentDiameter / 4) * packingDensity);

	parseGCode(gcode);

	endResetModel();
	emit layersNumChanged(rowCount());

	return true;
}

QModelIndex GCModel::getLayerIndex(const QModelIndex &index)
{
	if (!index.isValid()) {
		return QModelIndex();
	}

	if (index.parent().isValid()) {
		return index.parent();
	}

	return index;
}

QModelIndex GCModel::getCommandIndex(const QModelIndex &index)
{
	if (!index.isValid()) {
		return QModelIndex();
	}

	if (!index.parent().isValid()) {
		// If parent is invalid, then "index" is layer index.
		return QModelIndex();
	}

	return index;
}

bool getGCParam(const QString &line, const QString &param, double &value)
{
	int paramStartIndex = line.indexOf(param);

	if (paramStartIndex == -1) {
		return false;
	}

	int valueStartIndex = paramStartIndex + param.length();
	int valueEndIndex = line.indexOf(" ", valueStartIndex) - 1;

	if (valueEndIndex < 0) {
		valueEndIndex = line.length() - 1;
	}

	value = line.mid(valueStartIndex, valueEndIndex - valueStartIndex + 1).toDouble();

	return true;
}

bool getGCParam(const QString &line, const QString &param, int &value)
{
	int paramStartIndex = line.indexOf(param);

	if (paramStartIndex == -1) {
		return false;
	}

	int valueStartIndex = paramStartIndex + param.length();
	int valueEndIndex = line.indexOf(" ", valueStartIndex) - 1;

	if (valueEndIndex < 0) {
		valueEndIndex = line.length() - 1;
	}

	value = line.mid(valueStartIndex, valueEndIndex - valueStartIndex + 1).toInt();

	return true;
}

void GCModel::parseGCode(QTextStream &gcodeStream)
{
	// Parses only G1.

	QString line;
	QPointF currPos;
	QPointF newPos;
	double currZ = 0.0;
	double newZ = 0.0;
	double zRise = 0.0;
	double e;
	double param;

	m_layerStarts.push_back(0);

	while (!gcodeStream.atEnd()) {
		line = gcodeStream.readLine();

		parsedGCData data;
		data.z = currZ;
		data.commandText = line;

		line = line.simplified().toUpper();

		int gNum;
		if (line.startsWith("G") && getGCParam(line, "G", gNum)) {

			if (gNum == 1) {
				if (getGCParam(line, "X", param)) {
					newPos.setX(param);
				}

				if (getGCParam(line, "Y", param)) {
					newPos.setY(param);
				}

				if (getGCParam(line, "Z", param)) {
					newZ = param;
				}

				if (!getGCParam(line, "E", e)) {
					e = 0.0;
				}


				if (currZ != newZ) {
					// Inter layers travel move.
					zRise = newZ - currZ;
					currZ = newZ;
					m_layerStarts.push_back(m_data.size() + 1);

				} else {
					createThread(currPos, newPos, e, zRise, data);
				}

				currPos = newPos;
			}

			m_data.push_back(data);
		}
	}

}

void GCModel::createThread(const QPointF &begin, const QPointF &end, double e, double zRise, parsedGCData &data) const
{
	data.thread =  QLineF(begin, end);

	if (e == 0.0 || data.thread.isNull()) {
		data.threadWidth = 0;
		data.threadHeight = 0;
	} else {
		double threadXsectioArea = m_filamentXsectionArea * e / data.thread.length();

		// http://hydraraptor.blogspot.com/2011/03/spot-on-flow-rate.html
		data.threadWidth = (threadXsectioArea / zRise) - (M_PI * zRise / 4) + zRise;
		data.threadHeight = zRise;

		if (data.threadWidth < data.threadHeight) {
			// "Bridge" - circular x-section.
			data.threadWidth = std::sqrt(threadXsectioArea / M_PI);
			data.threadHeight = data.threadWidth;
		}
	}

	return;
}

QPair<int, int> GCModel::getLayerIndexRange(int layerIndex) const
{
	if (layerIndex < 0 || layerIndex >= m_layerStarts.size()) {
		return QPair<int, int>(0, 0);
	}

	int start = m_layerStarts[layerIndex];
	int end;

	if (layerIndex + 1 >= m_layerStarts.size()) {
		end = m_data.size();
	} else {
		end = m_layerStarts[layerIndex + 1];
	}

	if (start > end) {
		return QPair<int, int>(0, 0);
	}

	return QPair<int, int>(start, end);
}
