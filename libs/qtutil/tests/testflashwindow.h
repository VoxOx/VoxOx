#ifndef TESTFLASHWINDOW_H
#define TESTFLASHWINDOW_H

#include <QtGui/QWidget>

class QFrame;
class QLabel;

class TestFlashWindow : public QWidget {
	Q_OBJECT
public:
	TestFlashWindow();

private Q_SLOTS:
	void scheduleFlashMe();
	void flashMe();

private:
	QLabel* _label;
};

#endif /* TESTFLASHWINDOW_H */
