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

#include <qtutil/PixmapMerging.h>

#include <QtGui/QtGui>

QPixmap PixmapMerging::merge(const std::string & foregroundPixmapData, const std::string & backgroundPixmapFilename, Qt::AspectRatioMode foregroundAspectRadio, Qt::Alignment horizontalAlignment, Qt::Alignment verticalAlignment) {//VOXOX CHANGE by Rolando - 2009.06.27 
	int x = 0;
	int y = 0;
	QImage foregroundImage;
	foregroundImage.loadFromData((uchar *) foregroundPixmapData.c_str(), foregroundPixmapData.size());

	QPixmap backgroundPixmap = QPixmap(QString::fromStdString(backgroundPixmapFilename));

	QImage newForegroundImage = foregroundImage.scaled(backgroundPixmap.size(), foregroundAspectRadio, Qt::SmoothTransformation);//VOXOX CHANGE by Rolando - 2009.06.27 


	switch(horizontalAlignment){

		case Qt::AlignLeft:
			x = 0;
		break;

		case Qt::AlignRight:
			x = backgroundPixmap.width() - newForegroundImage.width();
			if(x < 0){
				x = 0;	
			}
		break;

		case Qt::AlignHCenter:
			x = backgroundPixmap.width() - newForegroundImage.width();
			if(x < 0){
				x = 0;	
			}
			x = x/2;
		break;

		case Qt::AlignJustify:
			x = 0;
			newForegroundImage = foregroundImage.scaled(backgroundPixmap.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
		break;

		default:;//No Qt::Alignment allowed
		
	}

	switch(verticalAlignment){
		case Qt::AlignTop:
			y = 0;
		break;

		case Qt::AlignBottom:
			y = backgroundPixmap.height() - newForegroundImage.height();
			if(y < 0){
				y = 0;	
			}
		break;

		case Qt::AlignVCenter:
			y = backgroundPixmap.height() - newForegroundImage.height();
			if(y < 0){
				y = 0;	
			}
			y = y/2;
		break;

		default:;//No Qt::Alignment allowed
	}

	if (!newForegroundImage.isNull()) {
		QPainter painter(&backgroundPixmap);
		painter.drawImage(x, y, newForegroundImage);
				//VOXOX CHANGE for VoxOx by Rolando 01-09-09, deleted for VoxOx purposes
				//foregroundImage.scaled(backgroundPixmap.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
		painter.end();
	}

	return backgroundPixmap;
}

//VOXOX CHANGE for VoxOx by Rolando 01-09-09
/*
 * This method merges the foregroundPixmapFilename on the backgroundPixmapFilename and returns the new Pixmap merged according alignment specified or default alignment
*/
QPixmap PixmapMerging::mergeFromPixmap(const std::string & foregroundPixmapFilename, const std::string & backgroundPixmapFilename, Qt::AspectRatioMode foregroundAspectRadio, Qt::Alignment horizontalAlignment, Qt::Alignment verticalAlignment) {//VOXOX CHANGE by Rolando - 2009.06.27 
	int x = 0;
	int y = 0;
	QPixmap foregroundPixmap = QPixmap(QString::fromStdString(foregroundPixmapFilename));
	QPixmap backgroundPixmap = QPixmap(QString::fromStdString(backgroundPixmapFilename));	
	QPixmap newForegroundPixmap = foregroundPixmap.scaled(backgroundPixmap.size(), foregroundAspectRadio, Qt::SmoothTransformation);//VOXOX CHANGE by Rolando - 2009.06.27 

	switch(horizontalAlignment){

		case Qt::AlignLeft:
			x = 0;
		break;

		case Qt::AlignRight:
			x = backgroundPixmap.width() - newForegroundPixmap.width();
			if(x < 0){
				x = 0;	
			}
		break;

		case Qt::AlignHCenter:
			x = backgroundPixmap.width() - newForegroundPixmap.width();
			if(x < 0){
				x = 0;	
			}
			x = x/2;
		break;

		case Qt::AlignJustify:
			x = 0;
			newForegroundPixmap = foregroundPixmap.scaled(backgroundPixmap.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
		break;

		default:;//No Qt::Alignment allowed
		
	}

	switch(verticalAlignment){
		case Qt::AlignTop:
			y = 0;
		break;

		case Qt::AlignBottom:
			y = backgroundPixmap.height() - newForegroundPixmap.height();
			if(y < 0){
				y = 0;	
			}
		break;

		case Qt::AlignVCenter:
			y = backgroundPixmap.height() - newForegroundPixmap.height();
			if(y < 0){
				y = 0;	
			}
			y = y/2;
		break;

		default:;//No Qt::Alignment allowed
	}

	if (!newForegroundPixmap.isNull()) {
		QPainter painter(&backgroundPixmap);
		painter.drawPixmap(x, y, newForegroundPixmap);
		painter.end();
	}

	return backgroundPixmap;
}

//VOXOX CHANGE for VoxOx by Rolando 01-09-09
/*
 * This method merges the foregroundPixmap on the backgroundPixmap and returns the new Pixmap merged according alignment specified or default alignment
*/
QPixmap PixmapMerging::mergeFromPixmap(QPixmap foregroundPixmap,QPixmap backgroundPixmap, Qt::AspectRatioMode foregroundAspectRadio, Qt::Alignment horizontalAlignment, Qt::Alignment verticalAlignment){//VOXOX CHANGE by Rolando - 2009.06.27 

	int x = 0;
	int y = 0;
	QSize foregroundPixmapSize = foregroundPixmap.size();
	QSize backgroundPixmapSize = backgroundPixmap.size();
	QPixmap newForegroundPixmap = foregroundPixmap.scaled(backgroundPixmap.size(), foregroundAspectRadio, Qt::SmoothTransformation);//VOXOX CHANGE by Rolando - 2009.06.27 
	foregroundPixmapSize = newForegroundPixmap.size();

	switch(horizontalAlignment){

		case Qt::AlignLeft:
			x = 0;
		break;

		case Qt::AlignRight:
			x = backgroundPixmap.width() - newForegroundPixmap.width();
			if(x < 0){
				x = 0;	
			}
		break;

		case Qt::AlignHCenter:
			x = backgroundPixmap.width() - newForegroundPixmap.width();
			if(x < 0){
				x = 0;	
			}
			x = x/2;
		break;

		case Qt::AlignJustify:
			x = 0;
			newForegroundPixmap = foregroundPixmap.scaled(backgroundPixmap.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
		break;

		default:;//No Qt::Alignment allowed
		
	}

	switch(verticalAlignment){
		case Qt::AlignTop:
			y = 0;
		break;

		case Qt::AlignBottom:
			y = backgroundPixmap.height() - newForegroundPixmap.height();
			if(y < 0){
				y = 0;	
			}
		break;

		case Qt::AlignVCenter:
			y = backgroundPixmap.height() - newForegroundPixmap.height();
			if(y < 0){
				y = 0;	
			}
			y = y/2;
		break;

		default:;//No Qt::Alignment allowed
	}

	foregroundPixmapSize = foregroundPixmap.size();
	backgroundPixmapSize = backgroundPixmap.size();
	

	if (!newForegroundPixmap.isNull()) {
		QPainter painter(&backgroundPixmap);
		painter.drawPixmap(x, y, newForegroundPixmap);
		painter.end();
	}

	return backgroundPixmap;
}

//VOXOX CHANGE for VoxOx by Rolando 01-09-09
/*
* mergeFromPixmap: merges two pixmaps on one and returns this one, sets foreground pixmap's position according  offset point's parameter
* Parameters: QPixmap foregroundPixmap,QPixmap backgroundPixmap, QPoint offset
* returns: QPixmap
*/
QPixmap PixmapMerging::mergeFromPixmap(QPixmap foregroundPixmap,QPixmap backgroundPixmap, QPoint offset){

	if (!foregroundPixmap.isNull()) {
		QPainter painter(&backgroundPixmap);
		
		if(foregroundPixmap.width() + offset.x() > backgroundPixmap.width() ){//checks if foreground pixmap and offset point fits on background pixmap
			foregroundPixmap = foregroundPixmap.scaledToWidth (backgroundPixmap.width() + offset.x(), Qt::SmoothTransformation );
		}
		
		if(foregroundPixmap.height() + offset.y() > backgroundPixmap.height() ){//checks if foreground pixmap and offset point fits on background pixmap
			foregroundPixmap = foregroundPixmap.scaledToHeight (backgroundPixmap.height() + offset.y(), Qt::SmoothTransformation );
		}
		
		painter.drawPixmap(offset, foregroundPixmap);
		painter.end();
	}

	return backgroundPixmap;
}

//VOXOX CHANGE Rolando 03-25-09
QPixmap PixmapMerging::concatenatePixmaps(QPixmap leftPixmap,QPixmap rightPixmap, QSize finalPixmapSize, QPoint offsetRightPixmap, QPoint offsetLeftPixmap){
	QPixmap finalPixmap(finalPixmapSize);
	finalPixmap.fill(Qt::transparent);

	if (!leftPixmap.isNull() && !rightPixmap.isNull()) {
		QPainter painter(&finalPixmap);

		if(offsetRightPixmap.x() < finalPixmap.width() && offsetRightPixmap.y() <  finalPixmap.height() && offsetLeftPixmap.x() < finalPixmap.width() && offsetLeftPixmap.y() < finalPixmap.height()){

			if(leftPixmap.width() + offsetLeftPixmap.x() >= finalPixmap.width()){				
				painter.drawPixmap(offsetLeftPixmap,
					leftPixmap.scaled(QSize(finalPixmap.width() - offsetLeftPixmap.x(), finalPixmap.height() - offsetLeftPixmap.y()), Qt::KeepAspectRatio, Qt::SmoothTransformation)
					);
				painter.end();
				return finalPixmap;
			}
			else{
				if(leftPixmap.width() + offsetLeftPixmap.x() + rightPixmap.width() + offsetRightPixmap.x() > finalPixmap.width() ){//checks if leftPixmap, rightPixmap and offsetLeftPixmap.x() fits on finalPixmap pixmap
					int newRightPixmapWidth = finalPixmap.width() - offsetLeftPixmap.x() - leftPixmap.width() - offsetRightPixmap.x();
					if(newRightPixmapWidth > 0){
						rightPixmap = rightPixmap.scaledToWidth (newRightPixmapWidth, Qt::SmoothTransformation );
					}
					else{
						rightPixmap = QPixmap();
					}
					
				}
				
				if(leftPixmap.height() + offsetLeftPixmap.y() > finalPixmap.height() ){//checks if leftPixmap pixmap fits on finalPixmap pixmap
					leftPixmap = leftPixmap.scaledToHeight (finalPixmap.height() - offsetLeftPixmap.y(), Qt::SmoothTransformation );
				}

				if(rightPixmap.height() + offsetRightPixmap.y()> finalPixmap.height() ){//checks if leftPixmap pixmap fits on finalPixmap pixmap
					rightPixmap = rightPixmap.scaledToHeight (finalPixmap.height() - offsetRightPixmap.y(), Qt::SmoothTransformation );
				}

				painter.drawPixmap(offsetLeftPixmap.x(),offsetLeftPixmap.y(), leftPixmap);
				painter.drawPixmap(offsetRightPixmap.x(), offsetRightPixmap.y(), rightPixmap);
			}

			painter.end();	


		}	

		
	}

	return finalPixmap;

}

//VOXOX CHANGE Rolando 03-25-09
QPixmap PixmapMerging::concatenatePixmaps(QPixmap leftPixmap,QPixmap rightPixmap, QSize finalPixmapSize){
	QPixmap finalPixmap(finalPixmapSize);
	finalPixmap.fill(Qt::transparent);

	int separationX = 5;

	if (!leftPixmap.isNull() && !rightPixmap.isNull()) {
		QPainter painter(&finalPixmap);		

		if(leftPixmap.width() + separationX >= finalPixmap.width()){				
			painter.drawPixmap(0, (finalPixmap.height() - leftPixmap.height())/2,
				leftPixmap.scaled(QSize(finalPixmap.width() - separationX, finalPixmap.height()), Qt::KeepAspectRatio, Qt::SmoothTransformation)
				);
			painter.end();
			return finalPixmap;
		}
		else{
			if(leftPixmap.width() + rightPixmap.width() + separationX > finalPixmap.width() ){//checks if leftPixmap, rightPixmap fits on finalPixmap pixmap
				int newRightPixmapWidth = finalPixmap.width() - leftPixmap.width() - separationX;
				if(newRightPixmapWidth > 0){
					rightPixmap = rightPixmap.scaledToWidth (newRightPixmapWidth, Qt::SmoothTransformation );
				}
				else{
					rightPixmap = QPixmap();
				}
				
			}
			
			if(leftPixmap.height() > finalPixmap.height() ){//checks if leftPixmap pixmap fits on finalPixmap pixmap
				leftPixmap = leftPixmap.scaledToHeight (finalPixmap.height(), Qt::SmoothTransformation );
			}

			if(rightPixmap.height() > finalPixmap.height() ){//checks if leftPixmap pixmap fits on finalPixmap pixmap
				rightPixmap = rightPixmap.scaledToHeight (finalPixmap.height(), Qt::SmoothTransformation );
			}

			painter.drawPixmap(0,(finalPixmap.height() - leftPixmap.height())/2, leftPixmap);
			painter.drawPixmap(leftPixmap.width() + separationX, (finalPixmap.height() - rightPixmap.height())/2, rightPixmap);
		}

		painter.end();	
		
	}

	return finalPixmap;

}