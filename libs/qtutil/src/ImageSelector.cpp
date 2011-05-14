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

#include "qtutil/ImageSelector.h"

#include <QtCore/QDir>
#include <QtCore/QString>

#include <QtGui/QLineEdit>
#include <QtGui/QStandardItemModel>
#include <QtGui/QStyle>
//VOXOX CHANGE for VoxOx by Rolando 01-09-09 added Logger.h
#include <util/Logger.h>
#include <util/SafeDelete.h>

#include <cutil/global.h>

#include <qtutil/SafeConnect.h>
#include <qtutil/DesktopService.h>

#include "QtThumbnailManager.h"
#include "QtThumbnailDirModel.h"

#include "ui_ImageSelector.h"

const int COMBOBOX_ICON_SIZE = 16;

const int THUMBNAIL_SIZE = 64;

/** Blank space between items */
const int ITEM_SPACING = 6;


ImageSelector::ImageSelector(QWidget * parent)
	: QDialog(parent) {
	_ui = new Ui::ImageSelector();
	_ui->setupUi(this);

	_ui->thumbnailListView->ensurePolished();

	// Setup model
	_model = new QtThumbnailDirModel(this);
	_model->setPixmapSize(THUMBNAIL_SIZE);

	// Init thumbnailListView
	_ui->thumbnailListView->setModel(_model);

	_ui->thumbnailListView->setResizeMode(QListView::Adjust);
	_ui->thumbnailListView->setSpacing(ITEM_SPACING);
	_ui->thumbnailListView->setThumbnailSize(THUMBNAIL_SIZE);

	// This is a bit tricky. See showEvent documentation.
	_ui->thumbnailListView->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

	SAFE_CONNECT(_ui->thumbnailListView, SIGNAL(activated(const QModelIndex &)),
		SLOT(slotThumbnailListViewActivated(const QModelIndex &)) );
	SAFE_CONNECT(_ui->thumbnailListView->selectionModel(), SIGNAL(currentChanged(const QModelIndex&, const QModelIndex &)),
		SLOT(updateOkButton()) );

	// Init startDirListView
	_startDirModel = new QStandardItemModel(this);
	_startDirModel->insertColumn(0);
	_ui->startDirListView->setModel(_startDirModel);

	SAFE_CONNECT(_ui->startDirListView->selectionModel(), SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)),
		SLOT(slotStartDirListViewChanged(const QModelIndex&)) );

//VOXOX CHANGE for VoxOx by Rolando 01-09-09 we use QEvent
//VOXOX CHANGE for VoxOx by Rolando 01-09-09, in VoxOx this sentences are eliminated from here...
	// Icons
	#ifndef OS_MACOSX
	// There is usually no icon for parent folder on MacOS X.
	_ui->goUpButton->setIcon(QIcon(":/pics/parent_folder.png"));
	#endif

	_ui->refreshButton->setIcon(QIcon(":/pics/refresh.png"));
//VOXOX CHANGE for VoxOx by Rolando 01-09-09, ...to here.

	// Other connections
	SAFE_CONNECT(_ui->goUpButton, SIGNAL(clicked()), SLOT(goUp()) );
	SAFE_CONNECT(_ui->refreshButton, SIGNAL(clicked()), SLOT(refresh()) );
	SAFE_CONNECT(_ui->dirComboBox, SIGNAL(activated(int)), SLOT(slotDirComboBoxActivated(int)) );
}


ImageSelector::~ImageSelector() {
	OWSAFE_DELETE(_ui);
}

void ImageSelector::showEvent(QShowEvent* event) {
	QDialog::showEvent(event);
	_ui->thumbnailListView->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
}


QString ImageSelector::path() const {
	return _path;
}




void ImageSelector::addStartDirItem(const QString& dir, const QString& name, const QPixmap& pixmap) {
	int row = _startDirModel->rowCount();
	_startDirModel->insertRow(row);
	QModelIndex index = _startDirModel->index(row, 0);
	_startDirModel->setData(index, dir, Qt::UserRole);
	_startDirModel->setData(index, name, Qt::DisplayRole);
	_startDirModel->setData(index, pixmap, Qt::DecorationRole);

	//	Now that we have some data, let's resize the list
	int width = _ui->startDirListView->sizeHintForColumn(0) + 2 * layout()->margin();
	_ui->startDirListView->setFixedWidth(width);
}


void ImageSelector::setCurrentDir(const QString & dir) {
	QFileInfo fileInfo(dir);
	if (!fileInfo.exists()) {
		return;
	}

	if (!fileInfo.isDir()) {
		return;
	}

	_model->setDir(dir);

	updateDirComboBox();
	updateOkButton();

	_ui->goUpButton->setEnabled(!fileInfo.isRoot());
}


void ImageSelector::goUp() {
	QFileInfo fileInfo(_model->dir());
	setCurrentDir(fileInfo.absolutePath());
}


void ImageSelector::refresh() {
	_model->refresh();
}


void ImageSelector::accept() {
	QModelIndex index = _ui->thumbnailListView->selectionModel()->currentIndex();
	if (!index.isValid()) {
		return;
	}

	QFileInfo fileInfo = _model->fileInfo(index);
	_path = fileInfo.absoluteFilePath();

	QDialog::accept();
}


void ImageSelector::updateOkButton() {
	bool enabled = false;
	QModelIndex index = _ui->thumbnailListView->selectionModel()->currentIndex();

	if (index.isValid()) {
		QFileInfo fileInfo = _model->fileInfo(index);
		enabled = ! fileInfo.isDir();
	}
	_ui->okButton->setEnabled(enabled);
}


void ImageSelector::updateDirComboBox() {
	_ui->dirComboBox->clear();
	DesktopService* service = DesktopService::getInstance();
	QFileInfo fileInfo(_model->dir());
	while (true) {
		QString path = fileInfo.absoluteFilePath();
		QIcon icon = service->pixmapForPath(path, COMBOBOX_ICON_SIZE);
		QString name = service->userFriendlyNameForPath(path);

		_ui->dirComboBox->addItem(icon, name, path);

		if (fileInfo.isRoot()) {
			break;
		}
		fileInfo = QFileInfo(fileInfo.absolutePath());
	}
}


void ImageSelector::slotThumbnailListViewActivated(const QModelIndex& current) {
	if (!current.isValid()) {
		return;
	}

	QFileInfo info = _model->fileInfo(current);
	setCurrentDir(info.absoluteFilePath());
}


void ImageSelector::slotStartDirListViewChanged(const QModelIndex& current) {
	if (!current.isValid()) {
		return;
	}

	QString path = current.data(Qt::UserRole).toString();
	setCurrentDir(path);
}


void ImageSelector::slotDirComboBoxActivated(int index) {
	QVariant data = _ui->dirComboBox->itemData(index);
	QString dir = data.toString();
	setCurrentDir(dir);
}
