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

#include <cutil/global.h>

#ifdef OS_WINDOWS
	#include <winsock2.h>
	#include <windows.h>
#endif

#include <stdlib.h>
//#ifndef CC_MSVC8
//extern "C" {
//#endif
/*#include "glib.h"
#include "libpurple/account.h"
#include "libpurple/blist.h"
#include "libpurple/connection.h"
#include "libpurple/conversation.h"
#include "libpurple/core.h"
#include "libpurple/eventloop.h"
#include "libpurple/internal.h"
#include "libpurple/privacy.h"
#include "libpurple/util.h"*/
#ifdef OS_WINDOWS
//#include "libpurple/win32/win32dep.h"
#endif
#include "libpurple/purple.h"
//#ifndef CC_MSVC8
//}
//#endif

#include "PurpleIMFactory.h"
#include "PurpleAccountMngr.h"
#include "PurpleChatMngr.h"
#include "PurpleConnectMngr.h"
#include "PurpleContactListMngr.h"
#include "PurplePresenceMngr.h"

#include <util/File.h>
#define LOGGER_COMPONENT "Purple"
#include <util/Logger.h>
#include <util/Path.h>

extern PurpleConversationUiOps chat_wg_ops;
extern PurpleBlistUiOps blist_wg_ops;
extern PurpleBlistUiOps null_blist_wg_ops;
extern PurpleConnectionUiOps conn_wg_ops;
extern PurpleConnectionUiOps null_conn_wg_ops;
extern PurpleAccountUiOps acc_wg_ops;
extern PurpleAccountUiOps null_acc_wg_ops;
extern PurplePrivacyUiOps privacy_wg_ops;

/* Static vars */
static GMainLoop * gMainLoop = NULL;


/* ********************* PURPLE CALLBACK ********************* */
#define PURPLE_WG_READ_COND  (G_IO_IN | G_IO_HUP | G_IO_ERR)
#define PURPLE_WG_WRITE_COND (G_IO_OUT | G_IO_HUP | G_IO_ERR | G_IO_NVAL)

extern "C" GIOChannel *wpurple_g_io_channel_win32_new_socket(int socket);

typedef struct _PurpleWgIOClosure {
	PurpleInputFunction function;
	guint result;
	gpointer data;
} PurpleWgIOClosure;

static void purple_wg_io_destroy(gpointer data) {
	g_free(data);
}

static gboolean purple_wg_io_invoke(GIOChannel * source, GIOCondition condition, gpointer data) {

	PurpleInputCondition purple_cond = (PurpleInputCondition) 0;
	if (condition & PURPLE_WG_READ_COND) {
		purple_cond = (PurpleInputCondition)(purple_cond|PURPLE_INPUT_READ);
	}
	if (condition & PURPLE_WG_WRITE_COND) {
		purple_cond = (PurpleInputCondition)(purple_cond|PURPLE_INPUT_WRITE);
	}

#ifdef OS_WINDOWS
	if (!purple_cond) {
		return TRUE;
	}
#endif /* OS_WINDOWS */

	PurpleWgIOClosure * closure = (PurpleWgIOClosure *) data;

	closure->function(closure->data, g_io_channel_unix_get_fd(source), purple_cond);

	return TRUE;
}

static guint purple_wg_input_add(gint fd, PurpleInputCondition condition,
	PurpleInputFunction function, gpointer data) {

	PurpleWgIOClosure * closure = g_new0(PurpleWgIOClosure, 1);

	closure->function = function;
	closure->data = data;

	GIOCondition cond = (GIOCondition) 0;
	if (condition & PURPLE_INPUT_READ) {
		cond = (GIOCondition)(cond|PURPLE_WG_READ_COND);
	}
	if (condition & PURPLE_INPUT_WRITE) {
		cond = (GIOCondition)(cond|PURPLE_WG_WRITE_COND);
	}

	GIOChannel * channel;
#ifdef OS_WINDOWS
	channel = wpurple_g_io_channel_win32_new_socket(fd);
#else
	channel = g_io_channel_unix_new(fd);
#endif
	closure->result = g_io_add_watch_full(channel, G_PRIORITY_DEFAULT, cond,
		purple_wg_io_invoke, closure, purple_wg_io_destroy);

	g_io_channel_unref(channel);
	return closure->result;
}

static void* purple_wg_request_action(const char *title, const char *primary,
		const char *secondary, int default_action,
		PurpleAccount *account, const char *who,
		PurpleConversation *conv, void *user_data, size_t action_count,
		va_list actions) {
	const char *text = va_arg(actions, const char *);
	PurpleRequestActionCb callback = va_arg(actions, PurpleRequestActionCb);

	LOG_WARN("requested action: %s: %s, %s. Default action: %s", title, primary, secondary, text);
	callback(user_data, 0);
        return NULL;
}
gpointer PurpleMainEventLoop(gpointer data) {
	if (gMainLoop) {
		LOG_FATAL("gMainLoop already created");
	}
	gMainLoop = g_main_loop_new(NULL, FALSE);
	LOG_DEBUG("Starting gMainLoop");

	g_main_loop_run(gMainLoop);
	g_main_loop_unref(gMainLoop);
	LOG_DEBUG("gMainLoop stopped");
	gMainLoop = NULL;
	g_thread_exit(NULL);
	return NULL;
}


/* ******************************************************* */

static PurpleCoreUiOps core_wg_ops = {
	NULL,
	NULL,
	PurpleIMFactory::PurpleSetCallbacks,
	PurpleIMFactory::PurpleQuitCallback,
};

static PurpleEventLoopUiOps eventloop_wg_ops = {
	g_timeout_add,
	g_source_remove,
	purple_wg_input_add,
	g_source_remove
};

static PurpleRequestUiOps request_wg_ops = {
	NULL,
	NULL,
	purple_wg_request_action,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};

bool PurpleIMFactory::equals(const IMAccount & imAccount, std::string login, EnumIMProtocol::IMProtocol protocol) {
	bool result = false;
	if(!imAccount.empty()){//VOXOX - CJC - 2009.07.03 
		result = ((imAccount.getLogin() == login) && (imAccount.getProtocol() == protocol));
	}

	return result;
}

PurpleIMFactory::PurpleIMFactory() {
	AccountMngr = PurpleAccountMngr::getInstance();
	ConnectMngr = PurpleConnectMngr::getInstance();
	PresenceMngr = PurplePresenceMngr::getInstance();
	ChatMngr = PurpleChatMngr::getInstance();
	ContactListMngr = PurpleContactListMngr::getInstance();

	if (!g_thread_supported()) {
		g_thread_init(NULL);
	}
	g_thread_create(PurpleMainEventLoop, NULL, FALSE, NULL);
}

PurpleIMFactory::~PurpleIMFactory() {
	if (gMainLoop) {
		LOG_DEBUG("Stopping gMainLoop");
		g_main_loop_quit(gMainLoop);
	} else {
		LOG_ERROR("No gMainLoop created");
	}
}

void PurpleIMFactory::PurpleSetCallbacks() {
	purple_accounts_set_ui_ops(&acc_wg_ops);
	purple_blist_set_ui_ops(&blist_wg_ops);
	purple_privacy_set_ui_ops(&privacy_wg_ops);
	purple_connections_set_ui_ops(&conn_wg_ops);
}

void PurpleIMFactory::PurpleWrapperInit() {
	AccountMngr->Init();
	ConnectMngr->Init();
	ContactListMngr->Init();
	ChatMngr->Init();
	PresenceMngr->Init();
}

void PurpleIMFactory::PurpleIMInit(const std::string & pathToProfile) {

	char * home_dir = g_build_filename(pathToProfile.c_str(), "purple", NULL);
	
	// Remove Purple config directory
	File file(home_dir);
	file.remove();

	File::createPath(home_dir + File::getPathSeparator());
	purple_util_set_user_dir(home_dir);

	if(getenv("OW_PURPLE_DEBUG")) {
		purple_debug_set_enabled(TRUE);
	}
	purple_core_set_ui_ops(&core_wg_ops);
	purple_eventloop_set_ui_ops(&eventloop_wg_ops);

	char * search_path = g_build_filename(Path::getApplicationDirPath().c_str(), "plugins", NULL);
	purple_plugins_add_search_path(search_path);
	purple_plugins_add_search_path("plugins");
	g_free(search_path);
	purple_request_set_ui_ops(&request_wg_ops);

	if (!purple_core_init("Wengo PURPLE")) {
		LOG_WARN("Initialization of the Purple core failed\n");
	}
}

void PurpleIMFactory::init(const std::string & pathToProfile) {
	PurpleIMInit(pathToProfile);
	PurpleWrapperInit();
}


void PurpleIMFactory::PurpleQuitCallback() {
	LOG_DEBUG("");
	// Don't stop gMainLoop here: this is called when the user logoff, but we
	// want to keep the loop running for the whole time the application is
	// running.
}

void PurpleIMFactory::terminate() {
	if (AccountMngr) {
		AccountMngr->reset();
	}
	
	purple_connections_set_ui_ops(&null_conn_wg_ops);
	purple_accounts_set_ui_ops(&null_acc_wg_ops);
	purple_blist_set_ui_ops(&null_blist_wg_ops);

	purple_core_quit();
}

IMConnect * PurpleIMFactory::createIMConnect(IMAccount &account) {
	return ConnectMngr->AddIMConnect(account);
}

IMChat * PurpleIMFactory::createIMChat(IMAccount &account) {
	return ChatMngr->AddIMChat(account);
}

IMPresence * PurpleIMFactory::createIMPresence(IMAccount &account) {
	return PresenceMngr->AddIMPresence(account);
}

IMContactList * PurpleIMFactory::createIMContactList(IMAccount &account) {
	return ContactListMngr->AddIMContactList(account);
}

void PurpleIMFactory::removeIMAccount(IMAccount imAccount) {
	AccountMngr->RemoveIMAccount(imAccount);
}

//VOXOX CHANGE CJC ADD FRIEND FROM MYSPACE SERVER
void PurpleIMFactory::importContactsFromMySpaceServer(IMAccount imAccount) {
	AccountMngr->importContactsFromMySpaceServer(imAccount);
}

void PurpleIMFactory::imAccountUpdated(IMAccount imAccount) {
	AccountMngr->UpdateIMAccount(imAccount);
}
