#ifndef OWQTFLAGSLISTWIDGET_H
#define OWQTFLAGSLISTWIDGET_H

#include <QtCore/QString>
#include <QtGui/QDialog>
#include <QtGui/QComboBox>
#include <QtGui/QListWidget>
#include "QtFlagsManager.h"

class QtFlagsListWidget: public QDialog {
Q_OBJECT
public:
	QtFlagsListWidget(QWidget * parent = 0);
	~QtFlagsListWidget();

	void setPosition(QPoint newPosition);
	void setCodeAreaDefault(std::string codeNumber);
	void setDefaultFlagByCountryName(QString countryName);

	QPixmap getCurrentFlagPixmap();
	QString getCurrentCountryCode();	
	QString getCurrentCountryName();
	QString getCountryNameByRow(int row);
	QString getCountryCodeByRow(int row);
	QString getCountryCodeByItem(QListWidgetItem * qListWidgetItem);
	QString getCountryNameByItem(QListWidgetItem * qListWidgetItem);
	int getWidthNeededInItem( QtFlagsManager * qtFlagsManager, QString flagsNameLanguage);
	QString init(QtFlagsManager * qtFlagsManager, QComboBox * comboBox, QString flagsNameLanguage);
	
	int getRowByCountryName(QString countryName);
	void setDefaultFlag(QListWidgetItem * item );	
	int getPixelSizeText(QString text, QFont font);
	int numberTimesToFillWithChar(QChar fillChar, int sizeNeeded, QString text, QFont font);
	void setSize(int width, int height);
	void setWidthSize(int width);
	void setHeightSize(int height);
	void keyReleaseEvent ( QKeyEvent * event );
	void leaveEvent( QEvent * event );

	virtual void focusOutEvent ( QFocusEvent * );
Q_SIGNALS:
	void currentFlagChanged(QString currentText);
public Q_SLOTS:
	void currentTextChangedSlot(QString currentText);
	void itemClickedSlot(QListWidgetItem * item );
	
private:
	QListWidget * _qListWidget;
};
#endif	//OWQTFLAGSLISTWIDGET_H