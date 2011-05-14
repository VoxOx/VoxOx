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
#include <system/Processes.h>

#include <model/WengoPhone.h>
#include <model/config/CommandLineParser.h>
#include <model/config/Config.h>
#include <model/config/ConfigImporter.h>
#include <model/config/ConfigManagerFileStorage.h>
#include <model/config/ConfigManager.h>
#include <model/config/StartupSettingListener.h>
#include <model/classic/ClassicExterminator.h>
#include <model/commandserver/CommandClient.h>


#include <control/CWengoPhone.h>

#ifdef GTKINTERFACE
	#include <presentation/gtk/GtkFactory.h>
#else
	#include <presentation/qt/QtFactory.h>
	#include <presentation/qt/QtLanguage.h>
#endif

#include <imwrapper/IMWrapperFactory.h>
#include <sipwrapper/SipWrapperFactory.h>
#include <sipwrapper/SipWrapper.h>

#include <WengoPhoneBuildId.h>

#ifdef PHAPIWRAPPER
	#include <PhApiFactory.h>
#elif defined(SIPXWRAPPER)
	#include <SipXFactory.h>
	#include <NullIMFactory.h>
#elif defined (MULTIIMWRAPPER)
	#include <PhApiFactory.h>
	#include <multiim/MultiIMFactory.h>
	#include <PurpleIMFactory.h>
#else
	#include <NullSipFactory.h>
	#include <NullIMFactory.h>
#endif

#include <util/File.h>
#include <util/Logger.h>
#include <util/Path.h>
#include <util/Platform.h>			//VOXOX - JRT - 2009.12.18 
#include <util/SafeDelete.h>
#include <util/WebBrowser.h>

#include <system/RegisterProtocol.h>

#include <qtutil/FSResourceFileEngineHandler.h>

#include <cutil/global.h>

#if defined(ENABLE_CRASHREPORT)
#if defined(CC_MSVC)
	#include <memorydump/MSVCMemoryDump.h>
#elif defined(CC_MINGW)
	#include <winsock2.h>
#elif defined(OS_LINUX)
	#include <memorydump/UNIXMemoryDump.h>
#elif defined(OS_MACOSX)
	#include <memorydump/MACMemoryDump.h>
#endif
#endif // ENABLE_CRASHREPORT

#if defined(OS_MACOSX)
#include <SmartCrashReportsInstall.h>
#endif

#if defined(OS_MACOSX) || defined(OS_LINUX)
	#include <signal.h>
	#include <sys/wait.h>
#endif

#include <QtGui/QtGui>

#include <sstream>

#include <buildconfig.h>

#if defined(OS_LINUX)
    #include <X11/Xlib.h>
#endif

static const char* CONFIG_SUBDIR = "/config/";

#ifdef OS_WINDOWS		//VOXOX - JRT - 2009.11.04 
#include <system/WindowsVersion.h>
#endif

#ifdef _WINDOWS	//VOXOX - JRT - 2009.04.09 - For memory leak detection.
#ifdef _DEBUG
//#include "ThreadMonitor.h"
//CThreadMonitor gThreadMonitor;
int AllocHook( int  nAllocType,  void * /*pUserData*/,  size_t nSize, 
			   int nBlockType,   long nRequestNumber, 
			   const unsigned char * /*strFileName*/, int /*nLineNumber*/ );
#endif
#endif


/**
 * Helper class to instantiate and delete factories based on the compilation
 * flags.
 */
struct FactoryHelper {
	FactoryHelper() {
	#ifdef SIPXWRAPPER
		_sipFactory = new SipXFactory();
		_imFactory = new NullIMFactory();

	#elif defined(PHAPIWRAPPER)
		_imFactory = _sipFactory = new PhApiFactory();

	#elif defined(MULTIIMWRAPPER)
		PhApiFactory* phApiFactory = new PhApiFactory();
		_sipFactory = phApiFactory;
		_purpleIMFactory = new PurpleIMFactory();
		_imFactory = new MultiIMFactory(*phApiFactory, *_purpleIMFactory);

	#else
		_sipFactory = new NullSipFactory();
		_imFactory = new NullIMFactory();
	#endif

		SipWrapperFactory::setFactory(_sipFactory);
		IMWrapperFactory::setFactory(_imFactory);
	}

	~FactoryHelper() {
	#ifdef PHAPIWRAPPER
		// In this case _imFactory == _sipFactory, so don't delete it twice
		OWSAFE_DELETE(_imFactory);
	#else
		OWSAFE_DELETE(_sipFactory);
		OWSAFE_DELETE(_imFactory);
	#endif

	#ifdef MULTIIMWRAPPER
		OWSAFE_DELETE(_purpleIMFactory);
	#endif
	}

	IMWrapperFactory * _imFactory;
	SipWrapperFactory * _sipFactory;

#if defined(MULTIIMWRAPPER)
	PurpleIMFactory* _purpleIMFactory;
#endif

};


/**
 * Stub function to make GCC silent.
 *
 * @see http://www-eleves-isia.cma.fr/documentation/BoostDoc/boost_1_29_0/libs/test/doc/minimal.htm
 */
int test_main(int argc, char *argv[]) {
	return 1;
}

static void initLogger(const std::string& userConfigDir) {
	std::string logFileName = userConfigDir + BINARY_NAME + ".log";
	// create directory if it doesn't exist
	if (!File::exists(userConfigDir)) {
		File::createPath(userConfigDir);
	}
	if (!File::isDirectory(userConfigDir)) {
		LOG_FATAL("User configuration dir '" + userConfigDir + "' does not exist or is not a directory");
	}

	Logger::getInstance()->setLogFileName(logFileName);
}

static void initConfig(const CommandLineParser& cmdLineParser) {
	ConfigManagerFileStorage configManagerStorage(ConfigManager::getInstance());

	std::string resourcesDir = cmdLineParser.getResourcesDir();
	if (!File::isDirectory(resourcesDir)) {
		LOG_FATAL("Resources dir '" + resourcesDir + "' does not exist or is not a directory");
	}

	std::string userConfigDir = cmdLineParser.getUserConfigDir();
	configManagerStorage.loadSystemConfig(resourcesDir + CONFIG_SUBDIR);
	Config::setConfigDir(userConfigDir);
	ConfigImporter importer;
	importer.importConfig();
	configManagerStorage.loadUserConfig(userConfigDir);

	Config & config = ConfigManager::getInstance().getCurrentConfig();
	config.setResourcesDir(resourcesDir);
	WebBrowser::setBrowser(config.getLinuxPreferedBrowser());
}

static void registerHyperlinkProtocol(const Config& config, const std::string& executableName) {
	RegisterProtocol registerProtocol(config.getHyperlinkProtocol());
	std::string executableFullName = Path::getApplicationDirPath() + executableName;
	registerProtocol.bind(executableFullName + " -c %1", executableFullName + ",0", config.getCompanyWebSiteUrl());
}

/**
 * Callback additional info for MemoryDump.
 */
std::string getAdditionalInfo() {
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	std::stringstream stream;
	stream
		<< "User: " << config.getProfileLastUsedName() << std::endl
		<< "buildid: " << WengoPhoneBuildId::getBuildId() << std::endl
		<< "revision: " << WengoPhoneBuildId::getSvnRevision() << std::endl;
	return stream.str();
}

#if defined(OS_MACOSX) || defined(OS_LINUX)
static void sigchild_catcher(int sig) {
#ifndef NDEBUG
	// See comment in sigpipe_catcher 
	static char msg[] = "Signal catched: SIGCHLD\n";
	write(2, msg, sizeof(msg));
#endif

	int status;
	pid_t pid;

	do {
		pid = waitpid(-1, &status, WNOHANG);
	} while (pid != 0 && pid != (pid_t)-1);
}

static void sigpipe_catcher(int sig) {
#ifndef NDEBUG
	// Do not use LOG_DEBUG. There is only a limited set of functions you are
	// allowed to call from withing a signal catcher. See signal man page.
	static char msg[] = "Caught signal: SIGPIPE\n";
	write(2, msg, sizeof(msg));
#endif
}
#endif

int main(int argc, char * argv[]) {

#if defined(OS_MACOSX) || defined(OS_LINUX)
	signal(SIGPIPE, sigpipe_catcher);
	signal(SIGCHLD, sigchild_catcher);
#endif

#ifdef _WINDOWS
#ifdef _DEBUG
//	_CrtSetDbgFlag( _CRTDBG_LEAK_CHECK_DF );
	//::_CrtSetAllocHook( AllocHook );
#endif
#endif
	//Init presentation factories before parsing the command line so that Qt or
	//Gtk get a chance to parse their command line options ('-style' for Qt for
	//example)
	PFactory * pFactory = NULL;
#if defined(OS_LINUX)
	XInitThreads();
#endif

#ifdef GTKINTERFACE
	pFactory = new GtkFactory(argc, argv);
#else
	pFactory = new QtFactory(argc, argv);
#endif
        
#if defined(OS_MACOSX)
        QDir dir(QApplication::applicationDirPath());
        dir.cdUp();
        dir.cd("plugins");
	QApplication::setLibraryPaths(QApplication::libraryPaths() + QStringList(dir.absolutePath()));
#endif
        
	PFactory::setFactory(pFactory);

	CommandLineParser cmdLineParser(BINARY_NAME, argc, argv);
	initLogger(cmdLineParser.getUserConfigDir());
	LOG_DEBUG("Started");

#ifdef OS_WINDOWS	//VOXOX - JRT - 2009.11.04 
	LOG_INFO( WindowsVersion::getVersion() + std::string(" - ") + Platform::getPlatformCode() );
#endif

#if defined(OS_MACOSX) || defined(OS_LINUX)
	signal(SIGPIPE, sigpipe_catcher);
#endif

	// Setup factories. These must be initialized before loading config because
	// it's used when importing configuration from the Classic version.
	FactoryHelper factoryHelper;

	initConfig(cmdLineParser);

	Config & config = ConfigManager::getInstance().getCurrentConfig();

	SipWrapper* w = factoryHelper._sipFactory->createSipWrapper();
	
	w->setSipOptions("sip.register_timeout", QString::number(config.getSipRegisterTimeOut()).toStdString());
	w->setSipOptions("sip.publish_timeout", QString::number(config.getSipPublishTimeOut()).toStdString());
	if(config.getSipUseOptions())
		w->setSipOptions("sip.use_options_request", "true");
	else
		w->setSipOptions("sip.use_options_request", "false");

	if(config.getSipP2pPresence())
		w->setSipOptions("sip.p2p_presence", "true");
	else
		w->setSipOptions("sip.p2p_presence", "false");

	if(config.getSipChatWithoutPresence())
		w->setSipOptions("sip.chat.without.presence", "true");
	else
		w->setSipOptions("sip.chat.without.presence", "false");

	if(config.getSipUseTypingState())
		w->setSipOptions("sip.use_typing_state", "true");
	else
		w->setSipOptions("sip.use_typing_state", "false");


	//Remove WengoPhone Classic from startup registry
	ClassicExterminator::killClassicExecutable();

	//Remove WengoPhone Classic from startup registry
	ClassicExterminator::removeClassicFromStartup();

#if defined(ENABLE_CRASHREPORT)
	int revision = atoi( WengoPhoneBuildId::getSvnRevision() );		//VOXOX - JRT - 2009.09.01 
	std::string prefix = String::fromNumber( revision, 4, "0" );

    #if defined(CC_MSVC)
	#ifndef _DEBUG		//VOXOX - JRT - 2009.08.05 - I want normal debugging if in debug build.
	MSVCMemoryDump * memoryDump = new MSVCMemoryDump("VoxOx", WengoPhoneBuildId::getSvnRevision(), prefix.c_str());	//VOXOX - JRT - 2009.07.23 
	// This is needed for CRT to not show dialog for invalid param
	// failures and instead let the code handle it.
	_CrtSetReportMode(_CRT_ASSERT, 0);
    #endif
	#endif

    #if defined(OS_MACOSX)
	MACMemoryDump * memoryDump = new MACMemoryDump("VoxOx", WengoPhoneBuildId::getSvnRevision(), prefix.c_str()); // VOXOX -ASV- 07-28-2009
    #endif

    #if defined(OS_LINUX)
	UNIXMemoryDump * memoryDump = new UNIXMemoryDump("VoxOx", WengoPhoneBuildId::getSvnRevision());
    #endif

    #if defined(CC_MSVC)
	#ifndef _DEBUG		//VOXOX - JRT - 2009.08.05 - I want normal debugging if in debug build.
	memoryDump->setGetAdditionalInfo(getAdditionalInfo);
	#endif
	#else
	memoryDump->setGetAdditionalInfo(getAdditionalInfo);
	#endif

#endif // ENABLE_CRASHREPORT

	// Uncomment to test crash report
	
	/*int* pointer;
	pointer = 0;
	*pointer = 12;*/

// VOXOX -ASV- 07-29-2009: added SmartCrashReports for Mac, this will install the 
// client for the user if it is not located on the user's computer.
#if defined(OS_MACOSX)	
	Boolean authenticationWillBeRequired = FALSE;
	if (UnsanitySCR_CanInstall(&authenticationWillBeRequired))
		UnsanitySCR_Install(authenticationWillBeRequired ? kUnsanitySCR_GlobalInstall : 0);
#endif	
	
	//No 2 qtwengophone at the same time
	if (Processes::isRunning(BINARY_NAME) && !cmdLineParser.isSeveralWengoPhoneAllowed()) {
		const std::string callFromCommandLine = cmdLineParser.getCommand();
		CommandClient client;
		client.connect();
		if (!callFromCommandLine.empty()) {
			client.call(callFromCommandLine);
		} else {
			client.bringToFront();
		}
		return EXIT_SUCCESS;
	}

	// Register protocol used to associate the application in HTML links
	registerHyperlinkProtocol(config, cmdLineParser.getExecutableName());

	// Make the application starts when user logs on computer
	StartupSettingListener settingListener(cmdLineParser.getExecutableName());

	// Init file system resource engine
	FSResourceFileEngineHandler handler(QString::fromStdString(config.getResourcesDir()));

//VOXOX - JRT - 2009.04.11 - debugging
#ifdef _WINDOWS
#ifdef _DEBUG
//	gThreadMonitor.SetInterval( 1000 );	//VOXOX - JRT - 2009.04.09 
//	gThreadMonitor.Start();
#endif
#endif
//End VoxOx

	// CWengoPhone creates PWengoPhone (QtWengoPhone, GtkWengoPhone...)
	// and then starts the model thread. This way the gui is shown as soon as
	// possible
	WengoPhone & wengoPhone = WengoPhone::getInstance();
	wengoPhone.setStartupCall(cmdLineParser.getCommand());
	CWengoPhone cWengoPhone(wengoPhone, cmdLineParser.getRunInBackground());
	pFactory->exec();

	WengoPhone::deleteInstance();
	LOG_DEBUG("Ended");

//	pFactory->cleanup();	//VOXOX - JRT - 2009.04.13 
	
	return EXIT_SUCCESS;
}

//=============================================================================

//VOXOX - JRT - 2009.04.13 - Debug memory leaks
#ifdef _WINDOWS
#ifdef _DEBUG
int AllocHook( int  nAllocType,  void * /*pUserData*/,  size_t nSize, 
			   int nBlockType,   long nRequestNumber, 
			   const unsigned char * /*strFileName*/, int /*nLineNumber*/ )	//No data in FileName/LineNumber
{
	if ( nBlockType == _CRT_BLOCK )
		return( TRUE );

//	UNUSED_ALWAYS(nAllocType);

//	_HOOK_ALLOC	   1;
//	_HOOK_REALLOC  2;
//	_HOOK_FREE     3;

	int xx = 0;

	//First allocation request we get is #1235.  I think all previous allocations are for static objects.
//	if ( nRequestNumber == 301 )
//		xx = 1;
//
//	if ( nRequestNumber >= 489 && nRequestNumber <= 637 )
//		xx = 1;


	if ( nSize == 1 && nRequestNumber > 3365 ) 
		xx = 1;

	if ( nSize == 296 && nRequestNumber > 6030 ) 
		xx = 1;

	if ( nSize == 172 && nRequestNumber > 6222 ) 
		xx = 1;

	if ( nSize == 80 && nRequestNumber > 202000 ) 
		xx = 1;


//	int x = 0;
//	UINT nMinSize = 1000;
/*
	if ( nRequestNumber > 0 && nAllocType != _HOOK_FREE && nSize > nMinSize )
	{
//		wsprintf( g_Buff, _T("WG       - Request# %6d, Type: %d, Size: %6d, File: %s, Line: %4d\n"), nRequestNumber, nAllocType, nSize, strFileName, nLineNumber );
		wsprintf( g_Buff, _T("WG       - Request# %6d, Type: %d, Size: %6d\n"), nRequestNumber, nAllocType, nSize );
		TRACE( g_Buff );
	}
*/

	return TRUE;
}

#endif //_DEBUG
#endif //_WINDOWS
