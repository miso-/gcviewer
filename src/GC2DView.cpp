#include "GC2DView.h"

#include "GCModel.h"
#include "GCThreadItem.h"
#include "GCGraphicsView.h"

#include <QGraphicsLineItem>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSpacerItem>
#include <QGroupBox>
#include <QRadioButton>

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
	QAbstractItemView::currentChanged(current, previous);

	QModelIndex currLayer = GCModel::getLayerIndex(current);
	QModelIndex prevLayer = GCModel::getLayerIndex(previous);

	if (currLayer != prevLayer) {
		// Change layer.
		clear();
		int numItems = model()->rowCount(currLayer);

		for (int item = 0; item < numItems; ++item) {
			addItem(model()->index(item, 0, currLayer));
		}

	} else if (m_indexToItem.contains(previous)) {
		// Deselect previously selected item.
		m_indexToItem[previous]->setSelected(false);
	}

	// Select newly selected item.
	if (m_indexToItem.contains(current)) {
		m_indexToItem[current]->setSelected(true);
	}

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

bool GC2DView::addItem(const QModelIndex &index)
{
	if (!model()) {
		return false;
	}

	QVariant data = model()->data(index, Qt::UserRole);

	if (!data.isValid()) {
		return false;
	}

	if (m_indexToItem.contains(index)) {
		// Already added.
		return false;
	}

	if (!data.value<parsedGCData>().thread.isNull()) {
		QGraphicsLineItem *line = new GCThreadItem(data.value<parsedGCData>()); // QGraphicsLineItem(data.value<parsedGCData>().thread);

		if (!line) {
			return false;
		}

		m_indexToItem.insert(index, line);
		m_itemToIndex.insert(line, index);
		m_gcGraphicsView->scene()->addItem(line);
	}

	return true;
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

void GC2DView::clear()
{
	m_indexToItem.clear();
	m_itemToIndex.clear();
	m_gcGraphicsView->scene()->clear();
}
