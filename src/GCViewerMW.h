#ifndef GCVIEWERMW_H
#define GCVIEWERMW_H

#include <QMainWindow>

class GCModel;
class QItemSelectionModel;
class QModelIndex;
class QTextStream;

namespace Ui
{
class GCViewerMW;
}

class GCViewerMW : public QMainWindow
{
	Q_OBJECT
	Q_DISABLE_COPY(GCViewerMW)

public:
	explicit GCViewerMW(QWidget *parent = 0, Qt::WindowFlags flags = 0);
	virtual ~GCViewerMW();

private slots:
	void on_action_FileOpen_triggered();
	void on_action_FileQuit_triggered();
	void on_action_SettingsFilament_triggered();
	void on_action_Settings3DView_triggered();
	void on_action_HelpAbout_triggered();
	void on_layerSlider_valueChanged(int);
	void currentChanged(const QModelIndex &, const QModelIndex &);
	void layersNumChanged(int);

private:
	Ui::GCViewerMW *ui;

	double m_filamentDiameter;
	double m_packingDensity;

	GCModel *m_gcModel;
	QItemSelectionModel *m_gcSelectionModel;
};

#endif // GCVIEWERMW_H
