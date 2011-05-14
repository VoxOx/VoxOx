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

#include "stdafx.h"		//VOXOX - JRT - 2009.04.02
#include "ILogin.h"

ILogin::ILogin(QtLoginDialog * qtLoginDialog, CUserProfileHandler & cUserProfileHandler)
	: _cUserProfileHandler(cUserProfileHandler) 
	//, _backPage(QtLoginDialog::LogAccount)//VOXOX CHANGE by Rolando 02-25-09
	, _backPage(QtLoginDialog::AddVoxOxAccount)//VOXOX CHANGE by Rolando 02-25-09
{
	_loginDialog = qtLoginDialog;
}

ILogin::~ILogin() {
}

void ILogin::load(const SipAccount & sipAccount) {
	load(sipAccount.getUserProfileName());
}

void ILogin::setBackPage(QtLoginDialog::LoginDialogPage backPage) {
	_backPage = backPage;
}

void ILogin::goBack() {
	_loginDialog->changePage(_backPage);
}
