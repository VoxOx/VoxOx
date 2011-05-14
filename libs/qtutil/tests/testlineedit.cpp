#include <qtutil/ClearLineEdit.h>

#include <QtGui/QApplication>
#include <QtGui/QAbstractButton>
#include <QtGui/QHBoxLayout>
#include <QtGui/QIcon>
#include <QtGui/QWidget>

class Window : public QWidget {
public:
	Window() {
		ClearLineEdit* lineEdit = new ClearLineEdit(this);

		QPixmap pix(16, 16);
		pix.fill(Qt::blue);
		QIcon icon(pix);
		QAbstractButton* button = lineEdit->createLeftButton(icon);
		connect(button, SIGNAL(clicked()), SLOT(close()));

		QHBoxLayout* layout = new QHBoxLayout(this);
		layout->addWidget(lineEdit);
	}
};

int main(int argc, char** argv) {
	QApplication app(argc, argv);
	Q_INIT_RESOURCE(qtutil);
	Window window;

	window.show();
	app.exec();
	return 0;
}
