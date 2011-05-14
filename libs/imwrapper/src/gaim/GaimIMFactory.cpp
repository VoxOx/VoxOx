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

#ifndef CC_MSVC8
extern "C" {
#endif
#include "glib.h"
#include "gaim/account.h"
#include "gaim/blist.h"
#include "gaim/connection.h"
#include "gaim/conversation.h"
#include "gaim/core.h"
#include "gaim/eventloop.h"
#include "gaim/internal.h"
#include "gaim/privacy.h"
#include "gaim/util.h"
#ifdef OS_WINDOWS
#include "gaim/win32dep.h"
#endif
#ifndef CC_MSVC8
}
#endif

#include "GaimIMFactory.h"
#include "GaimAccountMngr.h"
#include "GaimChatMngr.h"
#include "GaimConnectMngr.h"
#include "GaimContactListMngr.h"
#include "GaimPresenceMngr.h"

#include <util/File.h>
#define LOGGER_COMPONENT "Gaim"
#include <util/Logger.h>
#include <util/Path.h>

extern GaimConversationUiOps chat_wg_ops;
extern GaimBlistUiOps blist_wg_ops;
extern GaimBlistUiOps null_blist_wg_ops;
extern GaimConnectionUiOps conn_wg_ops;
extern GaimConnectionUiOps null_conn_wg_ops;
extern GaimAccountUiOps acc_wg_ops;
extern GaimAccountUiOps null_acc_wg_ops;
extern GaimPrivacyUiOps privacy_wg_ops;

/* Static vars */
static GMainLoop * gMainLoop = NULL;


/* ********************* GAIM CALLBACK ********************* */
#define GAIM_WG_READ_COND  (G_IO_IN | G_IO_HUP | G_IO_ERR)
#define GAIM_WG_WRITE_COND (G_IO_OUT | G_IO_HUP | G_IO_ERR | G_IO_NVAL)

typedef struct _GaimWgIOClosure {
	GaimInputFunction function;
	guint result;
	gpointer data;
} GaimWgIOClosure;

static void gaim_wg_io_destroy(gpointer data) {
	g_free(data);
}

static gboolean gaim_wg_io_invoke(GIOChannel * source, GIOCondition condition, gpointer data) {

	GaimInputCondition gaim_cond = (GaimInputCondition) 0;
	if (condition & GAIM_WG_READ_COND) {
		gaim_cond = (GaimInputCondition)(gaim_cond|GAIM_INPUT_READ);
	}
	if (condition & GAIM_WG_WRITE_COND) {
		gaim_cond = (GaimInputCondition)(gaim_cond|GAIM_INPUT_WRITE);
	}

#ifdef OS_WINDOWS
	if (!gaim_cond) {
		return TRUE;
	}
#endif /* OS_WINDOWS */

	GaimWgIOClosure * closure = (GaimWgIOClosure *) data;

	closure->function(closure->data, g_io_channel_unix_get_fd(source), gaim_cond);

	return TRUE;
}

static guint gaim_wg_input_add(gint fd, GaimInputCondition condition,
	GaimInputFunction function, gpointer data) {

	GaimWgIOClosure * closure = g_new0(GaimWgIOClosure, 1);

	closure->function = function;
	closure->data = data;

	GIOCondition cond = (GIOCondition) 0;
	if (condition & GAIM_INPUT_READ) {
		cond = (GIOCondition)(cond|GAIM_WG_READ_COND);
	}
	if (condition & GAIM_INPUT_WRITE) {
		cond = (GIOCondition)(cond|GAIM_WG_WRITE_COND);
	}

	GIOChannel * channel;
#ifdef OS_WINDOWS
	channel = wgaim_g_io_channel_win32_new_socket(fd);
#else
	channel = g_io_channel_unix_new(fd);
#endif
	closure->result = g_io_add_watch_full(channel, G_PRIORITY_DEFAULT, cond,
		gaim_wg_io_invoke, closure, gaim_wg_io_destroy);

	g_io_channel_unref(channel);
	return closure->result;
}

gpointer GaimMainEventLoop(gpointer data) {
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

static GaimCoreUiOps core_wg_ops = {
	NULL,
	NULL,
	GaimIMFactory::GaimSetCallbacks,
	GaimIMFactory::GaimQuitCallback,
};

static GaimEventLoopUiOps eventloop_wg_ops = {
	g_timeout_add,
	(guint (*)(guint))g_source_remove,
	gaim_wg_input_add,
	(guint (*)(guint))g_source_remove
};

bool GaimIMFactory::equals(const IMAccount & imAccount, std::string login, EnumIMProtocol::IMProtocol protocol) {
	return ((imAccount.getLogin() == login) && (imAccount.getProtocol() == protocol));
}

GaimIMFactory::GaimIMFactory() {
	AccountMngr = GaimAccountMngr::getInstance();
	ConnectMngr = GaimConnectMngr::getInstance();
	PresenceMngr = GaimPresenceMngr::getInstance();
	ChatMngr = GaimChatMngr::getInstance();
	ContactListMngr = GaimContactListMngr::getInstance();

	if (!g_thread_supported()) {
		g_thread_init(NULL);
	}
	g_thread_create(GaimMainEventLoop, NULL, FALSE, NULL);
}

GaimIMFactory::~GaimIMFactory() {
	if (gMainLoop) {
		LOG_DEBUG("Stopping gMainLoop");
		g_main_loop_quit(gMainLoop);
	} else {
		LOG_ERROR("No gMainLoop created");
	}
}

void GaimIMFactory::GaimSetCallbacks() {
	gaim_accounts_set_ui_ops(&acc_wg_ops);
	gaim_blist_set_ui_ops(&blist_wg_ops);
	gaim_privacy_set_ui_ops(&privacy_wg_ops);
	gaim_connections_set_ui_ops(&conn_wg_ops);
}

void GaimIMFactory::GaimWrapperInit() {
	AccountMngr->Init();
	ConnectMngr->Init();
	ContactListMngr->Init();
	ChatMngr->Init();
	PresenceMngr->Init();
}

void GaimIMFactory::GaimIMInit(const std::string & pathToProfile) {

	char * home_dir = g_build_filename(pathToProfile.c_str(), "gaim", NULL);
	
	// Remove Gaim config directory
	File file(home_dir);
	file.remove();

	File::createPath(home_dir + File::getPathSeparator());
	gaim_util_set_user_dir(home_dir);

	gaim_core_set_ui_ops(&core_wg_ops);
	gaim_eventloop_set_ui_ops(&eventloop_wg_ops);

	char * search_path = g_build_filename(Path::getApplicationDirPath().c_str(), "plugins", NULL);
	gaim_plugins_add_search_path(search_path);
	gaim_plugins_add_search_path("plugins");
	g_free(search_path);

	if (!gaim_core_init("Wengo GAIM")) {
		LOG_WARN("Initialization of the Gaim core failed\n");
	}
}

void GaimIMFactory::init(const std::string & pathToProfile) {
	GaimIMInit(pathToProfile);
	GaimWrapperInit();
}

static gboolean quit_timeout = 0;

void GaimIMFactory::GaimQuitCallback() {
	LOG_DEBUG("");
	// Don't stop gMainLoop here: this is called when the user logoff, but we
	// want to keep the loop running for the whole time the application is
	// running.
}

static gboolean terminate_cb(gpointer data)
{
	gaim_connections_set_ui_ops(&null_conn_wg_ops);
	gaim_accounts_set_ui_ops(&null_acc_wg_ops);
	gaim_blist_set_ui_ops(&null_blist_wg_ops);

	gaim_core_quit();

	if (quit_timeout > 0)
	{
		gaim_timeout_remove(quit_timeout);
		quit_timeout = 0;
	}

	return TRUE;
}

void GaimIMFactory::terminate() {
	if (AccountMngr) {
		AccountMngr->reset();
	}
	
	quit_timeout = gaim_timeout_add(0, (GSourceFunc) terminate_cb, NULL);
}

IMConnect * GaimIMFactory::createIMConnect(IMAccount &account) {
	return ConnectMngr->AddIMConnect(account);
}

IMChat * GaimIMFactory::createIMChat(IMAccount &account) {
	return ChatMngr->AddIMChat(account);
}

IMPresence * GaimIMFactory::createIMPresence(IMAccount &account) {
	return PresenceMngr->AddIMPresence(account);
}

IMContactList * GaimIMFactory::createIMContactList(IMAccount &account) {
	return ContactListMngr->AddIMContactList(account);
}

void GaimIMFactory::removeIMAccount(IMAccount imAccount) {
	AccountMngr->RemoveIMAccount(imAccount);
}

void GaimIMFactory::imAccountUpdated(IMAccount imAccount) {
	AccountMngr->UpdateIMAccount(imAccount);
}
