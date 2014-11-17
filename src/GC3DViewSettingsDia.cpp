#include "GC3DViewSettingsDia.h"

#include <QLabel>
#include <QSlider>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSpacerItem>
#include <QSettings>

GC3DViewSettingsDia::GC3DViewSettingsDia(QWidget *parent, Qt::WindowFlags f)
	: QDialog(parent, f),
	  m_savedLOD(0),
	  m_LODSlider(0),
	  m_saveBtn(0)
{
	init();

	m_LODSlider->setValue(m_savedLOD);
}

GC3DViewSettingsDia::GC3DViewSettingsDia(unsigned char LOD, QWidget *parent, Qt::WindowFlags f)
	: QDialog(parent, f),
	  m_savedLOD(0),
	  m_LODSlider(0),
	  m_saveBtn(0)
{
	init();

	m_LODSlider->setValue(LOD);
}

void GC3DViewSettingsDia::init()
{
	setWindowTitle(tr("3D view settings"));

	QHBoxLayout *LODLayout = new QHBoxLayout();
	QLabel *LODLabel = new QLabel(tr("Level of detail:"));
	m_LODSlider = new QSlider(Qt::Horizontal);
	m_LODSlider->setMaximum(15);
	LODLayout->addWidget(LODLabel);
	LODLayout->addWidget(m_LODSlider);

	QHBoxLayout *btnLayout = new QHBoxLayout();
	QPushButton *okBtn = new QPushButton("Ok");
	m_saveBtn = new QPushButton("Save", this);
	m_saveBtn->setEnabled(false);
	m_saveBtn->setDefault(false);
	m_saveBtn->setAutoDefault(false);
	QPushButton *cancelBtn = new QPushButton("Cancel");
	cancelBtn->setDefault(false);
	cancelBtn->setAutoDefault(false);
	btnLayout->addSpacerItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
	btnLayout->addWidget(okBtn);
	btnLayout->addWidget(m_saveBtn);
	btnLayout->addWidget(cancelBtn);

	QVBoxLayout *dialogLayout = new QVBoxLayout(this);
	dialogLayout->addLayout(LODLayout);
	dialogLayout->addSpacerItem(new QSpacerItem(40, 20, QSizePolicy::Minimum, QSizePolicy::Expanding));
	dialogLayout->addLayout(btnLayout);

	setLayout(dialogLayout);

	connect(okBtn, SIGNAL(clicked()), this, SLOT(on_okBtn_clicked()));
	connect(m_saveBtn, SIGNAL(clicked()), this, SLOT(on_saveBtn_clicked()));
	connect(cancelBtn, SIGNAL(clicked()), this, SLOT(on_cancelBtn_clicked()));

	connect(m_LODSlider, SIGNAL(valueChanged(int)), this, SLOT(on_qslider_valueChanged(int)));

	readSettings();
}

unsigned char GC3DViewSettingsDia::LOD() const
{
	return static_cast<unsigned char>(m_LODSlider->value());
}

void GC3DViewSettingsDia::on_okBtn_clicked()
{
	accept();
}

void GC3DViewSettingsDia::on_saveBtn_clicked()
{
	saveSettings();
	m_saveBtn->setEnabled(false);
}

void GC3DViewSettingsDia::on_cancelBtn_clicked()
{
	reject();
}

void GC3DViewSettingsDia::on_qslider_valueChanged(int value)
{
	m_saveBtn->setEnabled(value != m_savedLOD);
}

void GC3DViewSettingsDia::readSettings()
{
	QSettings settings;

	settings.beginGroup("3d_view_settings");
	m_savedLOD = static_cast<unsigned char>(settings.value("LOD", 3).toUInt());
	settings.endGroup();
}

void GC3DViewSettingsDia::saveSettings() const
{
	unsigned char LOD = static_cast<unsigned char>(m_LODSlider->value());

	QSettings settings;
	settings.beginGroup("3d_view_settings");
	settings.setValue("LOD", LOD);
	settings.endGroup();

	m_savedLOD = LOD;
}
