#include "GC2DView.h"

#include "GCModel.h"
#include "GCThreadItem.h"
#include "GCGraphicsView.h"
#include "GCTree/GCFile.h"
#include "GCTree/GCLayer.h"
#include "GCTree/GCPath.h"
#include "GCTree/GCCommand.h"

#include <QGraphicsLineItem>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSpacerItem>
#include <QGroupBox>
#include <QRadioButton>

const QColor layerColor(0, 127, 0);
const QColor pathColor(127, 127, 0);
const QColor commandColor(0, 0, 127);

GC2DView::GC2DView(QWidget *parent)
	: GCAbstractView(parent),
	  m_gcGraphicsView(0),
	  m_offRBtn(0), m_foregroundRBtn(0), m_backgroundRBtn(0),
	  m_indexToItem(), m_itemToIndex()
{
	QWidget *mainWidget = new QWidget();
	QVBoxLayout *vLayout = new QVBoxLayout();
	mainWidget->setLayout(vLayout);

	m_gcGraphicsView = new GCGraphicsView(this);
	m_gcGraphicsView->setDragMode(QGraphicsView::ScrollHandDrag);
	m_gcGraphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_gcGraphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	QGroupBox *gridGrpBox = new QGroupBox("Grid");
	QHBoxLayout *gridGrpBoxLayout = new QHBoxLayout();
	m_offRBtn = new QRadioButton("Off");
	m_foregroundRBtn = new QRadioButton("Foreground");
	m_backgroundRBtn = new QRadioButton("BackGground");
	gridGrpBoxLayout->addWidget(m_offRBtn);
	gridGrpBoxLayout->addWidget(m_foregroundRBtn);
	gridGrpBoxLayout->addWidget(m_backgroundRBtn);
	gridGrpBox->setLayout(gridGrpBoxLayout);
	m_foregroundRBtn->setChecked(true);
	QHBoxLayout *hLayout = new QHBoxLayout();
	hLayout->addSpacerItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
	hLayout->addWidget(gridGrpBox);

	vLayout->addWidget(m_gcGraphicsView);
	vLayout->addLayout(hLayout);

	connect(m_gcGraphicsView->scene(), SIGNAL(selectionChanged()), this, SLOT(selection()));

	connect(m_offRBtn, SIGNAL(toggled(bool)), this, SLOT(on_gridRBtn_toggled()));
	connect(m_foregroundRBtn, SIGNAL(toggled(bool)), this, SLOT(on_gridRBtn_toggled()));
	connect(m_backgroundRBtn, SIGNAL(toggled(bool)), this, SLOT(on_gridRBtn_toggled()));

	on_gridRBtn_toggled();

	setViewport(mainWidget);
}

void GC2DView::setGridDimensions(const QRectF &dimensions)
{
	m_gcGraphicsView->setGridDimensions(dimensions);
}

const QRectF &GC2DView::gridDimensions() const
{
	return m_gcGraphicsView->gridDimensions();
}

void GC2DView::on_gridRBtn_toggled()
{
	if (m_offRBtn->isChecked()) {
		m_gcGraphicsView->setGridPosition(GCGraphicsView::Off);
	} else if (m_foregroundRBtn->isChecked()) {
		m_gcGraphicsView->setGridPosition(GCGraphicsView::Foreground);
	} else if (m_backgroundRBtn->isChecked()) {
		m_gcGraphicsView->setGridPosition(GCGraphicsView::Background);
	}
}

void GC2DView::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
	if (!model()) {
		return;
	}

	//WARNING: Must reconnect before exiting this method.
	disconnect(m_gcGraphicsView->scene(), SIGNAL(selectionChanged()), this, SLOT(selection()));

	QModelIndex currLayer = GCModel::getLayerIndex(current);
	QModelIndex prevLayer = GCModel::getLayerIndex(previous);

	QModelIndex prevCmdIndex = GCModel::getCommandIndex(previous);
	QModelIndex prevPathIndex = GCModel::type(previous) == GCTreeItem::GC_PATH ? previous : prevCmdIndex.parent();

	if (currLayer != prevLayer) {
		// Change layer.
		clear();
		int numItems = model()->rowCount(currLayer);

		for (int item = 0; item < numItems; ++item) {
			addItem(model()->index(item, 0, currLayer));
		}

	} else {
		// Deselect previously selected item.
		if (prevPathIndex.isValid()) {
			highlightItem(prevPathIndex, layerColor);
		}

		if (prevCmdIndex.isValid() && m_indexToItem.contains(prevCmdIndex)) {
			m_indexToItem[prevCmdIndex]->setSelected(false);
			highlightItem(prevCmdIndex, layerColor);
		}

	}

	// Select newly selected item.
	QModelIndex currCmdIndex = GCModel::getCommandIndex(current);
	QModelIndex currPathIndex = GCModel::type(current) == GCTreeItem::GC_PATH ? current : currCmdIndex.parent();

	if (currPathIndex.isValid()) {
		highlightItem(currPathIndex, pathColor);
	}

	if (currCmdIndex.isValid()) {
		if (m_indexToItem.contains(current)) {
			m_indexToItem[current]->setSelected(true);
			highlightItem(currCmdIndex, commandColor);
		}
	}

	connect(m_gcGraphicsView->scene(), SIGNAL(selectionChanged()), this, SLOT(selection()));
}

void GC2DView::reset()
{
	QAbstractItemView::reset();

	clear();
}

void GC2DView::selection()
{
	if (!selectionModel()) {
		return;
	}

	QModelIndex currentIndex = selectionModel()->currentIndex();

	if (m_gcGraphicsView->scene()->selectedItems().empty()) {
		// No items are selected.
		selectionModel()->setCurrentIndex(GCModel::getLayerIndex(currentIndex), QItemSelectionModel::ClearAndSelect);
		return;
	}

	if (m_itemToIndex.contains(m_gcGraphicsView->scene()->selectedItems()[0])) {
		currentIndex = m_itemToIndex[m_gcGraphicsView->scene()->selectedItems()[0]];
		selectionModel()->setCurrentIndex(currentIndex, QItemSelectionModel::ClearAndSelect);
	}

}

void GC2DView::addItem(const QModelIndex &index)
{
	const GCCommand *gcCommand = dynamic_cast<const GCCommand *>(static_cast<GCTreeItem *>(index.internalPointer()));

	if (gcCommand) {
		if (m_indexToItem.contains(index)) {
			// Already added.
			return;
		}

		GCThreadItem *line = new GCThreadItem(*gcCommand);

		if (!line) {
			return;
		}

		line->setColor(layerColor);

		m_indexToItem.insert(index, line);
		m_itemToIndex.insert(line, index);
		m_gcGraphicsView->scene()->addItem(line);
	} else {
		int numItems = model()->rowCount(index);

		for (int item = 0; item < numItems; ++item) {
			addItem(model()->index(item, 0, index));
		}
	}
}

bool GC2DView::removeItem(const QModelIndex &index)
{
	if (!m_indexToItem.contains(index)) {
		return false;
	}

	QGraphicsItem *gfxToRemove = m_indexToItem[index];

	m_indexToItem.remove(index);
	m_itemToIndex.remove(gfxToRemove);
	m_gcGraphicsView->scene()->removeItem(gfxToRemove);

	return true;
}

void GC2DView::highlightItem(const QModelIndex &index, const QColor &color)
{
	if (GCModel::type(index) == GCTreeItem::GC_COMMAND && m_indexToItem.contains(index)) {
		static_cast<GCThreadItem *>(m_indexToItem[index])->setColor(color);
	} else {
		int numItems = model()->rowCount(index);

		for (int item = 0; item < numItems; ++item) {
			highlightItem(model()->index(item, 0, index), color);
		}
	}
}

void GC2DView::clear()
{
	m_indexToItem.clear();
	m_itemToIndex.clear();
	m_gcGraphicsView->scene()->clear();
}
