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

#ifndef QTHTTPPROXYLOGIN_H
#define QTHTTPPROXYLOGIN_H

#include <util/NonCopyable.h>

#include <QtCore/QObject>

#include <string>

class QWidget;
class QDialog;
namespace Ui { class HttpProxyLoginWindow; }

/**
 * Window asking for the local HTTP proxy login/password.
 *
 * @author Tanguy Krotoff
 */
class QtHttpProxyLogin : public QObject, NonCopyable {
	Q_OBJECT
public:

	QtHttpProxyLogin(QWidget * parent, const std::string & proxyAddress, unsigned proxyPort);

	QDialog * getWidget() const {
		return _httpProxyLoginWindow;
	}

	std::string getLogin() const;

	std::string getPassword() const;

	std::string getProxyAddress() const;

	unsigned getProxyPort() const;

public Q_SLOTS:

	int show();

private:

	Ui::HttpProxyLoginWindow * _ui;

	QDialog * _httpProxyLoginWindow;
};

#endif	//QTHTTPPROXYLOGIN_H
