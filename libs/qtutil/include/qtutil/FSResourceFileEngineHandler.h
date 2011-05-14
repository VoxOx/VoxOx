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
#ifndef FSRESOURCEFILEENGINEHANDLER_H
#define FSRESOURCEFILEENGINEHANDLER_H

#include <QtCore/QAbstractFileEngineHandler>

/**
 * This QAbstractFileEngineHandler implementation makes it possible to use .qrc
 * files while loading the images directly from the file system.
 *
 * To use it, create an instance of QAbstractFileEngineHandler in the main()
 * function, like this:
 *
 * int main() {
 *   FSResourceFileEngineHandler handler("/path/to/resources");
 * }
 *
 * It is important to create the instance in main() and not as a global static
 * variable to make sure the handler is called *before* Qt resource handler.
 */
class FSResourceFileEngineHandler : public QAbstractFileEngineHandler {
public:
	FSResourceFileEngineHandler(const QString& baseDir);
	virtual QAbstractFileEngine* create(const QString& path) const;

private:
	QString _baseDir;
};
#endif /* FSRESOURCEFILEENGINEHANDLER_H */
