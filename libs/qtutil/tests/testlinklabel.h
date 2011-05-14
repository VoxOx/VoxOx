#ifndef TESTLINKLABEL_H
#define TESTLINKLABEL_H

#include <QtGui/QWidget>
#include <qtutil/LinkLabel.h>

class TestWindow : public QWidget {
	Q_OBJECT

public:
	TestWindow();

private Q_SLOTS:
	void slotClicked(const QString& link);
	void slotHovered(const QString& link);

private:
	LinkLabel* _label1;
	LinkLabel* _label2;
	QLabel* _caption;
};

#endif /* TESTLINKLABEL_H */
