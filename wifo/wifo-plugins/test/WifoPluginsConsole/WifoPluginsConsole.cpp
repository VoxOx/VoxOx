#include "phapi.h"
#include "phCallbacks.h"
#include "sfpCallbacks.h"

#include <phapi-util/phapi-globals.h>
#include <sfp-plugin/sfp-plugin.h>
#include <phapi-util/util.h>
#include <phapi-util/mystring.h>
#include <phapi-util/mystdio.h>

#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef WIN32
#include <tchar.h>
#endif

// ----------

#define MAX_INPUT_SIZE	256

// ----------

static unsigned int display_menu(const char * step);
static unsigned int execute_action(const char * input);
#ifdef WIN32
static unsigned int __stdcall interact(void * arg);
#else
static void * __stdcall interact(void * arg);
#endif

// ----------

int vlid = -1;
int	g_callId = -1;
char SipDomain[200] = "";
int wx_wengo = 1;
char step[16];
int users_to_callids[9] = {-1,-1,-1,-1,-1,-1,-1,-1,-1};
// ----------

static int get_callid_from_wxwengo_user(const char * username){
	return users_to_callids[username[9]];
}

static unsigned int display_menu(const char * step){

	if(strfilled(step) && strequals(step, "start")){
		printf("\nAvailable commands :\n");
		printf(" - register\n");		
		printf(" - quit\n\n");
		printf("> ");
	}else if(strfilled(step) && strequals(step, "send")){
		printf("\nAvailable commands :\n");
		printf(" - send <user>;<full_file_path>;<filename>;<file_type>;<file_size_in_bytes>\n");		
		printf(" - quit\n\n");
		printf("> ");
	}else if(strfilled(step) && strequals(step, "accept")){
		printf("\nAvailable commands :\n");
		printf(" - accept <user>;<full_file_path>\n");
		printf(" - refuse <user>\n");
		printf(" - quit\n\n");
		printf("> ");
	}else if(strfilled(step) && strequals(step, "cancel")){
		printf("\nAvailable commands :\n");
		printf(" - cancel <user>\n\n");
		printf("> ");
	}else{
		printf("\nAvailable commands :\n");
		printf(" - send <user>;<full_file_path>;<filename>;<file_type>;<file_size_in_bytes>\n");
		printf(" - accept <user>;<full_file_path>\n");
		printf(" - pause <user>\n");
		printf(" - resume <user>\n");
		printf(" - refuse <user>\n");
		printf(" - cancel <user>\n");
		printf(" - quit\n\n");
		printf("> ");
	}
	
	return 1;
}

static unsigned int execute_action(const char * input){
	char command[MAX_INPUT_SIZE];
	char argument[MAX_INPUT_SIZE];

	sscanf2(input, "%s %s\n", command, sizeof(command), argument, sizeof(argument));

	if(!strfilled(command)){
		return FALSE;
	}

	if(strequals(step, "start") && strequals(command, "register")){

		int i;
		char _username[100];
		char _display_name[100];
		char _passwd[100];
		char _domain[100];
		char _proxy[100];
		char _server[100];

		phTerminate();	

		strncpy(_proxy, "192.168.4.58:4060", sizeof(_proxy));
		snprintf(_display_name, sizeof(_display_name), "%s%d", "wx_wengo_", wx_wengo);
		//if(wx_wengo == 1) strncpy(_display_name, "wx_wengo_1", sizeof(_display_name));
		//if(wx_wengo == 2) strncpy(_display_name, "wx_wengo_2", sizeof(_display_name));
		strncpy( _passwd, "testwengo", sizeof(_passwd));
		strncpy( _domain, "voip.wengo.fr", sizeof(_domain));
		snprintf(_username, sizeof(_username), "%s%d", "wx_wengo_", wx_wengo);
		//if(wx_wengo == 1) strncpy(_username, "wx_wengo_1", sizeof(_username));
		//if(wx_wengo == 2) strncpy(_username, "wx_wengo_2", sizeof(_username));
		strncpy(_server, "voip.wengo.fr", sizeof(_server));

		memset(&phcfg, 0, sizeof(phcfg));
		strcpy(phcfg.local_rtp_port, "10601");
		snprintf(phcfg.sipport, sizeof(phcfg.sipport), "%s%d", "506", wx_wengo);
		//if(wx_wengo == 1) strcpy(phcfg.sipport, "5061");
		//if(wx_wengo == 2) strcpy(phcfg.sipport, "5062");
		strcpy(phcfg.nattype, "none");
		strcpy(phcfg.audio_dev,"IN=0 OUT=0");
		phcfg.asyncmode = 0;
		
		strncpy(phcfg.proxy, _proxy, sizeof(phcfg.proxy));

		phcfg.force_proxy = 1;
		phcfg.use_tunnel = 0;
		sprintf(phcfg.identity, "%s <sip:%s@%s>", _display_name, _username, _domain);

		i = phInit(&callbacks, "127.0.0.1", 0);

		phcfg.nat_refresh_time = 15; // TO CALL AFTER PHINIT, ELSE USELESS!!!!

		if (i == 0) {
			i = phAddAuthInfo(_username, _username, _passwd, 0, _domain);
		}

		if (i == 0) {
			vlid = phAddVline(_username, _server, _proxy, 10000);
		}else if (phcb) {
			phcb->regProgress(0, -1);
		}

		strncpy(SipDomain, _domain, sizeof(SipDomain));

		// register of sfp plugin
		sfp_register_plugin();
		sfp_set_plugin_callbacks(&sfp_cbks);
		sfp_set_file_transfer_port("11000");

		strncpy(step, "send", sizeof(step));

	}else if(strequals(command, "send")){

		char _dest[200];
		char tmp[200];
		char file_path[MAX_INPUT_SIZE];
		char file_name[MAX_INPUT_SIZE];
		char file_type[MAX_INPUT_SIZE];
		char file_size[MAX_INPUT_SIZE];

		if(!strfilled(argument)){
			return FALSE;
		}

		sscanf2(argument, "%s;%s;%s;%s;%s", _dest, sizeof(_dest), file_path, sizeof(file_path), file_name, sizeof(file_name), file_type, sizeof(file_type), file_size, sizeof(file_size));

		if(!strfilled(_dest) || !strfilled(file_path) || !strfilled(file_name) || !strfilled(file_type) || !strfilled(file_size)){
			return FALSE;
		}

		//if(wx_wengo == 2) strncpy(_dest, "wx_wengo_1", sizeof(_dest));
		//if(wx_wengo == 1) strncpy(_dest, "wx_wengo_2", sizeof(_dest));

		if (!strstr(_dest, "@") && !strstr("sip:", _dest)) {
			sprintf(tmp, "sip:%s@%s", _dest, SipDomain);
		}

		sfp_set_file_transfer_port("11000");
		sfp_send_file(vlid, phcfg.identity, tmp, file_path, file_name, file_type, file_size);

		//strncpy(step, "cancel", sizeof(step));

	}else if(strequals(command, "accept")){
		char user[32];
		char filename[128];
		int call_id = -1;

		if(!strfilled(argument)){
			return FALSE;
		}

		sscanf2(argument, "%s;%s", user, sizeof(user), filename, sizeof(filename));

		if(!strfilled(user) || !strfilled(filename)){
			return FALSE;
		}

		if((call_id = get_callid_from_wxwengo_user(user)) > 0){
			sfp_receive_file(call_id, argument);

			//strncpy(step, "cancel", sizeof(step));
		}

	}else if(strequalsamong(command, 2,"cancel", "refuse")){
		int call_id = -1;

		if(!strfilled(argument)){
			return FALSE;
		}

		if((call_id = get_callid_from_wxwengo_user(argument)) > 0){
			sfp_cancel_transfer(call_id);
		}

		//strncpy(step, "send", sizeof(step));

	}else if(strequals(command, "pause")){
		int call_id = -1;

		if(!strfilled(argument)){
			return FALSE;
		}

		if((call_id = get_callid_from_wxwengo_user(argument)) > 0){
			sfp_pause_transfer(call_id);
		}
	}else if(strequals(command, "resume")){
		int call_id = -1;

		if(!strfilled(argument)){
			return FALSE;
		}

		if((call_id = get_callid_from_wxwengo_user(argument)) > 0){
			sfp_resume_transfer(call_id);
		}
	}else{
		return FALSE;
	}

	return TRUE;
}

#ifdef WIN32
static unsigned int __stdcall interact(void * arg){
#else
static void * __stdcall interact(void * arg){
#endif
	char input[MAX_INPUT_SIZE];

	//while(fgets(input, sizeof(input), stdin)){
	while(gets(input)){
		// if a quit, then quit
		if(strequals(input, "quit")){
			strncpy(step, "quit", sizeof(step));
			printf("\nBye.\n");
			return 0;
		}
		execute_action(input);
	}

	return 0;
}
// ----------

int main(int argc, char * argv[]){
	char input[MAX_INPUT_SIZE];
	pthread_t thread;

	if(argc < 2){
		printf("usage :\n");
		printf("WifoPluginsConsole <userid>\n");
		printf("with <userid equal to 1 or 2\n");
		return -1;
	}

	strncpy(step, "start", sizeof(step));
	wx_wengo = atoi(argv[1]);


	printf("Using wx_wengo_%d\n\n", wx_wengo);

	// register or quit
	while(strequals(step, "start")){
		display_menu(step);
		//fgets(input, sizeof(input), stdin);
		gets(input);
		// if a quit, then quit
		if(strequals(input, "quit")){
			printf("\nBye.\n");
			return 0;
		}
		execute_action(input);
	}

	// display whole menu
	display_menu(NULL);

	// launch a thread to display the menu
	if(pthread_create(&thread, NULL, interact, NULL) != 0){
		// TODO notify GUI
		return FALSE;
	}


	// event loop
	while(phPoll() == 0){
		if(strequals(step, "quit")){
			break;
		}
		usleep(40);
	}

	return 0;
}


