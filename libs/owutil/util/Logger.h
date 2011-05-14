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

#ifndef OWLOGGER_H
#define OWLOGGER_H

#include <util/owutildll.h>
#include <util/NonCopyable.h>
#include <util/String.h>
#include <util/Event.h>

#include <cutil/global.h>

#include <boost/thread/mutex.hpp>

#include <fstream>

/*
__FILE__ : file source name
__LINE__ : line number inside the source file
__DATE__ : compilation date
__TIME__ : compilation time
__FUNCTION__ : function name
__PRETTY_FUNCTION__ : function name
*/

#ifdef CC_GCC
	#define __FUNCTION__ __PRETTY_FUNCTION__
#endif

#ifndef LOGGER_COMPONENT
	#define LOGGER_COMPONENT "Common"
#endif

/** Macros for the Logger class. */
#define LOG_DEBUG Logger::getInstance()->getHelper(LOGGER_COMPONENT, Logger::Debug, __FUNCTION__, NULL, 0)
#define LOG_INFO  Logger::getInstance()->getHelper(LOGGER_COMPONENT, Logger::Info,  __FUNCTION__, NULL, 0)
#define LOG_WARN  Logger::getInstance()->getHelper(LOGGER_COMPONENT, Logger::Warn,  __FUNCTION__, NULL, 0)
#define LOG_ERROR Logger::getInstance()->getHelper(LOGGER_COMPONENT, Logger::Error, __FUNCTION__, NULL, 0)
#define LOG_FATAL Logger::getInstance()->getHelper(LOGGER_COMPONENT, Logger::Fatal, __FUNCTION__, __FILE__, __LINE__)

/**
 * Logger class.
 *
 * Most users will use the LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR and
 * LOG_FATAL macros.
 *
 * This logging system supports components.
 * By default, all log messages belong to the "Common" component.
 *
 * To use a different component, define the LOGGER_COMPONENT macro before
 * including Logger.h. Like this:
 *
 * <pre>
 * #define LOGGER_COMPONENT "MyComponent"
 * #include <util/Logger.h>
 * </pre>
 *
 * @see org.apache.log4j
 * @see java.util.logging.Logger
 * @see log4cpp
 * @see http://developers.sun.com/solaris/articles/logging.html
 * @see http://logging.apache.org/log4j/docs/manual.html
 * @author Tanguy Krotoff
 * @author Aurelien Gateau
 */
class OWUTIL_API Logger : NonCopyable {
public:
	enum Level {
		Debug,
		Info,
		Warn,
		Error,
		Fatal
	};

	/**
	 * This class is a functor, used by the LOG_* macros to provide
	 * cross-platform support for variadic arguments. It is necessary because
	 * variadic arguments for macros are not supported by all compilers.
	 * (VS 2003 for example)
	 */
	class OWUTIL_API Helper {
	public:
		 Helper(const char* component, Level, const char* className, const char* fileName, int line);
		 void operator()(const std::string& message);
		 void operator()(const char* format, ...);
	
	private:
		const char* _component;
		Level _level;
		const char* _className;
		const char* _fileName;
		int _line;
	};

#ifdef LOGGER_ENABLE_EVENT
	/**
	 * A message has been added to the logging system.
	 *
	 * @param message message added to the logging system
	 */
	Event<void (const std::string & message)> messageAddedEvent;
#endif
	/**
	 * Sets the log filename. This should be called *before* any call to a
	 * log method. If you don't want to log to a file, call this method with
	 * an empty string as an argument.
	 */
	 void setLogFileName(const std::string& logFileName);

	 std::string getLogFileName() const;

	/** Singleton. */
	static  Logger* getInstance();

	 ~Logger();

	 Helper getHelper(const char* component, Logger::Level, const char* className, const char* filename, int line);

	 void debug(const std::string & component, const std::string & className, const std::string & message);

	 void info(const std::string & component, const std::string & className, const std::string & message);

	 void warn(const std::string & component, const std::string & className, const std::string & message);

	 void error(const std::string & component, const std::string & className, const std::string & message);

	 void fatal(const std::string & component, const std::string & className, const std::string & message, const char* filename, int line);

	/**
	 * Flushes the logger.
	 */
	 void flush();

	 void log(const std::string & component, Level level, const std::string & className, const std::string & message,
		const char* filename = NULL, int line = 0);

private:

	Logger();

	enum LogFileState {
		LogFileState_Unset,    /** setLogFileName() has not been called yet */
		LogFileState_Disabled, /** setLogFileName() has been called with an empty string */
		LogFileState_Defined,  /** setLogFileName() has been called, but the file has not been created yet */
		LogFileState_Created   /** setLogFileName() has been called and the file has been created */
	};
	LogFileState _logFileState;

	std::string _logFileName;
	int			_logLineNumber;		//VOXOX - JRT - 2009.07.22 

	std::ofstream _file;

	/** Ensures Logger is thread-safe. */
	mutable boost::mutex _mutex;
};

#endif	//OWLOGGER_H
