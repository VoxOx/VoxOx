/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2007  Wengo
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
#include "QtPresencePixmapHelper.h"

#include <QtGui/QPainter>
#include <QtGui/QPainterPath>
#include <QtGui/QPixmap>

#include <imwrapper/IMAccount.h>
#include <imwrapper/IMAccountList.h>

#include <util/Logger.h>

#include "QtIMAccountHelper.h"


static QPainterPath createArcPath(int centerX, int centerY, int radius, int startAngle, int stopAngle) {
	QPainterPath path;
	path.moveTo(centerX, centerY);
	path.arcTo(centerX - radius, centerY - radius, 2*radius, 2*radius, startAngle, stopAngle - startAngle + 1);
	return path;
}


QPixmap QtPresencePixmapHelper::compose(QPixmap pix1, QPixmap pix2, int startAngle, int stopAngle) {
	QPixmap out(pix1);
	QPainter painter(&out);
	QPainterPath path = createArcPath(_centerX, _centerY, _radius, startAngle, stopAngle);
	painter.setClipPath(path);
	painter.drawPixmap(0, 0, pix2);
	painter.end();
	return out;
}


QtPresencePixmapHelper::QtPresencePixmapHelper(const IMAccountList& imAccountList) {
	// Create a sorted vector of accounts
	QtIMAccountHelper::QtIMAccountPtrVector imAccountPtrVector;
	QtIMAccountHelper::copyListToPtrVector(imAccountList, &imAccountPtrVector);
	std::sort(imAccountPtrVector.begin(), imAccountPtrVector.end(), QtIMAccountHelper::compareIMAccountPtrs);
	////

	QtIMAccountHelper::QtIMAccountPtrVector::const_iterator
		it = imAccountPtrVector.begin(),
		end = imAccountPtrVector.end();

	QList<std::string> list;

	for (; it!=end; ++it) 
	{
//		if(!list.contains((*it)->getUUID()))
		if(!list.contains((*it)->getKey()))	//VOXOX - JRT - 2009.04.24
		{
			EnumPresenceState::PresenceState state;
			if ((*it)->isConnected()) {
				state = (*it)->getPresenceState();
			} else {
				state = EnumPresenceState::PresenceStateOffline;
			}
			_summary.push_back(state);
//			list.append((*it)->getUUID());
			list.append((*it)->getKey());	//VOXOX - JRT - 2009.04.24
		}
	}
}


QPixmap QtPresencePixmapHelper::createPixmap(const QString& tmpl, int centerX, int centerY, int radius) {
	QtPresencePixmapHelper::PresenceSummary::const_reverse_iterator
		it = _summary.rbegin(),
		end = _summary.rend();
	
	_centerX = centerX;
	_centerY = centerY;
	_radius = radius;

	int total = _summary.size();

	int angle = 90 + 360 / total;
	int newAngle;

	QString presenceString = QString::fromStdString( EnumPresenceState::toString(*it) );
	QString name = tmpl.arg(presenceString);
	QPixmap out(name);
	if (out.isNull()) {
		LOG_WARN("Image " + name.toStdString() + " does not exist");
		return QPixmap();
	}

	++it;
	for (; it!=end; ++it) {
		presenceString = QString::fromStdString( EnumPresenceState::toString(*it) );
		QString name = tmpl.arg(presenceString);
		QPixmap pix(name);
		if (pix.isNull()) {
			LOG_WARN("Image " + name.toStdString() + " does not exist");
			continue;
		}

		newAngle = angle + 360 / total;

		out = compose(out, pix, angle, newAngle);

		angle = newAngle;
	}

	return out;
}

