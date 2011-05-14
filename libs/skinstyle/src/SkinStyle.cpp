/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2005  Wengo
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

#include "SkinStyle.h"

#include "SkinWidget.h"
#include "SkinWindow.h"
#include "SkinButton.h"

#include <qtutil/Object.h>
#include <qtutil/NonResizeableWidget.h>

#include <QtGui/QtGui>

#include <iostream>
using namespace std;

static const QString STYLE_NAME = "skinstyle";

class SkinStylePlugin : public QStylePlugin {
public:

	QStringList keys() const {
		return QStringList() << STYLE_NAME;
	}

	QStyle * create(const QString & key) {
		if (key == STYLE_NAME) {
			return new SkinStyle();
		}
		return NULL;
	}
};


Q_EXPORT_PLUGIN(SkinStylePlugin)


SkinStyle::SkinStyle() {
	_scrollbarHandlePix = new QPixmap(_info.getSkinPath() + "scrollbarHandle.png");
	_scrollbarHandleHeight = _scrollbarHandlePix->height();

	_arrowUpPix = new QPixmap(_info.getSkinPath() + "arrowUp.png");
	_arrowUpWidth = _arrowUpPix->width();
	_arrowUpHeight = _arrowUpPix->height();
}

SkinStyle::~SkinStyle() {
	delete _scrollbarHandlePix;
	delete _arrowUpPix;
}

void SkinStyle::polish(QWidget * widget) {

	widget->setAttribute(Qt::WA_Hover, true);

	QString objectName = widget->objectName();

	if (widget->inherits("QAbstractButton")) {
		skinButton(widget, objectName);
	}

	else if (widget->inherits("QFrame") || widget->inherits("QGroupBox")) {
		skinFrame(widget, objectName);
	}

	else if (widget->isWindow() &&
		(widget->inherits("QMainWindow") || widget->inherits("QDialog"))) {

		skinWindow(widget, objectName);
	}

	else {
		skinWidget(widget, objectName);
	}

	if (widget->inherits("QMainWindow")) {
		QMainWindow * mainWindow = qobject_cast < QMainWindow * > (widget);

		//toolBars
		QList < QToolBar * > toolBars = mainWindow->findChildren < QToolBar * > ();
		for (int i = 0; i < toolBars.size(); i++) {
			QString toolBarObjectName = objectName + "-toolBar" + QString::number(i);
			if (!objectName.isEmpty()) {
				skinToolBar(toolBars.at(i), toolBarObjectName);
			}
		}

		//menuBar
		QList < QMenuBar * > menuBars = mainWindow->findChildren < QMenuBar * > ();
		if (!menuBars.isEmpty()) {
			skinMenuBar(menuBars.at(0));
		}

		//statusBar
		QList < QStatusBar * > statusBars = mainWindow->findChildren < QStatusBar * > ();
		if (!statusBars.isEmpty()) {
			skinStatusBar(statusBars.at(0));
		}

		//removeLayout(widget);
	}

	//Creates close and minimize buttons
	if (widget->isWindow() && !objectName.isEmpty()) {

		//closeButton
		QString closeButtonObjectName = objectName + "-closeButton";
		if (_info.contains(closeButtonObjectName)) {
			createButton(closeButtonObjectName, widget, SLOT(close()));
		}

		//minimizeButton
		QString minimizeButtonObjectName = objectName + "-minimizeButton";
		if (_info.contains(minimizeButtonObjectName)) {
			createButton(minimizeButtonObjectName, widget, SLOT(showMinimized()));
		}
	}

	//Object::print(widget);
}

void SkinStyle::addWidgetToLayout(QWidget * widget, QLayout * layout) {
	widget->setParent(NULL);
	layout->addWidget(widget);
}

void SkinStyle::createMenuBarContainer(QWidget * widget, const QString & objectName) {
	QHBoxLayout * layout = new QHBoxLayout();
	layout->setMenuBar(widget);

	QWidget * container = new QWidget(widget->parentWidget());
	container->setObjectName(objectName);
	container->setLayout(layout);
}

void SkinStyle::createWidgetsContainer(QWidget * widget, const QString & objectName) {
	QWidget * container = new QWidget(widget->parentWidget());
	container->setObjectName(objectName);

	QHBoxLayout * layout = new QHBoxLayout();
	container->setLayout(layout);

	QObjectList objects = widget->children();
	for (int i = 0; i < objects.size(); i++) {
		QObject * object = objects.at(i);

		if (object->isWidgetType()) {
			if (!object->inherits("QToolBarHandle") && !object->inherits("QToolBarExtension")) {
				QWidget * w = qobject_cast < QWidget * > (object);
				addWidgetToLayout(w, layout);
				w->show();
			}
		}

		else if (object->inherits("QAction")) {
			QAction * a = qobject_cast < QAction * > (object);
			QWidget * w = new QWidget(container);
			w->addAction(a);
			addWidgetToLayout(w, layout);
		}

		else if (object->inherits("QLayout")) {
			QLayout * l = qobject_cast < QLayout * > (object);
			layout->addLayout(l);
		}
	}

	container->show();
}

void SkinStyle::createButton(const QString & objectName, QWidget * parent, const char * member) {
	QPushButton * button = new QPushButton(parent);
	button->setObjectName(objectName);
	connect(button, SIGNAL(clicked()),
		parent, member);
}

void SkinStyle::skinToolBar(QToolBar * toolBar, const QString & objectName) {
	if (_info.contains(objectName)) {
		//toolBar->setMovable(false);
		//toolBar->setAllowedAreas(Qt::TopToolBarArea);

		//Qt crash inside resizeEvent()
		//new NonResizeableWidget(toolBar);

		createWidgetsContainer(toolBar, objectName);
		toolBar->hide();
	}
}

void SkinStyle::skinMenuBar(QMenuBar * menuBar) {
	QString parentObjectName = menuBar->parentWidget()->objectName();
	if (parentObjectName.isEmpty()) {
		return;
	}

	QString objectName = parentObjectName + "-menuBar";
	if (_info.contains(objectName)) {
		createMenuBarContainer(menuBar, objectName);
		//window->setMenuBar(NULL);
	}
}

void SkinStyle::skinStatusBar(QStatusBar * statusBar) {
	Object::print(statusBar->parentWidget());
	QString parentObjectName = statusBar->parentWidget()->objectName();
	if (parentObjectName.isEmpty()) {
		return;
	}

	QString objectName = parentObjectName + "-statusBar";
	if (_info.contains(objectName)) {
		statusBar->setSizeGripEnabled(false);
		createWidgetsContainer(statusBar, objectName);
		statusBar->hide();
		//window->setStatusBar(NULL);
		//delete statusBar;
	}
}

void SkinStyle::skinWidget(QWidget * widget, const QString & objectName) {
	SkinWidget * skinWidget = new SkinWidget(widget);

	setWidgetPosition(skinWidget, objectName);
	setWidgetSize(skinWidget, objectName);
	setMask(skinWidget, objectName);
	setPixmaps(skinWidget, objectName);
	setWidgetBackground(skinWidget, objectName);
	hide(skinWidget, objectName);
}

void SkinStyle::skinButton(QWidget * widget, const QString & objectName) {
	SkinButton * button = new SkinButton(widget);

	cout << "object name: " << objectName.toStdString() << endl;

	setWidgetPosition(button, objectName);
	setWidgetSize(button, objectName);
	setMask(button, objectName);
	setPixmaps(button, objectName);
	hide(button, objectName);
}

void SkinStyle::skinFrame(QWidget * widget, const QString & objectName) {
	SkinWidget * frame = new SkinWidget(widget);

	setWidgetPosition(frame, objectName);
	setWidgetSize(frame, objectName);
	setPixmaps(frame, objectName);
	setWidgetBackground(frame, objectName);
}

void SkinStyle::skinWindow(QWidget * widget, const QString & objectName) {
	SkinWindow * window = new SkinWindow(widget);

	setMask(window, objectName);
	setWidgetBackground(window, objectName);
}



void SkinStyle::setWidgetPosition(SkinWidget * widget, const QString & objectName) {
	int xPos = _info.getXPosition(objectName);
	int yPos = _info.getYPosition(objectName);
	if (xPos != SkinInfoReader::VALUE_ERROR || yPos != SkinInfoReader::VALUE_ERROR) {
		removeLayout(widget->getWidget());
		widget->setPosition(xPos, yPos);
	}
}

void SkinStyle::setWidgetSize(SkinWidget * widget, const QString & objectName) {
	int width = _info.getWidth(objectName);
	int height = _info.getHeight(objectName);
	widget->setSize(width, height);
}

void SkinStyle::setWidgetBackground(SkinWidget * widget, const QString & objectName) {
	QString backgroundPix = _info.getBackgroundPixmap(objectName);
	widget->setBackgroundPixmap(new QPixmap(backgroundPix));
}

void SkinStyle::setMask(SkinWidget * widget, const QString & objectName) {
	QString mask = _info.getMask(objectName);
	widget->setMask(new QBitmap(mask));
}

void SkinStyle::setPixmaps(SkinWidget * widget, const QString & objectName) {
	QString regularPix = _info.getRegularPixmap(objectName);
	QString activePix = _info.getActivePixmap(objectName);
	QString hoverPix = _info.getHoverPixmap(objectName);
	QString disablePix = _info.getDisablePixmap(objectName);
	widget->setPixmaps(new QPixmap(regularPix), new QPixmap(activePix),
			new QPixmap(hoverPix), new QPixmap(disablePix));

	/*cout << "Pixmaps: " << regularPix.toStdString() << ", "
			<< activePix.toStdString() << ", "
			<< hoverPix.toStdString() << ", "
			<< disablePix.toStdString() << endl;*/
}

void SkinStyle::hide(SkinWidget * widget, const QString & objectName) {
	bool hidden = _info.isHidden(objectName);
	if (hidden) {
		cout << "HIDE" << endl;
		widget->hide();
	}
}

void SkinStyle::removeLayout(QWidget * widget) {
	QWidget * parent = widget->parentWidget();
	if (parent) {
		QLayout * layout = parent->layout();
		if (layout) {
			layout->removeWidget(widget);
			QList<QLayout *> layouts = layout->findChildren<QLayout *>();
			for (int i = 0; i < layouts.size(); i++) {
				layouts.at(i)->removeWidget(widget);
			}
		}
	}
}

void SkinStyle::drawPixmap(const QString & pixmapFilename, const QRect & rect, QPainter * painter, bool rotate) const {
	QPixmap pix(_info.getSkinPath() + pixmapFilename);
	if (pix.isNull()) {
		return;
	}

	if (rotate) {
		//pix.scaledToWidth(rect.height());
		pix = pix.scaled(rect.height(), rect.width());
		QMatrix m;
		m.rotate(90.0);
		pix = pix.transformed(m);
	}
	painter->drawPixmap(rect.x(), rect.y(), pix);
}

void SkinStyle::drawScaledPixmap(const QString & pixmapFilename, const QRect & rect, QPainter * painter, bool rotate) const {
	QPixmap pix(_info.getSkinPath() + pixmapFilename);
	QPixmap scaledPix;

	if (pix.isNull()) {
		return;
	}

	if (rotate) {
		//pix.scaledToWidth(rect.height());
		//pix.scaled(rect.height(), rect.width());
		scaledPix = pix.scaled(rect.height(), rect.width());
		QMatrix m;
		m.rotate(90.0);
		scaledPix = scaledPix.transformed(m);
	} else {
		scaledPix = pix.scaled(rect.width(), rect.height());
	}
	painter->drawPixmap(rect.x(), rect.y(), scaledPix);
}

//Draws the ControlElement element with painter with the style options specified by option.
void SkinStyle::drawControl(ControlElement element, const QStyleOption * option, QPainter * painter,
		const QWidget * widget) const {

	SystemStyle::drawControl(element, option, painter, widget);
}

//Draws the ComplexControl control using painter with the style options specified by option.
void SkinStyle::drawComplexControl(ComplexControl control, const QStyleOptionComplex * option,
	QPainter * painter, const QWidget * widget) const {

	bool horizontal = (option->state & State_Horizontal);
	bool hover = (option->state & State_MouseOver);
	bool active = (option->state & State_Sunken);
	QString ext;

	if (hover) {
		if (active) {
			ext = "-active.png";
		} else {
			ext = "-hover.png";
		}
	} else {
		ext = ".png";
	}

	switch (control) {

	case CC_Slider:
		if (const QStyleOptionSlider * slider = qstyleoption_cast < const QStyleOptionSlider * > (option)) {

			QRect groove = subControlRect(CC_Slider, option, SC_SliderGroove, widget);
			QRect handle = subControlRect(CC_Slider, option, SC_SliderHandle, widget);

			//Paint the area where the slider handle may move
			//SC_SliderGroove: special sub-control which contains the area in which the slider handle may move
			if ((option->subControls & SC_SliderGroove) && groove.isValid()) {

				//Horizontal slider
				if (horizontal) {
					drawScaledPixmap("sliderGroove.png", groove, painter, true);
				}

				//Vertical slider
				else {
					drawScaledPixmap("sliderGroove.png", groove, painter);
				}
			}

			//Paint slider handle
			if ((option->subControls & SC_SliderHandle) && handle.isValid()) {

				//Horizontal slider
				if (horizontal) {
					drawScaledPixmap("sliderHandle" + ext, handle, painter, true);
				}

				//Vertical slider
				else {
					drawScaledPixmap("sliderHandle" + ext, handle, painter);
				}
			}

			//Slider tickmarks
			if (option->subControls & SC_SliderTickmarks) {
				QStyleOptionComplex tickOpt = *option;
				tickOpt.subControls = SC_SliderTickmarks;
				SystemStyle::drawComplexControl(control, &tickOpt, painter, widget);
			}
		}
		break;

	case CC_ScrollBar:
		if (const QStyleOptionSlider * scrollBar = qstyleoption_cast < const QStyleOptionSlider * > (option)) {

			//Keep geometry of scrollbar elements
			QRect groove = subControlRect(CC_ScrollBar, option, SC_ScrollBarGroove, widget);
			QRect handle = subControlRect(CC_ScrollBar, option, SC_ScrollBarSlider, widget);
			QRect addLine = subControlRect(CC_ScrollBar, option, SC_ScrollBarAddLine, widget);
			QRect subLine = subControlRect(CC_ScrollBar, option, SC_ScrollBarSubLine, widget);

			//Paint the area where the slider handle may move
			if ((option->subControls & SC_ScrollBarGroove) && groove.isValid()) {

				//Horizontal scrollbar
				if (horizontal) {
					drawScaledPixmap("scrollbarGroove.png", groove, painter, true);
				}

				//Vertical scrollbar
				else {
					drawScaledPixmap("scrollbarGroove.png", groove, painter);
				}

				//QCommonStyle::drawComplexControl(control, option, painter, widget);
			}

			//Paint the scrollbar addline (i.e., down/right arrow); see also QScrollBar
			if ((option->subControls & SC_ScrollBarAddLine) && addLine.isValid()) {

				//Horizontal scrollbar
				if (horizontal) {
					if ((scrollBar->activeSubControls & SC_ScrollBarAddLine)) {
						drawPixmap("arrowUp" + ext, addLine, painter, true);
					} else {
						drawPixmap("arrowUp.png", addLine, painter, true);
					}
				}

				//Vertical scrollbar
				else {
					if ((scrollBar->activeSubControls & SC_ScrollBarAddLine)) {
						drawPixmap("arrowDown" + ext, addLine, painter);
					} else {
						drawPixmap("arrowDown.png", addLine, painter);
					}
				}
			}

			//Paint the scrollbar subline (i.e., up/left arrow); see also QScrollBar
			if ((option->subControls & SC_ScrollBarSubLine) && subLine.isValid()) {

				//Horizontal scrollbar
				if (horizontal) {
					//Active or hover state
					if ((scrollBar->activeSubControls & SC_ScrollBarSubLine)) {
						drawPixmap("arrowDown" + ext, subLine, painter, true);
					} else {
						drawPixmap("arrowDown.png", subLine, painter, true);
					}
				}

				//Vertical scrollbar
				else {
					if ((scrollBar->activeSubControls & SC_ScrollBarSubLine)) {
						drawPixmap("arrowUp" + ext, subLine, painter);
					} else {
						drawPixmap("arrowUp.png", subLine, painter);
					}
				}
			}

			//Paint scrollbar slider handle
			if ((option->subControls & SC_ScrollBarSlider) && handle.isValid()) {
				//Horizontal scrollbar
				if (horizontal) {
					//Active or hover state
					if ((scrollBar->activeSubControls & SC_ScrollBarSlider)) {
						drawPixmap("scrollbarHandle" + ext, handle, painter, true);
					} else {
						drawPixmap("scrollbarHandle.png", handle, painter, true);
					}
				}

				//Vertical scrollbar
				else {
					if ((scrollBar->activeSubControls & SC_ScrollBarSlider)) {
						drawPixmap("scrollbarHandle" + ext, handle, painter);
					} else {
						drawPixmap("scrollbarHandle.png", handle, painter);
					}
				}
			}
		}
		break;

	default:
		SystemStyle::drawComplexControl(control, option, painter, widget);

	}
}

QRect SkinStyle::subControlRect(ComplexControl control, const QStyleOptionComplex * option,
				SubControl subControl, const QWidget * widget) const {

	QRect rect = SystemStyle::subControlRect(control, option, subControl, widget);

	switch (control) {
	case CC_Slider:
		if (const QStyleOptionSlider * slider = qstyleoption_cast<const QStyleOptionSlider *>(option)) {
			if (subControl == SC_SliderHandle) {
				int tickOffset = pixelMetric(PM_SliderTickmarkOffset, option, widget);
				int thickness = pixelMetric(PM_SliderControlThickness, option, widget);
				bool horizontal = slider->orientation == Qt::Horizontal;
				int len = pixelMetric(PM_SliderLength, option, widget) + 8;	//size of handle / 2
				int motifBorder = pixelMetric(PM_DefaultFrameWidth);
				int sliderPos = sliderPositionFromValue(slider->minimum, slider->maximum, slider->sliderPosition,
						horizontal ? slider->rect.width() - len - 2 * motifBorder : slider->rect.height() - len - 2 * motifBorder,
						slider->upsideDown);

				if (horizontal) {
					rect = visualRect(slider->direction, slider->rect,
							QRect(sliderPos + motifBorder, tickOffset + motifBorder,
								len, thickness - 2 * motifBorder));
				} else {
					rect = visualRect(slider->direction, slider->rect,
							QRect(tickOffset + motifBorder, sliderPos + motifBorder,
								thickness - 2 * motifBorder, len));
				}
			}
		}
		break;

	case CC_ScrollBar:
		if (const QStyleOptionSlider * scrollbar = qstyleoption_cast<const QStyleOptionSlider *>(option)) {
			QRect scrollBarRect = scrollbar->rect;

			int grooveMargin = pixelMetric(PM_ButtonDefaultIndicator, scrollbar, widget);

			int scrollBarExtent = pixelMetric(PM_ScrollBarExtent, scrollbar, widget);
			int sliderMaxLength = ((scrollbar->orientation == Qt::Horizontal) ?
						scrollbar->rect.width() + grooveMargin : scrollbar->rect.height()) - 2 * (_arrowUpWidth);

			int sliderLength = _scrollbarHandleHeight;
			int sliderStart = _arrowUpWidth + sliderPositionFromValue(scrollbar->minimum,
						scrollbar->maximum, scrollbar->sliderPosition,
						sliderMaxLength - sliderLength, scrollbar->upsideDown);

			switch (subControl) {
			case SC_ScrollBarSlider:	//Handle
				if (scrollbar->orientation == Qt::Horizontal) {
					rect.setRect(sliderStart, 0, sliderLength, scrollBarExtent);
				} else {
					rect.setRect(0, sliderStart, scrollBarExtent, sliderLength);
				}
				break;

			case SC_ScrollBarSubLine:	//Top/left button
				if (scrollbar->orientation == Qt::Horizontal) {
					rect.setRect(scrollBarRect.left(), scrollBarRect.top(), _arrowUpWidth, scrollBarExtent);
				} else {
					rect.setRect(scrollBarRect.left(), scrollBarRect.top(), scrollBarExtent, _arrowUpHeight);
				}
				break;

			case SC_ScrollBarAddLine:	//Bottom/right button
				if (scrollbar->orientation == Qt::Horizontal) {
					rect.setRect(scrollBarRect.right() - _arrowUpWidth + grooveMargin, scrollBarRect.top(),
							_arrowUpWidth, _arrowUpHeight);
				} else {
					rect.setRect(scrollBarRect.left(), scrollBarRect.bottom() - _arrowUpWidth + grooveMargin,
							_arrowUpWidth, _arrowUpHeight);
				}
				break;

			case SC_ScrollBarGroove:
				if (scrollbar->orientation == Qt::Horizontal) {
					rect = scrollBarRect.adjusted(_arrowUpWidth, - 2 * grooveMargin, - _arrowUpWidth + grooveMargin, 0);
				} else {
					rect = scrollBarRect.adjusted(-grooveMargin, _arrowUpWidth, 0, - _arrowUpWidth + grooveMargin);
				}
				break;

			default:
				break;
			}
			rect = visualRect(scrollbar->direction, scrollBarRect, rect);
		}
		break;

	default:
		break;
	}

	return rect;
}

int SkinStyle::pixelMetric(PixelMetric metric, const QStyleOption * option, const QWidget * widget) const {
	int ret = 0;
	QPixmap arrowPix(_info.getSkinPath() + "arrowUp.png");

	switch(metric) {
	case PM_ScrollBarExtent:
		ret = 15;
		//ret = _arrowUpWidth;
		break;

	case PM_ScrollBarSliderMin:
		ret = _arrowUpHeight;
		break;

	case PM_ButtonDefaultIndicator:
		ret = 1;
		break;

	default:
		ret = SystemStyle::pixelMetric(metric, option, widget);
		break;
	}

	return ret;
}
