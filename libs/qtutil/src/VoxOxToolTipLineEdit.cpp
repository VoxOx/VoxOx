/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2006  Wengo
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <qtutil/VoxOxToolTipLineEdit.h>

#include <qtutil/LanguageChangeEventFilter.h>
#include <qtutil/SafeConnect.h>
#include <QtGui/QtGui>

//static const int MAXIMUM_LENGTH_ALLOWED = 32;//VOXOX CHANGE by Rolando - 2009.10.13 

VoxOxToolTipLineEdit::VoxOxToolTipLineEdit(QWidget * parent)
	: QLineEdit(0) {

	_parentWidget = parent;//VOXOX CHANGE by Rolando - 2009.05.22 - varible to send focus when it is needed

	LANGUAGE_CHANGE(this);

	_primaryStyleSheet = QString("QWidget{ border: none; color: white; background: black; }");
	_secondaryStyleSheet = QString("QWidget{ border: none; color: gray; background: black; }");

	SAFE_CONNECT(this, SIGNAL(textChanged(QString)), SLOT(textChangedSlot(QString)));//VOXOX CHANGE Rolando 03-24-09
	SAFE_CONNECT(this, SIGNAL(textEdited(QString)), SLOT(textChangedSlot(QString)));//VOXOX CHANGE Rolando 03-24-09

	_shortMessage = "";
	_message = "";

	init();
}

//VOXOX CHANGE by Rolando - 2009.05.22 - method to send focus to another widget when it is necessary
void VoxOxToolTipLineEdit::changeFocusToParent(){
	if(_parentWidget){
		_parentWidget->setFocus();
	}
}
void VoxOxToolTipLineEdit::init() {
	_cleared = false;
	_toolTipTextDone = false;
	_maximumWidthPixels = width();//VOXOX CHANGE by Rolando - 2009.10.13 
}

void VoxOxToolTipLineEdit::setText(const QString & text) {
	clearLineEdit();
	updateMessageText(text);
	QLineEdit::setText(_shortMessage);
	
}

//VOXOX CHANGE by Rolando - 2009.05.11 - updates variables _shortMessage and _message according parameter text
void VoxOxToolTipLineEdit::updateMessageText(QString text){

	//VOXOX CHANGE by Rolando - 2009.06.18 - bug fixed when a user changes current text and this one was ending with ellipsis i.e "some large text...e"

	//VOXOX CHANGE by Rolando - 2009.10.13 
	if(text.length() > getMaximumCharsAllowed() && text.indexOf(QRegExp(QString("^%1.$").arg(_shortMessage))) != -1){//VOXOX CHANGE by Rolando - 2009.06.18 - check if current text contains to _shortMessage and ends with a different character i.e "shortMessage...e"
		QString lastChar = text.at(text.length()-1);//VOXOX CHANGE by Rolando - 2009.06.18 -  gets the last char of current text
		text = _message + lastChar;//VOXOX CHANGE by Rolando - 2009.06.18 - merge _message with lastChar and updates text value
	}

	if(_shortMessage != text){
		_message = text;

		////if message is larger than maximum length of status message, removes that difference and adds ellipsis (...)
		//if(text.length()> _maximumWidthPixels){//VOXOX CHANGE by Rolando - 2009.10.13
		//	//VOXOX CHANGE by Rolando - 2009.10.13 
		//	text = text.remove(_maximumWidthPixels-3,text.length());//VOXOX CHANGE by Rolando - 2009.10.13 
		//	text += "...";
		//}

		_shortMessage = verifyText(rect(), font(), text);//VOXOX CHANGE by Rolando - 2009.10.13 



		//_shortMessage = text;//VOXOX CHANGE by Rolando - 2009.10.13 
		
	}
	repaintPrimaryColor();
}

QString VoxOxToolTipLineEdit::verifyText(const QRect & painterRect, QFont font, QString text){
	int xText = painterRect.left();
	int textWidth = QFontMetrics(font).width(text);//gets new "x" position according width's text	
	
	bool fixedText = false;

	while(xText + textWidth > _maximumWidthPixels){//we need to check that we can paint on the QtContactWidget but not on the vertical line
		text = text.remove(text.length()- 1,text.length());
		textWidth = QFontMetrics(font).width(text);//gets new "x" position according width's text
		fixedText = true;
	}
	
	if(fixedText){
		if(text.length() >= 3){			
			text = text.remove(text.length()- 3,text.length() );
			text += "...";
		}		
	}
	return text;
}


//VOXOX CHANGE by Rolando - 2009.05.11 - Set default text when focus is lost and there is not text on lineedit
void VoxOxToolTipLineEdit::setToolTipDefaultText(const QString & text) {
	if(text != ""){
		_toolTipTextDone = true;
		_toolTip = text;
		displayToolTipMessage();
	}
}

//VOXOX CHANGE by Rolando - 2009.05.11 - gets text from lineedit
QString VoxOxToolTipLineEdit::text() const {
	if (QLineEdit::text() == _toolTip) {
		return QString::null;
	} else {
		return QLineEdit::text();
	}
}

//VOXOX CHANGE by Rolando - 2009.05.11 - gets long message instead message with "..."  if it is larger than allowed length size
QString VoxOxToolTipLineEdit::realText() const {	
	return _message;	
}

void VoxOxToolTipLineEdit::displayToolTipMessage() {
	//Text color is grey
	QLineEdit::setText(_toolTip);
	repaintSecondaryColor();
	_cleared = false;
	_message = "";
	_shortMessage = "";
}

void VoxOxToolTipLineEdit::clearLineEdit() {
	//Text color back to original color
	repaintPrimaryColor();

	clear();
	_cleared = true;
}

void VoxOxToolTipLineEdit::mousePressEvent(QMouseEvent * event) {
	if (!_cleared) {
		clearLineEdit();
		repaintPrimaryColor();
	}
	
	QLineEdit::mousePressEvent(event);
}

void VoxOxToolTipLineEdit::enterEvent ( QEvent * event )   {
	if (_cleared) {
		int cursorPositionValue =  cursorPosition();//VOXOX CHANGE by Rolando - 2009.06.02 - gets current Cursor Position
		int intSelectedStart = -1;
		QString stringText;

		if(hasSelectedText()){//VOXOX CHANGE by Rolando - 2009.06.02 - if there is selected text
			stringText = selectedText();//VOXOX CHANGE by Rolando - 2009.06.02 - gets selected text
			intSelectedStart = selectionStart();//VOXOX CHANGE by Rolando - 2009.06.02 - gets position where selection starts		
		}

		if(_shortMessage == QLineEdit::text()){
			QLineEdit::setText(_message);
		}

		//VOXOX CHANGE by Rolando - 2009.10.13 
		if(intSelectedStart >= 0 && intSelectedStart + stringText.length() <= getMaximumCharsAllowed()){//VOXOX CHANGE by Rolando - 2009.06.02 - if there is a text selected
			setSelection(intSelectedStart, stringText.length());//VOXOX CHANGE by Rolando - 2009.06.02 - as we set the long message in lineedit, we need to set the text was selected
		}
	}
	QLineEdit::enterEvent(event);
}

void VoxOxToolTipLineEdit::keyPressEvent(QKeyEvent * event) {
	if (!_cleared) {
		clearLineEdit();
		repaintPrimaryColor();
	}

	if (event->key()==Qt::Key_Return || event->key()==Qt::Key_Enter) {//if key pressed was return or enter
		if(!QLineEdit::text().isEmpty()){
			updateMessageText(QLineEdit::text());
			QLineEdit::setText(_shortMessage);
			currentTextChanged(text());
		}
		
		QLineEdit::keyPressEvent(event);
		changeFocusToParent();//VOXOX CHANGE by Rolando - 2009.05.22 - method to send focus to another widget when mouse leaves the lineedit
	}
	else{//if key pressed was not return or enter key
		int cursorPositionValue =  cursorPosition();//VOXOX CHANGE by Rolando - 2009.06.02 - gets current Cursor Position
		bool isShortMessage = QLineEdit::text() == _shortMessage;//VOXOX CHANGE by Rolando - 2009.06.02 - we need to check if current text is equal to _shortMessage
		int intSelectedStart = -1;
		QString text;

		if(hasSelectedText()){//VOXOX CHANGE by Rolando - 2009.06.02 - if there is selected text
			text = selectedText();//VOXOX CHANGE by Rolando - 2009.06.02 - gets selected text
			intSelectedStart = selectionStart();//VOXOX CHANGE by Rolando - 2009.06.02 - gets position where selection starts		
		}

		QLineEdit::setText(_message);//VOXOX CHANGE by Rolando - 2009.06.02 - sets long message 
		if(intSelectedStart >= 0){//VOXOX CHANGE by Rolando - 2009.06.02 - if there is a text selected
			setSelection(intSelectedStart, text.length());//VOXOX CHANGE by Rolando - 2009.06.02 - as we set the long message in lineedit, we need to set the text was selected
		}
		else{
			//VOXOX CHANGE by Rolando - 2009.06.02 - if there is not a text selected
			
			if(isShortMessage){//VOXOX CHANGE by Rolando - 2009.06.02 - if we had currentText equal to _shortMessage
				//VOXOX CHANGE by Rolando - 2009.10.13 
				if(cursorPositionValue == _shortMessage.length()){//VOXOX CHANGE by Rolando - 2009.06.02 - if cursor position is the rightest pos allowed
					setCursorPosition(_message.length());//VOXOX CHANGE by Rolando - 2009.06.02 - we move the cursor to rightest position after we changed the current text by _message
				}
				else{
					//VOXOX CHANGE by Rolando - 2009.06.02 - if cursor position is not the rightest pos allowed
					setCursorPosition(cursorPositionValue);//VOXOX CHANGE by Rolando - 2009.06.02 - if cursor position is not the rightest pos allowed
				}
			}
			else{
				//VOXOX CHANGE by Rolando - 2009.06.02 - if we had not currentText equal to _shortMessage
				setCursorPosition(cursorPositionValue);//VOXOX CHANGE by Rolando - 2009.06.02 - we move the cursor to old position + 1
			}
			
		}		
		
		QLineEdit::keyPressEvent(event);//processes event - it should be processed before update _shortMessage and _message variables
		updateMessageText(QLineEdit::text());//VOXOX CHANGE by Rolando - 2009.06.02 - we update the short and long messages
		
	}

	keyPressedSignal(event->key());
	
}

//VOXOX CHANGE by Rolando - 2009.05.11 - method called when lineedit lost focus
void VoxOxToolTipLineEdit::leaveEvent ( QEvent * event ){
	if (QLineEdit::text() == _toolTip || QLineEdit::text().isEmpty()) {//VOXOX CHANGE by Rolando - 2009.05.11 - if current text is equal to tooltip or is empty
		displayToolTipMessage();
		
	}
	else{
		int cursorPositionValue =  cursorPosition();//VOXOX CHANGE by Rolando - 2009.06.02 - gets current Cursor Position
		int intSelectedStart = -1;
		QString stringText;

		if(hasSelectedText()){//VOXOX CHANGE by Rolando - 2009.06.02 - if there is selected text
			stringText = selectedText();//VOXOX CHANGE by Rolando - 2009.06.02 - gets selected text
			intSelectedStart = selectionStart();//VOXOX CHANGE by Rolando - 2009.06.02 - gets position where selection starts		
		}
		
		if(_message == QLineEdit::text()){//VOXOX CHANGE by Rolando - 2009.05.11 - if current text is equal to _message then sets _shortMessage
			QLineEdit::setText(_shortMessage);
		}
		else{//VOXOX CHANGE by Rolando - 2009.05.11 - if current text is not equal to _message then updates _shortMessage and _message and then sets _shortMessage as current text
			updateMessageText(QLineEdit::text());
			QLineEdit::setText(_shortMessage);
		}

		//VOXOX CHANGE by Rolando - 2009.10.13 
		if(intSelectedStart >= 0 && intSelectedStart + stringText.length() <= getMaximumCharsAllowed()){//VOXOX CHANGE by Rolando - 2009.06.02 - if there is a text selected
			setSelection(intSelectedStart, stringText.length());//VOXOX CHANGE by Rolando - 2009.06.02 - as we set the long message in lineedit, we need to set the text was selected
		}

		currentTextChanged(text());
	}
	
	QLineEdit::leaveEvent(event);
	//changeFocusToParent();//VOXOX CHANGE by Rolando - 2009.05.22 - method to send focus to another widget when mouse leaves the lineedit
} 

void VoxOxToolTipLineEdit::textChangedSlot(QString newText){
	if(_toolTip != newText){
		repaintPrimaryColor();
	}
}

void VoxOxToolTipLineEdit::languageChanged() {
	init();
}

void VoxOxToolTipLineEdit::setPrimaryColorStyleSheet(QString stylesheet){
	_primaryStyleSheet = stylesheet;	
}

void VoxOxToolTipLineEdit::setSecondaryColorStyleSheet(QString stylesheet){
	_secondaryStyleSheet = stylesheet;	
}

void VoxOxToolTipLineEdit::updateStyleSheet(QString newStyleSheet){
	setStyleSheet(newStyleSheet);
}

void VoxOxToolTipLineEdit::repaintPrimaryColor(){
	updateStyleSheet(_primaryStyleSheet);
}

void VoxOxToolTipLineEdit::repaintSecondaryColor(){
	updateStyleSheet(_secondaryStyleSheet);
}

//VOXOX CHANGE by Rolando - 2009.10.13 
void VoxOxToolTipLineEdit::resizeEvent ( QResizeEvent * event ){
	_maximumWidthPixels = width();
	updateCurrentText();
	QLineEdit::resizeEvent(event);
}

//VOXOX CHANGE by Rolando - 2009.10.13 
int VoxOxToolTipLineEdit::getMaximumCharsAllowed(){
	QRect rectTmp = rect();
	int leftRectPos = rectTmp.left();
	int rightRectPos = rectTmp.right();
	QString text = "";
	int textWidth = 0;
	int charCounter = 0;//VOXOX CHANGE by Rolando - 2009.10.13 

	do {//we need to check that we can paint on the QtContactWidget but not on the vertical line
		text.append("0");
		textWidth = QFontMetrics(font()).width(text);//gets new "x" position according width's text
		charCounter++;

	} while(leftRectPos + textWidth < rightRectPos);
	
	return charCounter;//VOXOX CHANGE by Rolando - 2009.10.13 
}

//VOXOX CHANGE by Rolando - 2009.10.13 
void VoxOxToolTipLineEdit::updateCurrentText(){

	if(text() != ""){
		_shortMessage = verifyText(rect(), font(), _message);
		QLineEdit::setText(_shortMessage);
	}
}
