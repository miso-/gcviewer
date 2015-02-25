#include "GCModel.h"

#include "GCTree/GCFile.h"
#include "GCTree/GCLayer.h"
#include "GCTree/GCPath.h"
#include "GCTree/GCCommand.h"

#include <QString>
#include <QTextStream>

#include <cmath>

GCModel::GCModel(QObject *parent)
	: QAbstractItemModel(parent),
	  m_filamentXsectionArea(0.0),
	  gcFile(0)
{

}

QVariant GCModel::data(const QModelIndex &index, int role) const
{
	GCTreeItem *item = getItem(index);

	if (item) {
		return item->data(role, index.column());
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
	GCTreeItem *item = getItem(parent);

	if (!item) {
		return 0;
	}

	return item->childCount();
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

	GCTreeItem *parentItem = getItem(parent);
	if (!parentItem) {
		return QModelIndex();
	}

	GCTreeItem *childItem = parentItem->child(row);
	if (childItem) {
		return createIndex(row, column, childItem);
	}

	return QModelIndex();
}

QModelIndex GCModel::parent(const QModelIndex &index) const
{
	if (!index.isValid()) {
		return QModelIndex();
	}

	GCTreeItem *item = getItem(index);
	if (!item) {
		return QModelIndex();
	}

	GCTreeItem *parentItem = item->parent();
	if (parentItem) {
		return createIndex(parentItem->childNumber(), 0, parentItem);
	}

	return QModelIndex();

}

bool GCModel::loadGCode(QTextStream &gcode, double filamentDiameter, double packingDensity)
{
	beginResetModel();

	if (gcFile) {
		delete gcFile;
		gcFile = 0;
	}
	gcFile = new GCFile();

	m_filamentXsectionArea =  std::fabs((M_PI * filamentDiameter * filamentDiameter / 4) * packingDensity);

	parseGCode(gcode);

	endResetModel();
	emit layersNumChanged(rowCount());

	return true;
}

QModelIndex GCModel::getLayerIndex(QModelIndex index)
{
	while (index.isValid()) {
		if (type(index) == GCTreeItem::GC_LAYER) {
			return index;
		}
		index = index.parent();
	}

	return QModelIndex();
}

QModelIndex GCModel::getCommandIndex(const QModelIndex &index)
{
	if (type(index) == GCTreeItem::GC_COMMAND) {
		return index;
	} else {
		return QModelIndex();
	}
}

GCTreeItem::TYPE GCModel::type(const QModelIndex &index)
{
	if (!index.isValid()) {
		return GCTreeItem::INVAL;
	}

	GCTreeItem *item = static_cast<GCTreeItem *>(index.internalPointer());
	if (item) {
		return item->type();
	}

	return GCTreeItem::INVAL;
}

GCTreeItem *GCModel::getItem(const QModelIndex &index) const
{
	if (!index.isValid()) {
		return gcFile;
	}

	return static_cast<GCTreeItem *>(index.internalPointer());
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

	GCLayer *layer = new GCLayer(currZ);
	GCPath *path = new GCPath(true);
	bool pathTravel = true;

	while (!gcodeStream.atEnd()) {
		line = gcodeStream.readLine();

		parsedGCData data;
		data.z = currZ;
		data.commandText = line;

		GCCommand *gcCommand = new GCCommand();
		gcCommand->z = currZ;
		gcCommand->commandText = line;

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

					layer->addChild(path);
					path = new GCPath(true);
					pathTravel = true;

					gcFile->addChild(layer);
					layer = new GCLayer(newZ);

				} else {
					createThread(currPos, newPos, e, zRise, data);
					gcCommand->thread = data.thread;
					gcCommand->threadHeight = data.threadHeight;
					gcCommand->threadWidth = data.threadWidth;
				}

				currPos = newPos;
			}

			if ((pathTravel && data.threadWidth > 0.001) || (!pathTravel && data.threadWidth < 0.001)) {
				layer->addChild(path);
				pathTravel = !pathTravel;
				path = new GCPath(pathTravel);

			}
			path->addChild(gcCommand);
		}
	}
	layer->addChild(path);
	gcFile->addChild(layer);

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
