#include <qtutil/DesktopService.h>

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QStringList>

#include <QtGui/QApplication>
#include <QtGui/QPixmap>


int main(int argc, char**argv) {
	QApplication app(argc, argv);
	Q_INIT_RESOURCE(qtutil);

	DesktopService* service = DesktopService::getInstance();

	// test desktopIconPixmap
	QPixmap pix = service->desktopIconPixmap(DesktopService::FolderIcon, 96);

	qDebug("Current path: %s", qPrintable(QDir::currentPath()));
	pix.save("Folder.png", "PNG");

	// test pixmapForPath
	QString path;
	if (argc==2) {
		path = argv[1];
	} else {
		path = QDir::homePath();
	}
	QFileInfo fileInfo(path);
	Q_ASSERT(fileInfo.exists());
	
	pix = service->pixmapForPath(path, 96);
	pix.save(fileInfo.fileName() + "_96.png", "PNG");

	pix = service->pixmapForPath(path, 32);
	pix.save(fileInfo.fileName() + "_32.png", "PNG");

	// test startFolderList and userFriendlyNameForPath
	QStringList startFolderList = service->startFolderList();

	qDebug("startFolderList");
	Q_FOREACH(QString folder, startFolderList) {
		QString name = service->userFriendlyNameForPath(folder);
		qDebug("- %s (%s)", 
			qPrintable(name),
			qPrintable(folder));
	}

}
