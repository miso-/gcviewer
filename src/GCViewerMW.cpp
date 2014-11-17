#include "GCViewerMW.h"

#include "ui_GCViewerMW.h"
#include "FilamentSettingsDia.h"
#include "GC3DViewSettingsDia.h"
#include "GCModel.h"
#include "GC2DView.h"

#ifdef BUILD_3D
#include "GC3DView.h"
#endif // BUILD_3D

#include <QItemSelectionModel>
#include <QModelIndex>
#include <QFileDialog>
#include <QMessageBox>
#include <QLabel>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QSettings>

GCViewerMW::GCViewerMW(QWidget *parent, Qt::WindowFlags flags)
	: QMainWindow(parent, flags),
	  ui(0),
	  m_filamentDiameter(0.0),
	  m_packingDensity(0.0),
	  m_gcModel(0),
	  m_gcSelectionModel(0)
{
	ui = new Ui::GCViewerMW();
	ui->setupUi(this);

	m_gcModel = new GCModel(this);
	m_gcSelectionModel = new QItemSelectionModel(m_gcModel, this);

	ui->gcTreeView->setModel(m_gcModel);
	ui->gcTreeView->setSelectionModel(m_gcSelectionModel);

	ui->gc2DView->setGridDimensions(QRectF(0, 0, 200, 200));
	ui->gc2DView->setModel(m_gcModel);
	ui->gc2DView->setSelectionModel(m_gcSelectionModel);

#ifdef BUILD_3D
	GC3DView *gc3DView = new GC3DView();
	GC3DViewSettingsDia gc3DViewSettings;
	gc3DView->setLOD(gc3DViewSettings.LOD());
	gc3DView->setGridDimensions(QRectF(0, 0, 200, 200));
	gc3DView->setModel(m_gcModel);
	gc3DView->setSelectionModel(m_gcSelectionModel);

	ui->tabWidget->addTab(gc3DView, "3D");

	ui->action_Settings3DView->setEnabled(true);
#else
	QLabel *label = new QLabel(tr("GCViewer was built without OpenGL support, 3D view is disabled"));
	label->setAlignment(Qt::AlignCenter);
	ui->tabWidget->addTab(label, "3D");
#endif // BUILD_3D

	connect(m_gcSelectionModel, SIGNAL(currentChanged(QModelIndex, QModelIndex)), this, SLOT(currentChanged(QModelIndex, QModelIndex)));
	connect(m_gcModel, SIGNAL(layersNumChanged(int)), this, SLOT(layersNumChanged(int)));

	FilamentSettingsDia filamentSettings;
	m_filamentDiameter = filamentSettings.filamentDiameter();
	m_packingDensity = filamentSettings.packingDensity();
}

GCViewerMW::~GCViewerMW()
{
	delete ui;
}

void GCViewerMW::on_action_FileOpen_triggered()
{
	QSettings settings;

	QString gcFilename = QFileDialog::getOpenFileName(this, tr("Open File"), settings.value("last_file").toString(), tr("Supported files(*.gcode);;All files(*.*)"));

	if (!gcFilename.isEmpty()) {
		QFile gcFile(gcFilename);

		if (!gcFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
			QMessageBox::critical(this, tr("Error"), tr("Unable to open G-code file."));
			return;
		}

		QTextStream g(&gcFile);
		m_gcModel->loadGCode(g, m_filamentDiameter, m_packingDensity);

		QDir dir;
		settings.setValue("last_file", dir.absoluteFilePath(gcFilename));
	}
}

void GCViewerMW::on_action_FileQuit_triggered()
{
	// Close this G-code viewer window.

	close();
}

void GCViewerMW::on_action_SettingsFilament_triggered()
{
	FilamentSettingsDia filamentSettings(m_filamentDiameter, m_packingDensity);
	if (filamentSettings.exec()) {
		m_filamentDiameter = filamentSettings.filamentDiameter();
		m_packingDensity = filamentSettings.packingDensity();
		if (m_gcModel->rowCount()) {
			// File is already loaded.
			QMessageBox message(QMessageBox::Information, tr("File loaded"),
								tr("File is already loaded. New settings will take effect after reloading the file"));
			message.exec();
		}
	}
}

void GCViewerMW::on_action_Settings3DView_triggered()
{
#ifdef BUILD_3D
	GC3DView *gc3DView = qobject_cast<GC3DView *>(ui->tabWidget->widget(1));
	if (!gc3DView) {
		return;
	}

	GC3DViewSettingsDia gc3DViewSettings(gc3DView->LOD());
	if (gc3DViewSettings.exec()) {
		gc3DView->setLOD(gc3DViewSettings.LOD());
	}
#endif // BUILD_3D
}

void GCViewerMW::on_action_HelpAbout_triggered()
{
	QMessageBox aboutBox(this);
	aboutBox.setWindowTitle(tr("About"));
	aboutBox.setTextFormat(Qt::RichText);
	aboutBox.setText("<h1><center>G-code viewer</center></h1>\
<center><small>Copyright (C) 2014  miso-</small></center>\
<br />\
<br />\
Simple tool for viewing gcode files."
					);
	aboutBox.exec();
}

void GCViewerMW::on_layerSlider_valueChanged(int value)
{
	if (GCModel::getLayerIndex(m_gcSelectionModel->currentIndex()) !=  m_gcModel->index(value, 0)) {
		m_gcSelectionModel->setCurrentIndex(m_gcModel->index(value, 0), QItemSelectionModel::ClearAndSelect);
		ui->gcTreeView->collapseAll();
		ui->gcTreeView->expand(m_gcSelectionModel->currentIndex());
	}

}

void GCViewerMW::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
	Q_UNUSED(previous)

	ui->layerSlider->setValue(GCModel::getLayerIndex(current).row());
}

void GCViewerMW::layersNumChanged(int value)
{
	if (value > 0) {
		ui->layerSlider->setRange(0, m_gcModel->rowCount() - 1);
		ui->layerSlider->setEnabled(true);
	} else {
		ui->layerSlider->setEnabled(false);
	}
}
