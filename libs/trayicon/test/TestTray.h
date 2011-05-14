#ifndef TESTTRAY_H
#define TESTTRAY_H

#include <QtGui/QWidget>

#include <trayicon.h>

#include <QtGui/QApplication>
#include <QtGui/QMenu>
#include <QtGui/QPainter>
#include <QtGui/QPixmap>
#include <QtCore/QTimer>

class TestWindow : public QWidget {
	Q_OBJECT
public:
	TestWindow()
	: QWidget(0)
	, _offset(0)
	{
		QMenu* menu = new QMenu(this);
		QAction* action = menu->addAction("start anim");
		menu->addAction("quit", qApp, SLOT(quit()));

		connect(action, SIGNAL(triggered()), &_timer, SLOT(start()) );

		QPixmap pix(16, 16);
		pix.fill(Qt::blue);
		_trayIcon = new TrayIcon(pix, "tooltip", menu, this);
		_trayIcon->setToolTip("Bla bla");

		QObject::connect(_trayIcon, SIGNAL(doubleClicked(const QPoint &)), SLOT(show()));
		_trayIcon->show();

		connect(&_timer, SIGNAL(timeout()), SLOT(updateTrayIcon()) );
		_timer.setInterval(300);
		_timer.setSingleShot(false);
	}

protected:
	virtual void closeEvent(QCloseEvent*) {
		hide();
	}

private Q_SLOTS:
	void updateTrayIcon() {
		QPixmap pix(16, 16);
		pix.fill(QColor(0,0,0,0));
		QPainter painter(&pix);
		painter.setRenderHint(QPainter::Antialiasing);
		painter.setPen(Qt::NoPen);
		painter.setBrush(QColor(0,0,0,64));
		painter.drawEllipse(_offset, 8, 8, 8);

		painter.setBrush(Qt::red);
		painter.setPen(Qt::black);
		painter.drawEllipse(4, 2, 8, 8);
		painter.end();

		_trayIcon->setIcon(pix);

		_offset++;
		if (_offset >= 8) {
			_offset = 0;
		}
	}

private:
	QTimer _timer;
	int _offset;
	TrayIcon* _trayIcon;
};

#endif /* TESTTRAY_H */
