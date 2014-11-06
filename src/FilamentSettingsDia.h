#ifndef FILAMENTSETTINGSDIA_H
#define FILAMENTSETTINGSDIA_H

#include <QDialog>

class QLineEdit;
class QPushButton;

class FilamentSettingsDia : public QDialog
{
	Q_OBJECT
	Q_DISABLE_COPY(FilamentSettingsDia)

public:
	explicit FilamentSettingsDia(QWidget *parent = 0, Qt::WindowFlags f = 0);
	FilamentSettingsDia(double diameter, double density, QWidget *parent = 0, Qt::WindowFlags f = 0);

	double filamentDiameter() const;
	double packingDensity() const;

public slots:
	void on_okBtn_clicked();
	void on_saveBtn_clicked();
	void on_cancelBtn_clicked();
	void on_qLineEdit_textChanged();

private:
	void init();
	void diameterToUi(double diameter);
	double diameterFromUi() const;
	void densityToUi(double density);
	double densityFromUi() const;
	void updateSaveBtn();
	void readSettings();
	void saveSettings() const;

	mutable double m_savedDiameter;
	mutable double m_savedDensity;

	QLineEdit *m_diameterLE;
	QLineEdit *m_packingLE;
	QPushButton *m_saveBtn;
};

#endif // FILAMENTSETTINGSDIA_H
