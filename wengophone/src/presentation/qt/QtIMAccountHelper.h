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
#ifndef QTIMACCOUNTHELPER_H
#define QTIMACCOUNTHELPER_H

#include <imwrapper/IMAccount.h>
#include <imwrapper/IMAccountList.h>

/**
 * This namespace contains code to sort an IMAccountList.
 * @see QtIMProfileWidget
 * @see QtMultiPresenceStatusHelper
 *
 * @author Aurelien Gateau
 */
namespace QtIMAccountHelper {

/**
 * The vector which will contains the sorted list. We must use a vector because
 * one can't run std::sort on a list.
 */
typedef std::vector<const IMAccount*> QtIMAccountPtrVector;

/**
 * Copy the account list to the provided vector
 * @param list the list to copy
 * @param vector a pointer to a QtIMAccountPtrVector
 */
void copyListToPtrVector(const IMAccountList& list, QtIMAccountPtrVector* vector);

/**
 * Returns true if account1 is "less than" account2
 */
bool compareIMAccountPtrs(const IMAccount* account1, const IMAccount* account2);

} // namespace


#endif /* QTIMACCOUNTHELPER_H */