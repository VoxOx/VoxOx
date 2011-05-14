#include <QtCore/QTimer>
#include <QtGui/QApplication>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QVBoxLayout>

#include <qtutil/WidgetUtils.h>

#include "testflashwindow.h"

TestFlashWindow::TestFlashWindow()
: QWidget() {
	_label = new QLabel(this);
	QVBoxLayout* layout = new QVBoxLayout(this);
	
	QPushButton* button = new QPushButton(this);
	button->setText("Flash Me");
	connect(button, SIGNAL(clicked()), SLOT(scheduleFlashMe()) );

	layout->addWidget(button);
	layout->addWidget(_label);

	scheduleFlashMe();
	setMinimumSize(200, 150);
}


void TestFlashWindow::scheduleFlashMe() {
	QTimer::singleShot(4000, this, SLOT(flashMe()));
}


void TestFlashWindow::flashMe() {
	_label->setText("Flash!");
	WidgetUtils::flashWindow(this);
}


int main(int argc, char** argv) {
	QApplication app(argc, argv);

    TestFlashWindow window;
	window.show();
	return app.exec();
}
