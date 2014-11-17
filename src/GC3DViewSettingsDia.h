#ifndef GC3DVIEWSETTINGSDIA_H
#define GC3DVIEWSETTINGSDIA_H

#include <QDialog>

class QSlider;
class QPushButton;

class GC3DViewSettingsDia : public QDialog
{
	Q_OBJECT
	Q_DISABLE_COPY(GC3DViewSettingsDia)

public:
	explicit GC3DViewSettingsDia(QWidget *parent = 0, Qt::WindowFlags f = 0);
	GC3DViewSettingsDia(unsigned char LOD, QWidget *parent = 0, Qt::WindowFlags f = 0);

	unsigned char LOD() const;

public slots:
	void on_okBtn_clicked();
	void on_saveBtn_clicked();
	void on_cancelBtn_clicked();
	void on_qslider_valueChanged(int);

private:
	void init();
	void readSettings();
	void saveSettings() const;

	mutable unsigned char m_savedLOD;

	QSlider *m_LODSlider;
	QPushButton *m_saveBtn;
};

#endif // GC3DVIEWSETTINGSDIA_H
