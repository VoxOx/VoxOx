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

#include "stdafx.h"		//VOXOX - JRT - 2009.04.01
#include "QtSVGDialpad.h"

#include <model/config/ConfigManager.h>
#include <model/config/Config.h>

#include <cutil/global.h>
#include <util/Logger.h>
#include <util/SafeDelete.h>
#include <util/File.h>

#include <qtutil/SafeConnect.h>

#include <QtGui/QtGui>
#include <QtSvg/QtSvg>

//QRgb -> #AARRGGBB
static const unsigned COLOR_0 = 0xFF000000;
static const unsigned COLOR_1 = 0xFFFF0000;
static const unsigned COLOR_2 = 0xFF00FF00;
static const unsigned COLOR_3 = 0xFFFFFF00;
static const unsigned COLOR_4 = 0xFF0000FF;
static const unsigned COLOR_5 = 0xFFFF00FF;
static const unsigned COLOR_6 = 0xFF00FFFF;
static const unsigned COLOR_7 = 0xFF990000;
static const unsigned COLOR_8 = 0xFF99FF00;
static const unsigned COLOR_9 = 0xFF9900FF;
static const unsigned COLOR_POUND = 0xFF99FFFF;
static const unsigned COLOR_STAR = 0xFF999999;

QtSVGDialpad::QtSVGDialpad(QWidget* parent)
	: QFrame(parent) {

	_svgMask = new QSvgRenderer();
	_mask = new QImage();

	_svgimage = new QSvgRenderer();

	_svgLayer = new QSvgRenderer();
	SAFE_CONNECT(_svgimage, SIGNAL(repaintNeeded()), SLOT(update()));

	setMouseTracking(true);
}

void QtSVGDialpad::setNewTheme(const std::string & themeRepertory) {

	//destroy old theme
	OWSAFE_DELETE(_svgMask);
	OWSAFE_DELETE(_svgimage);
	OWSAFE_DELETE(_svgLayer);
	OWSAFE_DELETE(_mask);

	//new repertory
	_repertory = themeRepertory;

	//new mask
	_svgMask = new QSvgRenderer(QString::fromStdString(_repertory + "dialpad_mask.svg"), this);
	_mask = new QImage();

	//new image
	_svgimage = new QSvgRenderer(QString::fromStdString(_repertory + "dialpad_image.svg"), this);

	//reinitialize layer
	_svgLayer = new QSvgRenderer();
}

QtSVGDialpad::~QtSVGDialpad() {
	OWSAFE_DELETE(_svgMask);
	OWSAFE_DELETE(_svgimage);
	OWSAFE_DELETE(_svgLayer);
	OWSAFE_DELETE(_mask);
}

void QtSVGDialpad::changeLayerFile(const std::string & newLayerFile) {
	
	QFile layerFile(QString::fromStdString(_repertory + newLayerFile));
	
	if ( (newLayerFile != _layerFile)  && (layerFile.exists()) ) {

		//delete old layer
		OWSAFE_DELETE(_svgLayer);

		if(newLayerFile.empty()) {
			//construct new empty layer
			_svgLayer = new QSvgRenderer(this);
		} else {
			//construct new layer
			_svgLayer = new QSvgRenderer(layerFile.fileName(), this);
			SAFE_CONNECT(_svgLayer, SIGNAL(repaintNeeded()), SLOT(update()));
		}

		//save _layerFile
		_layerFile = newLayerFile;

		update();
	}
}

void QtSVGDialpad::mouseMoveEvent(QMouseEvent *event) {

	//get mouse pos
	QPoint pos = event->pos();

	//get pixel color
	if (_mask->valid(pos.x(), pos.y())) {
		switch(_mask->pixel(pos.x(), pos.y())) {
			case COLOR_0: 		changeLayerFile("layer_0.svg"); 		break;
			case COLOR_1: 		changeLayerFile("layer_1.svg"); 		break;
			case COLOR_2: 		changeLayerFile("layer_2.svg"); 		break;
			case COLOR_3: 		changeLayerFile("layer_3.svg"); 		break;
			case COLOR_4: 		changeLayerFile("layer_4.svg"); 		break;
			case COLOR_5: 		changeLayerFile("layer_5.svg"); 		break;
			case COLOR_6: 		changeLayerFile("layer_6.svg"); 		break;
			case COLOR_7: 		changeLayerFile("layer_7.svg"); 		break;
			case COLOR_8: 		changeLayerFile("layer_8.svg"); 		break;
			case COLOR_9: 		changeLayerFile("layer_9.svg"); 		break;
			case COLOR_POUND: 	changeLayerFile("layer_pound.svg"); 	break;
			case COLOR_STAR: 	changeLayerFile("layer_star.svg"); 		break;
			default:			changeLayerFile("");
		}
	} else {
		changeLayerFile("");
	}
}

void QtSVGDialpad::mouseReleaseEvent(QMouseEvent * event) {

	if (event->button() == Qt::LeftButton) {

		//get mouse pos
		QPoint pos = event->pos();

		//get pixel color
		switch(_mask->pixel(pos.x(), pos.y())) {
			case COLOR_0: 		keyZeroSelected();		break;
			case COLOR_1: 		keyOneSelected(); 		break;
			case COLOR_2: 		keyTwoSelected(); 		break;
			case COLOR_3: 		keyThreeSelected(); 	break;
			case COLOR_4: 		keyFourSelected(); 		break;
			case COLOR_5: 		keyFiveSelected();		break;
			case COLOR_6: 		keySixSelected(); 		break;
			case COLOR_7: 		keySevenSelected();		break;
			case COLOR_8: 		keyEightSelected();		break;
			case COLOR_9: 		keyNineSelected(); 		break;
			case COLOR_POUND: 	keyPoundSelected(); 	break;
			case COLOR_STAR: 	keyStarSelected(); 		break;
		}
	}
}

void QtSVGDialpad::paintEvent(QPaintEvent *) {

	//mask
	if (_svgMask->isValid()) {
		if (_mask->size() != size()) {
			OWSAFE_DELETE(_mask);
			_mask = new QImage(size(), QImage::Format_ARGB32_Premultiplied);
			QPainter pm(_mask);
			pm.setViewport(0, 0, width(), height());
			pm.eraseRect(0, 0, width(), height());
			_svgMask->render(&pm);
		}
	}

	QPainter pi(this);
	pi.setViewport(0, 0, width(), height());

	//image
	if (_svgimage->isValid()) {
		_svgimage->render(&pi);
	}

	//layer
	if (_svgLayer->isValid()) {
		_svgLayer->render(&pi);
	}
}
