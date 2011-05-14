
#include "stdafx.h"		//VOXOX - JRT - 2009.04.05
#include "QtFlagsListWidget.h"

#include <QtGui/QtGui>
#include <qtutil/SafeConnect.h>
#include <util/SafeDelete.h>//VOXOX CHANGE by Rolando - 2009.06.12 
#include <util/Logger.h>

#include <qtutil/Widget.h>//VOXOX CHANGE by Rolando - 2009.06.12 

#include <model/config/ConfigManager.h>
#include <model/config/Config.h>

const int HEIGHT_SIZE = 200;
const int WIDTH_SIZE = 276;
const QChar FILL_CHAR = QChar(' ');
const QString FLAGS_NAME_LANGUAGE = QString("english");

//added by Rolando 04-11-08
QtFlagsListWidget::QtFlagsListWidget( QWidget * parent): QDialog(parent){//VOXOX CHANGE by Rolando - 2009.06.12 
		
	_qListWidget = new QListWidget();//VOXOX CHANGE by Rolando - 2009.06.12 
	_qListWidget->setSizePolicy ( QSizePolicy::Expanding, QSizePolicy::Expanding);//VOXOX CHANGE by Rolando - 2009.06.11 

	if (!layout()) {//VOXOX CHANGE by Rolando - 2009.06.12 
		Widget::createLayout(this);//VOXOX CHANGE by Rolando - 2009.06.12 
	}
	layout()->addWidget(_qListWidget);//VOXOX CHANGE by Rolando - 2009.06.12 
	setContentsMargins(0,0,0,0);//VOXOX CHANGE by Rolando - 2009.06.12 

	_qListWidget->setWindowOpacity(0.95);//VOXOX CHANGE by Rolando - 2009.06.12 
	_qListWidget->setVerticalScrollMode ( QAbstractItemView::ScrollPerItem );
	setFixedHeight ( HEIGHT_SIZE );
	setFixedWidth(WIDTH_SIZE);
	setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);	
	setAttribute( Qt::WA_GroupLeader, TRUE );
	setWindowTitle(QString("VoxOx - Countries Codes"));
	
	//SAFE_CONNECT(_qListWidget, SIGNAL(currentTextChanged(QString)), SLOT(currentTextChangedSlot(QString)));//VOXOX CHANGE Rolando 03-24-09
	SAFE_CONNECT(_qListWidget, SIGNAL(itemClicked(QListWidgetItem *)), SLOT(itemClickedSlot(QListWidgetItem *)));//VOXOX CHANGE by Rolando - 2009.06.12 

}

QtFlagsListWidget::~QtFlagsListWidget(){
	OWSAFE_DELETE(_qListWidget);//VOXOX CHANGE by Rolando - 2009.06.12 
}

//added by Rolando 04-11-08
void QtFlagsListWidget::setSize(int width, int height){
	setFixedHeight (height);
	setFixedWidth(width);
}

//added by Rolando 04-11-08
void QtFlagsListWidget::setWidthSize(int width){
	setFixedWidth(width + width/8);//we add width/8 because to reserve space at the end of item
}

//added by Rolando 04-11-08
void QtFlagsListWidget::setHeightSize(int height){
	setFixedHeight (height);
}

//added by Rolando 04-11-08
void QtFlagsListWidget::setPosition(QPoint newPositionPoint){

	move(newPositionPoint);
}

/* //added by Rolando 04-11-08
 * getCurrentCountryCode: gets the area code using regular expressions to search on text in listWidgetItem currently selected
 * parameters: none
 * returns QString
 */
QString QtFlagsListWidget::getCurrentCountryCode(){

	QListWidgetItem * qListWidgetItem = new QListWidgetItem();
	qListWidgetItem = _qListWidget->currentItem();//VOXOX CHANGE by Rolando - 2009.06.12 
	QString countryCodeText = qListWidgetItem->text();
	countryCodeText = countryCodeText.trimmed();
	if(countryCodeText.indexOf(QRegExp(".+\\+[0-9]+$")) != -1){
		int pos = countryCodeText.lastIndexOf(QRegExp("\\+[0-9]+"));
		QString codeNumber = countryCodeText.mid(pos+1, countryCodeText.size());
		return codeNumber;		
	}
	else{
		return QString::null;
		
	}	
}

/* //added by Rolando 04-11-08
 * getCurrentCountryName: gets the country name using regular expressions to search on text in listWidgetItem currently selected
 * parameters: none
 * returns QString
 */
QString QtFlagsListWidget::getCurrentCountryName(){

	QListWidgetItem * qListWidgetItem = new QListWidgetItem();
	qListWidgetItem = _qListWidget->currentItem();//VOXOX CHANGE by Rolando - 2009.06.12 
	QString itemText = qListWidgetItem->text();	
	QRegExp rx("(.+ ?)+. ");	//checks country name, because format is "countryname +areaCode"
	int pos = rx.indexIn(itemText);//gets the country name
	if( pos != -1){		
		QString countryName = rx.cap(0);//gets countryName captured
		countryName =  countryName.trimmed();
		return countryName;		
	}
	else{
		return QString::null;
		
	}	
	
}


/* //added by Rolando 04-11-08
 * getCountryCodeByItem: gets the country code using regular expressions to search on text in qListWidgetItem 
 * parameters: QListWidgetItem * qListWidgetItem
 * returns QString
 *
*/
QString QtFlagsListWidget::getCountryCodeByItem(QListWidgetItem * qListWidgetItem){

	QString countryCodeText = qListWidgetItem->text();
	countryCodeText = countryCodeText.trimmed();
	if(countryCodeText.indexOf(QRegExp(".+\\+[0-9]+$")) != -1){//checks areaCode format is valid, because format in item is "countryname +areaCode"
		int pos = countryCodeText.lastIndexOf(QRegExp("\\+[0-9]+"));//gets position where area code is in text
		QString codeNumber = countryCodeText.mid(pos+1, countryCodeText.size());//gets area code
		return codeNumber;		
	}
	else{
		return QString::null;
		
	}	
}

/* //added by Rolando 04-11-08
 * getCountryNameByItem: gets the country name using regular expressions to search in text in qListWidgetItem 
 * parameters: QListWidgetItem * qListWidgetItem
 * returns QString
 */
QString QtFlagsListWidget::getCountryNameByItem(QListWidgetItem * qListWidgetItem){

	QString itemText = qListWidgetItem->text();	
	QRegExp rx("(.+ ?)+. ");	//checks country name
	int pos = rx.indexIn(itemText);//gets the country name
	if( pos != -1){		
		QString countryName = rx.cap(0);
		countryName =  countryName.trimmed();
		return countryName;		
	}
	else{
		return QString::null;
		
	}	
	
}

 
/* //added by Rolando 04-11-08
 * getCountryNameByRow: gets the country name according index in listWidget, using regular expressions to search in text in qListWidgetItem 
 * parameters: int row
 * returns QString
 */
QString QtFlagsListWidget::getCountryNameByRow(int row){

	QListWidgetItem * qListWidgetItem = new QListWidgetItem();
	qListWidgetItem = _qListWidget->item(row);//VOXOX CHANGE by Rolando - 2009.06.12 gets the item in QtListWidget according parameter row received
	QString itemText = qListWidgetItem->text();	
	QRegExp rx("(.+ ?)+. ");	//checks country name
	int pos = rx.indexIn(itemText);//gets the country name
	if( pos != -1){		
		QString countryName = rx.cap(0);
		countryName =  countryName.trimmed();
		return countryName;		
	}
	else{
		return QString::null;
		
	}	
	
}


/* //added by Rolando 04-11-08
 * getCountryCodeByRow: gets the country code according index in listWidget, using regular expressions to search in text in qListWidgetItem 
 * parameters: int row
 * returns QString
 */
QString QtFlagsListWidget::getCountryCodeByRow(int row){

	QListWidgetItem * qListWidgetItem = new QListWidgetItem();
	qListWidgetItem = _qListWidget->item(row);//gets the item in QtListWidget according parameter row received
	QString countryCodeText = qListWidgetItem->text();
	countryCodeText = countryCodeText.trimmed();
	if(countryCodeText.indexOf(QRegExp(".+\\+[0-9]+$")) != -1){//checks that area code is contained in text
		int pos = countryCodeText.lastIndexOf(QRegExp("\\+[0-9]+"));//gets the position where area code is in text
		QString codeNumber = countryCodeText.mid(pos+1, countryCodeText.size());//gets area code
		return codeNumber;
	}
	else{
		return QString::null;
		
	}	

	
		
}

/* //added by Rolando 04-11-08
 * getRowByCountryName: gets the index in QtListWidget where parameter countryName is in QtListWidget
 * parameters: int row
 * returns QString
 */
int QtFlagsListWidget::getRowByCountryName(QString countryName){
	
	QRegExp rxCountryName(countryName);	//checks country name
	QRegExp rxCountryNameInItem("(.+ ?)+. ");	//regular expression to get the country name in text
	QString textCaptured;
	int pos = 0;
	for(int i = 0;  i < _qListWidget->count(); i++){	
		QString textItem = _qListWidget->item(i)->text();
		if( rxCountryNameInItem.indexIn(textItem) != -1){// if countryName is contained in text
			textCaptured = rxCountryNameInItem.cap();//gets the text captured by regular expression
			textCaptured = textCaptured.trimmed();			
			if( rxCountryName.exactMatch(textCaptured)){//if countryName is exactly the same name in item
				return i;	
			}	
		}			
	}

	return -1;
	
}

/* //added by Rolando 04-11-08 
 * setDefaultFlag: sets the current item selected according  parameter item
 * parameters: QListWidgetItem * item
 * returns void
*/
void QtFlagsListWidget::setDefaultFlag(QListWidgetItem * item ){
	_qListWidget->setCurrentItem ( item );
	int tmpCurrentRow = _qListWidget->currentRow ();
	_qListWidget->setCurrentRow(tmpCurrentRow);//VOXOX CHANGE by Rolando - 2009.06.12 
	_qListWidget->scrollToItem (item, QAbstractItemView::PositionAtCenter );//VOXOX CHANGE by Rolando - 2009.06.12 
	
}

/* //added by Rolando 04-11-08
 * setDefaultFlagByCountryName: sets the parameter countryName as current item in QtListWidget
 * parameters: QString countryName
 * returns void
*/
void QtFlagsListWidget::setDefaultFlagByCountryName(QString countryName){
	if(countryName == QString("")){
		countryName = QString("None");
	}
	int row = getRowByCountryName(countryName);	

	if(row == -1){
		row = 0;
	}

	_qListWidget->setCurrentItem ( _qListWidget->item(row) );//VOXOX CHANGE by Rolando - 2009.06.12 
	int tmpCurrentRow = _qListWidget->currentRow ();
	_qListWidget->setCurrentRow(tmpCurrentRow);//VOXOX CHANGE by Rolando - 2009.06.12 
	_qListWidget->scrollToItem (_qListWidget->item(row), QAbstractItemView::PositionAtCenter);//VOXOX CHANGE by Rolando - 2009.06.12 
}


/* //added by Rolando 04-11-08
 * getPixelSizeText: gets the size in pixels from text according font parameter
 * parameters: QString text, QFont font
 * returns int
*/
int QtFlagsListWidget::getPixelSizeText(QString text, QFont font){
	return QFontMetrics(font).width(text);

}

/* //added by Rolando 04-11-08
 * numberTimesToFillWithChar: gets the number times to fill with parameter fillChar a text with a size needed
 * parameters: QChar fillChar, int sizeNeeded, QString text, QFont font
 * returns int
*/
int QtFlagsListWidget::numberTimesToFillWithChar(QChar fillChar, int sizeNeeded, QString text, QFont font){
	int charWidth = QFontMetrics(font).width(fillChar);//gets the fillChar's width in pixels according parameter font
	int textWidth = QFontMetrics(font).width(text);//gets the text's width in pixels according parameter font
	int difference = sizeNeeded - textWidth; //gets the difference in pixels between size needed to fill and text's width
	if (difference < 0){//if is not necesary to fill with fillChar returns -1
		return -1;
	}
	else{//else
		return (difference/charWidth);//gets the number times to fill with fillChar
	}
	
}

/* //added by Rolando 04-11-08
 * getWidthNeededInItem: gets the width size needed in QtListWidget according the largest text in QtFlagsManager
 * parameters:  QtFlagsManager * qtFlagsManager, QString flagsNameLanguage
 * returns int, the width size needed in QtListWidget
*/
int QtFlagsListWidget::getWidthNeededInItem( QtFlagsManager * qtFlagsManager, QString flagsNameLanguage){
	
	QFont font = _qListWidget->font();	//VOXOX CHANGE by Rolando - 2009.06.12 

	//gets the largest country name text and sets their name to the variable
	QString largestCountryNameText = qtFlagsManager->getLargestCountryNameText();		
	
	QtFlag flag = qtFlagsManager->getFlagByCountryName(largestCountryNameText,flagsNameLanguage);

	//gets the largest country name size in pixels
	int largestCountryNamePixelSize = qtFlagsManager->getPixelSizeText(largestCountryNameText, font);

	int pixmapWidthSize = flag.getPixmap().width();
	int blankSpacePixelSize = qtFlagsManager->getPixelSizeText(QString(" "), font);
	int largestCountryCodePixelSize = 0;
	
	//gets the flaglist according flagsNameLanguage
	QtFlagsManager::QtFlagList flagList = qtFlagsManager->getQtFlagList(flagsNameLanguage);
	QtFlagsManager::QtFlagList::iterator it;

	QString areaCode;
	int tmpCountryCodePixelSize = 0;

	for (it = flagList.begin(); it != flagList.end(); it++) {
		areaCode = it->getDefaultCountryCode();		
		tmpCountryCodePixelSize =  qtFlagsManager->getPixelSizeText(areaCode, font);
		
		if(tmpCountryCodePixelSize > largestCountryCodePixelSize){
			largestCountryCodePixelSize	= tmpCountryCodePixelSize;
		}		
	}	
	
	//because the format in item of QtFlagsListWidget is pixmap + blank space + countryName + blank space + areaCode + blank space
	int totalPixelsSizeCountryName = pixmapWidthSize + blankSpacePixelSize + largestCountryNamePixelSize + blankSpacePixelSize + largestCountryCodePixelSize + blankSpacePixelSize;
	return totalPixelsSizeCountryName;
}

/* //added by Rolando 04-11-08
 * initFlagListWidget: initializes QtFlagsListWidget with items according QtFlagsManager
 * parameters:  QtFlagsManager * qtFlagsManager, QString flagsNameLanguage
 * returns QString, the country name default stored in config
*/
QString QtFlagsListWidget::init(QtFlagsManager * qtFlagsManager, QComboBox * comboBox, QString flagsNameLanguage){
	_qListWidget->clear();//VOXOX CHANGE by Rolando - 2009.06.12 
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	QString defaultFlagNameFile = QString::fromStdString(config.getCurrentFlag());//gets the default flagname file according config
	QString countryNameDefault = QString::null;

	QListWidgetItem *newItem = new QListWidgetItem();
	QFont font = _qListWidget->font();//VOXOX CHANGE by Rolando - 2009.06.12 
	
	//gets the largest country name text and sets its name to the variable
	QString largestCountryNameText = qtFlagsManager->getLargestCountryNameText();	

	//gets the largest country name size in pixels
	int largestCountryNamePixelSize = qtFlagsManager->getPixelSizeText(largestCountryNameText, font);


	int blankSpacePixelSize = qtFlagsManager->getPixelSizeText(QString(" "), font);
	int largestCountryCodePixelSize = 0;
	
	//gets the flaglist according flagsNameLanguage
	QtFlagsManager::QtFlagList flagList = qtFlagsManager->getQtFlagList(flagsNameLanguage);
	QtFlagsManager::QtFlagList::iterator it;
	QString countryName;
	QString areaCode;
	int numberTimes = 0;
	int tmpCountryCodePixelSize = 0;
	for (it = flagList.begin(); it != flagList.end(); it++) {
		countryName = it->getDefaultCountryName();//gets the country name
		areaCode = it->getDefaultCountryCode();
		//gets number times what FILL_CHAR is needed to justify the country name with code area number
		numberTimes = numberTimesToFillWithChar(FILL_CHAR, largestCountryNamePixelSize, countryName, font);
		if(numberTimes == -1){// if it is not necessary to fill with FILL_CHAR
			numberTimes = 0;
		}
		
		newItem = new QListWidgetItem(QIcon(it->getPixmap()),it->getDefaultCountryName().leftJustified(countryName.size()+numberTimes, FILL_CHAR) + QString(" ") + it->getDefaultCountryCode(),_qListWidget);
		
		if(defaultFlagNameFile == it->getFlagNameFile()){//if the country name is the default then sets their corresponding flag image to flagButton and selects it on _qtFlagsListWidget
			setDefaultFlag(newItem);			
			countryNameDefault = countryName;
			
		}
	}
	
	//sortItems();	
	
	//adds a item with text "None" because if we want to dial to another country not available on list or to dial a special phone number not must choose an area code, i.e the voice mail
	newItem = new QListWidgetItem(QIcon(QString::fromStdString(":/flags/none.png")),QString("None"));
	_qListWidget->insertItem ( 0, newItem );

	int totalPixelsSizeCountryName = getWidthNeededInItem(qtFlagsManager, flagsNameLanguage);
	setWidthSize(totalPixelsSizeCountryName);

	return countryNameDefault;
		
}

QPixmap QtFlagsListWidget::getCurrentFlagPixmap(){
	QtFlagsManager * qtFlagsManager = new QtFlagsManager();
	QtFlag qtFlag = qtFlagsManager->getFlagByCountryName(getCurrentCountryName(), FLAGS_NAME_LANGUAGE);
	return qtFlag.getPixmap();
}

//added by Rolando 04-11-08
void QtFlagsListWidget::keyReleaseEvent ( QKeyEvent * event ){
	switch (event->key()){
		case Qt::Key_Enter:
		case Qt::Key_Return:
			currentFlagChanged(getCurrentCountryName());
			close();
			break;			
		default:			
			QDialog::keyReleaseEvent(event);	
	}
}

//added by Rolando 04-11-08
void QtFlagsListWidget::leaveEvent ( QEvent * e ){
	currentFlagChanged(getCurrentCountryName());
	close();
}

void QtFlagsListWidget::focusOutEvent ( QFocusEvent * ){
	currentFlagChanged(getCurrentCountryName());
	close();
}

void QtFlagsListWidget::currentTextChangedSlot( QString currentText ){
	currentFlagChanged(getCurrentCountryName());	
}

void QtFlagsListWidget::itemClickedSlot(QListWidgetItem * item){
	close();
}