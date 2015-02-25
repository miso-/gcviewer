#ifndef GCLISTVIEW_H
#define GCLISTVIEW_H

#include "GCTree/GCLayer.h"
#include "GCTree/GCCommand.h"

#include <QAbstractItemModel>
#include <QVector>
#include <QLineF>

class QTextStream;
class GCFile;

struct parsedGCData {
	parsedGCData() : z(0.0), commandText(), threadWidth(0.0),
		threadHeight(0.0), thread() {}

	double z;
	QString commandText;			// G-commnad string.
	double threadWidth;
	double threadHeight;
	QLineF thread;					// 2D graphical representation.
};

Q_DECLARE_METATYPE(parsedGCData)

class GCModel : public QAbstractItemModel
{
	Q_OBJECT
	Q_DISABLE_COPY(GCModel)

public:
	GCModel(QObject *parent = 0);

	virtual QVariant data(const QModelIndex &index, int role) const;
	Qt::ItemFlags flags(const QModelIndex &index) const;
	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
	int columnCount(const QModelIndex &parent = QModelIndex()) const;
	QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
	QModelIndex parent(const QModelIndex &index) const;

	bool loadGCode(QTextStream &gcode, double filamentDiameter, double packingDensity);

	// GCModelIndex
	static QModelIndex getLayerIndex(QModelIndex index);
	static QModelIndex getCommandIndex(const QModelIndex &index);
	static GCTreeItem::TYPE type(const QModelIndex &index);

signals:
	void layersNumChanged(int);

private:
	GCTreeItem *getItem(const QModelIndex &index) const;
	void parseGCode(QTextStream &gcodeStream);
	void createThread(const QPointF &begin, const QPointF &end, double e, double zRise, parsedGCData &data) const;

	double m_filamentXsectionArea;

	GCFile *gcFile;
};

#endif // GCLISTVIEW_H
