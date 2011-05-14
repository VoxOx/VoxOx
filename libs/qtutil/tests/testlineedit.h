#ifndef TESTLINEEDIT_H
#define TESTLINEEDIT_H

#include <QtGui/QAbstractButton>
#include <QtGui/QApplication>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLineEdit>
#include <QtGui/QStyleOptionButton>
#include <QtGui/QStylePainter>


class LineEditButton : public QAbstractButton {
public:
	LineEditButton(QWidget* parent)
	: QAbstractButton(parent) {
	}

	QSize sizeHint() const {
		return iconSize();
	}

protected:
	void paintEvent(QPaintEvent* event) {
		QStylePainter painter(this);
		QStyleOptionButton option;
		option.initFrom(this);
		option.icon = icon();
		option.iconSize = iconSize();
		painter.drawControl(QStyle::CE_PushButtonLabel, option);
	}
};


class IconLineEdit : public QLineEdit {
public:
	IconLineEdit(QWidget* parent = 0)
	: QLineEdit(parent) {
		_leftButton = 0;
		_rightButton = 0;
		updateCss();
	}

	QAbstractButton* createLeftButton(const QIcon& icon) {
		_leftButton = createButton(icon);
		moveButtons();
		updateCss();
		return _leftButton;
	}

	QAbstractButton* createRightButton(const QIcon& icon) {
		_rightButton = createButton(icon);
		moveButtons();
		updateCss();
		return _rightButton;
	}

	QAbstractButton* createButton(const QIcon& icon) {
		LineEditButton* button = new LineEditButton(this);
		button->setIcon(icon);
		button->setCursor(Qt::ArrowCursor);
		button->setFocusPolicy(Qt::NoFocus);
		return button;
	}

	void updateCss() {
		int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
		int leftPadding = 0;
		if (_leftButton) {
			leftPadding = _leftButton->sizeHint().width() + frameWidth;
		}

		int rightPadding = 0;
		if (_rightButton) {
			rightPadding = _rightButton->sizeHint().width() + frameWidth;
		}

		QString css = QString("QLineEdit { padding-left: %1px; padding-right: %2px }")
			.arg(leftPadding).arg(rightPadding);
		setStyleSheet(css);
	}

	void moveButtons() {
		int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
		if (_leftButton) {
			QSize buttonSize = _leftButton->sizeHint();
			int posY = (height() - buttonSize.height()) / 2;
			_leftButton->move(frameWidth * 2, posY);
		}

		if (_rightButton) {
			QSize buttonSize = _rightButton->sizeHint();
			int posX = rect().right() - buttonSize.width() - (frameWidth * 2);
			int posY = (height() - buttonSize.height()) / 2;
			_rightButton->move(posX, posY);
		}
	}

	void resizeEvent(QResizeEvent* event) {
		QLineEdit::resizeEvent(event);
		moveButtons();
	}

protected:
	QAbstractButton* _leftButton;
	QAbstractButton* _rightButton;
};


class ClearLineEdit : public IconLineEdit {
	Q_OBJECT
public:
	ClearLineEdit(QWidget* parent)
	: IconLineEdit(parent) {
		QIcon clearIcon("clear.png");
		createRightButton(clearIcon);
		connect(_rightButton, SIGNAL(clicked()), SLOT(clear()) );
		connect(this, SIGNAL(textChanged(const QString&)), SLOT(updateRightButton()) );
		updateRightButton();
	}

private Q_SLOTS:
	void updateRightButton() {
		_rightButton->setVisible(!text().isEmpty());
	}
};


class Window : public QWidget {
public:
	Window() {
		ClearLineEdit* lineEdit = new ClearLineEdit(this);

		QAbstractButton* button = lineEdit->createLeftButton(QIcon("viewmag.png"));
		connect(button, SIGNAL(clicked()), SLOT(close()));

		QHBoxLayout* layout = new QHBoxLayout(this);
		layout->addWidget(lineEdit);
	}
};

#endif /* TESTLINEEDIT_H */
