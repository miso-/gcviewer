#include "FilamentSettingsDia.h"

#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QSpacerItem>
#include <QDoubleValidator>
#include <QString>
#include <QSettings>

#include <limits>
#include <cmath>

static const unsigned Decimals = 2;

FilamentSettingsDia::FilamentSettingsDia(QWidget *parent, Qt::WindowFlags f)
	: QDialog(parent, f),
	  m_savedDiameter(0.0),
	  m_savedDensity(0.0),
	  m_diameterLE(0),
	  m_packingLE(0),
	  m_saveBtn(0)
{
	init();

	diameterToUi(m_savedDiameter);
	densityToUi(m_savedDensity);
}

FilamentSettingsDia::FilamentSettingsDia(double diameter, double density, QWidget *parent, Qt::WindowFlags f)
	: QDialog(parent, f),
	  m_savedDiameter(0.0),
	  m_savedDensity(0.0),
	  m_diameterLE(0),
	  m_packingLE(0),
	  m_saveBtn(0)

{
	init();

	diameterToUi(diameter);
	densityToUi(density);
}

void FilamentSettingsDia::init()
{
	setWindowTitle("Filament settings");

	QGridLayout *formLayout = new QGridLayout();
	QLabel *diameterL = new QLabel("Filament diameter:");
	QLabel *packingL = new QLabel("Filament packing density:");
	m_diameterLE = new QLineEdit(this);
	m_diameterLE->setValidator(new QDoubleValidator(0, std::numeric_limits<double>::infinity() , Decimals, this));
	m_packingLE =  new QLineEdit(this);
	m_packingLE->setValidator(new QDoubleValidator(0, std::numeric_limits<double>::infinity(), Decimals, this));
	formLayout->addWidget(diameterL, 0, 0);
	formLayout->addWidget(m_diameterLE, 0, 1);
	formLayout->addWidget(packingL, 1, 0);
	formLayout->addWidget(m_packingLE, 1, 1);

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
	dialogLayout->addLayout(formLayout);
	dialogLayout->addSpacerItem(new QSpacerItem(40, 20, QSizePolicy::Minimum, QSizePolicy::Expanding));
	dialogLayout->addLayout(btnLayout);

	setLayout(dialogLayout);

	connect(okBtn, SIGNAL(clicked()), this, SLOT(on_okBtn_clicked()));
	connect(m_saveBtn, SIGNAL(clicked()), this, SLOT(on_saveBtn_clicked()));
	connect(cancelBtn, SIGNAL(clicked()), this, SLOT(on_cancelBtn_clicked()));

	connect(m_diameterLE, SIGNAL(textChanged(QString)), this, SLOT(on_qLineEdit_textChanged()));
	connect(m_packingLE, SIGNAL(textChanged(QString)), this, SLOT(on_qLineEdit_textChanged()));

	readSettings();
}

void FilamentSettingsDia::diameterToUi(double diameter)
{
	m_diameterLE->setText(QString::number(std::fabs(diameter), 'f', Decimals));
	updateSaveBtn();
}

double FilamentSettingsDia::diameterFromUi() const
{
	return m_diameterLE->text().toDouble();
}

void FilamentSettingsDia::densityToUi(double density)
{
	m_packingLE->setText(QString::number(std::fabs(density), 'f', Decimals));
	updateSaveBtn();
}

double FilamentSettingsDia::densityFromUi() const
{
	return m_packingLE->text().toDouble();
}

void FilamentSettingsDia::updateSaveBtn()
{
	m_saveBtn->setEnabled(diameterFromUi() != m_savedDiameter || densityFromUi() != m_savedDensity);
}

double FilamentSettingsDia::filamentDiameter() const
{
	return diameterFromUi();
}

double FilamentSettingsDia::packingDensity() const
{
	return densityFromUi();
}

void FilamentSettingsDia::on_okBtn_clicked()
{
	accept();
}

void FilamentSettingsDia::on_saveBtn_clicked()
{
	saveSettings();
	m_saveBtn->setEnabled(false);
}

void FilamentSettingsDia::on_cancelBtn_clicked()
{
	reject();
}

void FilamentSettingsDia::on_qLineEdit_textChanged()
{
	updateSaveBtn();
}

void FilamentSettingsDia::readSettings()
{
	QSettings settings;

	settings.beginGroup("filament_settings");
	m_savedDiameter = settings.value("filament_diameter").toDouble();
	m_savedDensity = settings.value("packing_density").toDouble();
	settings.endGroup();
}

void FilamentSettingsDia::saveSettings() const
{
	double diameter = diameterFromUi();
	double density = densityFromUi();

	QSettings settings;
	settings.beginGroup("filament_settings");
	settings.setValue("filament_diameter", diameter);
	settings.setValue("packing_density", density);
	settings.endGroup();

	m_savedDiameter = diameter;
	m_savedDensity = density;
}
