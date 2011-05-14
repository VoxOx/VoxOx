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


#ifndef IMAGESELECTOR_H
#define IMAGESELECTOR_H

#include <QtGui/QDialog>
#include <QtGui/QImage>

#include <qtutil/PictureFlow.h>

class QModelIndex;
class QStandardItemModel;
class QShowEvent;

class QtThumbnailDirModel;

namespace Ui {
	class ImageSelector;
}

/**
 * A dialog to select an avatar
 *
 * @author Aurélien Gâteau
 */
class ImageSelector : public QDialog {
	Q_OBJECT
public:
	static const int START_DIR_ICON_SIZE = 32;
	ImageSelector(QWidget * parent);
	~ImageSelector();

	QString path() const;

	QImage getCurrentImage(){ return _currentImage;}


	void setDefaultImagePath(QString imageFilename);

	/**
	 * Add an item to the "start dir" side bar.
	 * @param dir : the path to point to
	 * @param name : the item name in the side bar
	 * @param pixmap : the item icon in the side bar
	 */
	void addStartDirItem(const QString& dir, const QString& name, const QPixmap& pixmap);

	virtual void accept();

protected:
	/**
	 * This is a bit tricky: ImageSelector constructor sets the size policy of
	 * our thumbnail view to Minimum,Minimum so that the dialog get resized to
	 * show the view at the view.sizeHint() size. In showEvent() we reset the
	 * size policy of the view back to Preferred,Preferred so that the user can
	 * resize the dialog smaller.
	 */
	virtual void showEvent(QShowEvent*);

public Q_SLOTS:
	void setCurrentDir(const QString & currentDir);

private Q_SLOTS:
	void goUp();
	void refresh();
	void updateOkButton();
	void slotThumbnailListViewActivated(const QModelIndex& current);
	void slotStartDirListViewChanged(const QModelIndex& current);
	void slotDirComboBoxActivated(int index);

private:
	void updateDirComboBox();

	QtThumbnailDirModel* _model;
	QString _path;
	QImage _currentImage;
	Ui::ImageSelector* _ui;
	QStandardItemModel* _startDirModel;
	PictureFlow * _pictureFlow;//VOXOX CHANGE by Rolando - 2009.05.25 
};

#endif /* IMAGESELECTOR_H */
