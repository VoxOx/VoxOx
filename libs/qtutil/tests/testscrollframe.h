#ifndef TESTSCROLLFRAME_H
#define TESTSCROLLFRAME_H

#include <QtGui/QWidget>

class QFrame;

class TestScrollFrame : public QWidget {
	Q_OBJECT
public:
	TestScrollFrame();

private Q_SLOTS:
	void addButtons();

private:
	QFrame* _frame;
};

#endif /* TESTSCROLLFRAME_H */
