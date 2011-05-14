#include <QtGui/QApplication>
#include <QtGui/QVBoxLayout>

#include "testlinklabel.h"


TestWindow::TestWindow()
: QWidget()
{
	_label1 = new LinkLabel(this);
	_label1->setText("OpenWengo");
	_label1->setLink("http://www.openwengo.org");
	connect(_label1, SIGNAL(linkActivated(const QString&)), SLOT(slotClicked(const QString&)) );
	connect(_label1, SIGNAL(linkHovered(const QString&)), SLOT(slotHovered(const QString&)) );

	_label2 = new LinkLabel(this);
	_label2->setText("I need help");
	_label2->setLink("somehelpid");
	connect(_label2, SIGNAL(linkActivated(const QString&)), SLOT(slotClicked(const QString&)) );
	connect(_label2, SIGNAL(linkHovered(const QString&)), SLOT(slotHovered(const QString&)) );

	_caption = new QLabel(this);

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->addWidget(_label1);
	layout->addWidget(_label2);
	layout->addWidget(_caption);
}


void TestWindow::slotClicked(const QString& link) {
	_caption->setText(QString("link clicked: %1").arg(link));
}


void TestWindow::slotHovered(const QString& link) {
	_caption->setText(QString("link hovered: %1").arg(link));
}


int main(int argc, char** argv) {
	QApplication app(argc, argv);
	TestWindow window;
	window.show();
	return app.exec();
}
