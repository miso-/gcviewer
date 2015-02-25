#include "GCCommand.h"

QVariant GCCommand::data(int role, int column) const
{
	if (column != 0) {
		return QVariant();
	}

	switch (role) {

	case Qt::DisplayRole:
		return commandText;
		break;

	case Qt::ToolTipRole:
		return QString("Segment length: %1\nSegment width: %2")
			   .arg(QString::number(thread.length(), 'f', 2))
			   .arg(QString::number(threadWidth, 'f', 2));
		break;

	default:
		return QVariant();
	}
}
