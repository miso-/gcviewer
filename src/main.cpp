#include "GCViewerMW.h"

#include <QApplication>

int main(int argc, char **argv)
{
	QApplication app(argc, argv);
	app.setApplicationName("gcviewer");

	GCViewerMW mainWindow;
	mainWindow.show();
	return app.exec();
}
