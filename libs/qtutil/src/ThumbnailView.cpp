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
#include "qtutil/ThumbnailView.h"

#include <QtGui/QApplication>
#include <QtGui/QHelpEvent>
#include <QtGui/QPainter>
#include <QtGui/QPainterPath>
#include <QtGui/QScrollBar>
#include <QtGui/QToolTip>

/** Space between the item outer rect and the content */
const int ITEM_MARGIN = 11;

const int DEFAULT_COLUMN_COUNT = 4;
const int DEFAULT_ROW_COUNT = 3;

const int SELECTION_CORNER_RADIUS = 10;
const int SELECTION_BORDER_SIZE = 2;
const int SELECTION_ALPHA1 = 32;
const int SELECTION_ALPHA2 = 128;


static QPainterPath createRoundRectPainterPath(const QSize& size, int radius) {
	int width = size.width();
	int height = size.height();
	QRect rect(0, 0, width, height);

	QPainterPath roundRectPath;
	roundRectPath.moveTo(radius, 0);
	roundRectPath.arcTo(0, 0, radius, radius, 90.0, 90.0);

	roundRectPath.lineTo(0, height - radius - 1);

	roundRectPath.arcTo(0, height - radius - 1, radius, radius, 180.0, 90.0);

	roundRectPath.lineTo(width - radius - 1, height - 1);

	roundRectPath.arcTo(width - radius - 1, height - radius -1 , radius, radius, 270.0, 90.0);

	roundRectPath.lineTo(width - 1, radius);

	roundRectPath.arcTo(width - radius - 1, 0, radius, radius, 0.0, 90.0);

	roundRectPath.closeSubpath();

	return roundRectPath;
}



/**
 * An ItemDelegate which generates thumbnails for images. It also makes sure
 * all items are of the same size.
 */
class PreviewItemDelegate : public QAbstractItemDelegate {
public:
	PreviewItemDelegate(ThumbnailView* view)
	: QAbstractItemDelegate(view)
	, _view(view)
	{}


	virtual QSize sizeHint( const QStyleOptionViewItem & /*option*/, const QModelIndex & /*index*/ ) const {
		return QSize( _view->itemWidth(), _view->itemHeight() );
	}


	virtual bool eventFilter(QObject* object, QEvent* event) {
		if (event->type() == QEvent::ToolTip) {
			QAbstractItemView* view = static_cast<QAbstractItemView*>(object->parent());
			QHelpEvent* helpEvent = static_cast<QHelpEvent*>(event);
			showToolTip(view, helpEvent);
			return true;
		}
		return false;
	}


	virtual void paint( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const {
		QVariant value = index.data(Qt::DecorationRole);
		QPixmap thumbnail = qvariant_cast<QPixmap>(value);
		QRect rect = option.rect;

#ifdef DEBUG_RECT
		painter->setPen(Qt::red);
		painter->setBrush(Qt::NoBrush);
		painter->drawRect(rect);
#endif

		// Crop text
		QString fullText = index.data(Qt::DisplayRole).toString();
		QString text;
		QMap<QString, QString>::const_iterator it = _elidedTextMap.find(fullText);
		if (it == _elidedTextMap.constEnd()) {
			text = elidedText(option.fontMetrics, rect.width() - 2*ITEM_MARGIN, Qt::ElideRight, fullText);
			_elidedTextMap[fullText] = text;
		} else {
			text = it.value();
		}

		int textWidth = option.fontMetrics.width(text);

		// Select color group
		QPalette::ColorGroup cg;

		if ( (option.state & QStyle::State_Enabled) && (option.state & QStyle::State_Active) ) {
			cg = QPalette::Normal;
		} else if ( (option.state & QStyle::State_Enabled)) {
			cg = QPalette::Inactive;
		} else {
			cg = QPalette::Disabled;
		}

		// Draw selection
		if (option.state & QStyle::State_Selected) {
			int width = _view->itemWidth();
			QRect selectionRect(
				rect.left() + (rect.width() - width) / 2,
				rect.top(),
				width,
				rect.height());
			QColor borderColor = option.palette.color(cg, QPalette::Highlight);
			QColor fillColor = borderColor;
			fillColor.setAlpha(SELECTION_ALPHA1);
			drawSelectionRect(painter, selectionRect, borderColor, fillColor);
		}

		// Draw thumbnail
		painter->drawPixmap(
			rect.left() + (rect.width() - thumbnail.width())/2,
			rect.top() + (_view->thumbnailSize() - thumbnail.height())/2 + ITEM_MARGIN,
			thumbnail);

		// Draw text
		painter->setPen(option.palette.color(cg, QPalette::Text));

		painter->drawText(
			rect.left() + (rect.width() - textWidth) / 2,
			rect.top() + ITEM_MARGIN + _view->thumbnailSize() + ITEM_MARGIN + option.fontMetrics.ascent(),
			text);
	}


private:
	void drawSelectionRect(QPainter * painter, QRect rect, const QColor & borderColor, const QColor & fillColor) const {

		painter->setRenderHint(QPainter::Antialiasing);

		rect.adjust(SELECTION_BORDER_SIZE, SELECTION_BORDER_SIZE, 0, 0);

		QPainterPath roundRectPath = createRoundRectPainterPath(rect.size(), SELECTION_CORNER_RADIUS);

		QLinearGradient gradient(0, 0, 0, rect.height());
		gradient.setColorAt(0.0, fillColor);
		QColor fillColor2 = fillColor;
		fillColor2.setAlpha(SELECTION_ALPHA2);
		gradient.setColorAt(1.0, fillColor2);
		painter->setBrush(gradient);
		painter->setPen(QPen(borderColor, SELECTION_BORDER_SIZE));

		painter->translate(rect.left(), rect.top());
		painter->drawPath(roundRectPath);
		painter->translate(-rect.left(), -rect.top());
	}

	/**
	 * Show a tooltip only if the item has been elided.
	 * This function places the tooltip over the item text.
	 */
	void showToolTip(QAbstractItemView* view, QHelpEvent* helpEvent) {
		QModelIndex index = view->indexAt(helpEvent->pos());
		if (!index.isValid()) {
			return;
		}

		QString fullText = index.data().toString();
		QMap<QString, QString>::const_iterator it = _elidedTextMap.find(fullText);
		if (it == _elidedTextMap.constEnd()) {
			return;
		}
		QString elidedText = it.value();
		if (elidedText.length() == fullText.length()) {
			// text and tooltip are the same, don't show tooltip
			fullText = QString::null;
		}
		QRect rect = view->visualRect(index);
		QPoint pos(rect.left() + ITEM_MARGIN, rect.top() + _view->thumbnailSize() + ITEM_MARGIN);
		QToolTip::showText(view->mapToGlobal(pos), fullText, view);
		return;
	}


	/**
	 * Maps full text to elided text.
	 */
	mutable QMap<QString, QString> _elidedTextMap;

	ThumbnailView* _view;
};


ThumbnailView::ThumbnailView(QWidget* parent)
: QListView(parent) {
	PreviewItemDelegate* delegate = new PreviewItemDelegate(this);
	setItemDelegate(delegate);
	viewport()->installEventFilter(delegate);

#if QT_VERSION >= 0x040200
	setVerticalScrollMode(ScrollPerPixel);
	setHorizontalScrollMode(ScrollPerPixel);
#endif
}

QSize ThumbnailView::sizeHint() const {
	int viewBorderSize = frameWidth();
	int scrollBarSize = QApplication::style()->pixelMetric(QStyle::PM_ScrollBarExtent);

	QSize hint;

	// MacOS X wants this amount of extra pixels for width, otherwise it won't
	// show DEFAULT_COLUMN_COUNT items
	static const int EXTRA_DELTA = 5;

	// Adjust width to view DEFAULT_COLUMN_COUNT columns
	hint.rwidth() = itemWidth() * DEFAULT_COLUMN_COUNT
		+ spacing() * (DEFAULT_COLUMN_COUNT * 2)
		+ viewBorderSize * 2
		+ scrollBarSize + EXTRA_DELTA;

	// Adjust height to view DEFAULT_ROW_COUNT rows
	hint.rheight() = itemHeight() * DEFAULT_ROW_COUNT
		+ spacing() * (DEFAULT_ROW_COUNT + 1)
		+ viewBorderSize * 2;

	return hint;
}

void ThumbnailView::setThumbnailSize(int value) {
	_thumbnailSize = value;
}

int ThumbnailView::thumbnailSize() const {
	return _thumbnailSize;
}

int ThumbnailView::itemWidth() const {
	return int(_thumbnailSize * 1.4);
}

int ThumbnailView::itemHeight() const {
	return _thumbnailSize + fontMetrics().height() + 3*ITEM_MARGIN;
}
