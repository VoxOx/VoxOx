
#ifndef IMAGEBUTTON_H
#define IMAGEBUTTON_H

#include <QtGui/QPushButton>
#include <QtCore/QTimer>
class QMouseEvent;

class ImageButton : public QPushButton
{
	Q_OBJECT

public:
	ImageButton(QWidget* parent = 0);
	~ImageButton();
	const QString& getNormalPixmap() const;
	const QString& getPressedPixmap() const;

Q_SIGNALS:

	void clickAndHold();
	void rightClick();
	void hoverButtonEvent();
	void leftMouseButtonPress();//VOXOX CHANGE by Rolando - 2009.07.08 

private Q_SLOTS:
	
	void checkHold();

public Q_SLOTS:
	
	void setNormalPixmap(const QString& pixmap);
	void setPressedPixmap(const QString& pixmap);
	void setImages(const QString& normalPixmap="",const QString& pressedPixmap = "", const QString & hoverPixmap = "", const QString & disablePixmap = "", const QString & checkedPixmap = "");
	


private:

	bool eventFilter(QObject * obj, QEvent * event);

	void mousePressEvent(QMouseEvent * event); 
	void mouseReleaseEvent ( QMouseEvent * event );
	QString _normalPixmap;
	QString _pressedPixmap;
	QString _hoverPixmap;
	QString _disablePixmap;
	QString _checkedPixmap;
	void updateStyle();
	//VOXOX - CJC - 2009.06.10 
	QTimer * _holdTimer;
};

#endif // LINKLABEL_H
