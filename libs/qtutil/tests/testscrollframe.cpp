#include <QtGui/QApplication>
#include <QtGui/QLabel>
#include <QtGui/QToolButton>
#include <QtGui/QVBoxLayout>

#include <qtutil/ScrollFrame.h>

#include "testscrollframe.h"

TestScrollFrame::TestScrollFrame()
: QWidget() {
    ScrollFrame* scrollFrame1 = new ScrollFrame(this);

    QLabel* txt = new QLabel();
    txt->setText("Some text to fill the label so that it can be scrolled");
    scrollFrame1->setChild(txt);

	_frame = new QFrame();
	QHBoxLayout* frameLayout = new QHBoxLayout(_frame);
	frameLayout->setSizeConstraint(QLayout::SetFixedSize);
	for(int x=0; x<15; ++x) {
		QToolButton* button = new QToolButton(_frame);
		button->setText(QString("'%1'").arg(x));
		frameLayout->addWidget(button);

		connect(button, SIGNAL(clicked()), SLOT(addButtons()) );
	}
	ScrollFrame* scrollFrame2 = new ScrollFrame(this);
	scrollFrame2->setChild(_frame);
	scrollFrame2->setScrollStep(100);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(scrollFrame1);
	mainLayout->addWidget(scrollFrame2);
}


void TestScrollFrame::addButtons() {
	QToolButton* button = new QToolButton(_frame);
	button->setText("Added");
	_frame->layout()->addWidget(button);
}


int main(int argc, char** argv) {
	QApplication app(argc, argv);

    TestScrollFrame window;
	window.show();
	return app.exec();
}
