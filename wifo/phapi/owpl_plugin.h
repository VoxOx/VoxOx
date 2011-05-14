/* $Id: owpl_plugin.h,v 0.1 2006/10/12 14:47:00 $
 *
 * This plugin header file module is heavily based on the sr_plugin.h file of the SER project
 *
 */

#ifndef owpl_plugin_module_h
#define owpl_plugin_module_h

#include <phapi.h>
#include <phevents.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(WIN32) || defined(WIN32_WCE)
	#define OWPL_DECLARE_EXPORT __declspec(dllexport)
#else
	#define OWPL_DECLARE_EXPORT
#endif

//Define the version of OWPL plugin system
#define OWPL_PLUGIN_VERSION "0.1"

//typedef int modparam_t;
typedef enum owplParam_t
{
	OWPL_PARAM_INT,
	OWPL_PARAM_STR
} OWPL_PLUGIN_PARAM_TYPE;

typedef  int (*owplPS_CommandProc0)();
typedef  int (*owplPS_CommandProc1)(char * param1);
typedef  int (*owplPS_CommandProc2)(char * param1, char * param2);
typedef  int (*owplPS_CommandProc3)(char * param1, char * param2, char * param3);
typedef  int (*owplPS_CommandProc4)(char * param1, char * param2, char * param3,
									char * param4);
typedef  int (*owplPS_CommandProc5)(char * param1, char * param2, char * param3,
									char * param4, char * param5);
typedef  int (*owplPS_CommandProc6)(char * param1, char * param2, char * param3,
									char * param4, char * param5, char * param6);
typedef  int (*owplPS_CommandProc7)(char * param1, char * param2, char * param3,
									char * param4, char * param5, char * param6,
									char * param7);
typedef  int (*owplPS_CommandProc8)(char * param1, char * param2, char * param3,
									char * param4, char * param5, char * param6,
									char * param7, char * param8);
typedef  int (*owplPS_CommandProc9)(char * param1, char * param2, char * param3,
									char * param4, char * param5, char * param6,
									char * param7, char * param8, char * param9);
typedef  int (*owplPS_CommandProc10)(char * param1, char * param2, char * param3,
									char * param4, char * param5, char * param6,
									char * param7, char * param8, char * param9,
									char * param10);

typedef  struct owplPS_plugin_exports* (*module_register)();
//typedef  int (*owplPS_CommandProc)(const char * fmt, ...);
typedef  void * owplPS_CommandProc;
typedef void (*owplPS_DestroyProc)();
typedef int (*owplPS_InitProc)(void);
typedef int (*owplPS_ParamGetter)(const char * ParamName, void *retVal, int retSize);
typedef int (*owplPS_ParamSetter)(const char * ParamName, const void *newVal, int valSize);


struct owplPS_cmd_ {
	char* name;             /* null terminated command name */
	owplPS_CommandProc function;  /* pointer to the corresponding function */
	int param_no;           /* number of parameters used by the function */
	int flags;              /* Function flags */
};

struct owplPS_callback_ {
	char* name;             /* null terminated command name */
	owplPS_CommandProc * function;  /* pointer to the corresponding function */
};

struct owplPS_ContentType_ {
	const char* content_type;			  /* null terminated content type that the module handle */
	OWPL_EVENT_CALLBACK_PROC EventHandlerProc;
};

struct owplPS_param_ {
	char* name;             /* null terminated param. name */
	OWPL_PLUGIN_PARAM_TYPE type;        /* param. type */
	void* param_pointer;    /* pointer to the param. memory location */
	int size; /* param. size */
};

typedef struct owplPS_cmd_ OWPL_PLUGIN_CMD;
typedef struct owplPS_callback_ OWPL_PLUGIN_CALLBACK;
typedef struct owplPS_param_ OWPL_PLUGIN_PARAM;
typedef struct owplPS_event_handler_ OWPL_PLUGIN_EVENT_HANDLER;
typedef struct owplPS_ContentType_ OWPL_PLUGIN_CONTENT_TYPE;

typedef struct owplPS_plugin_exports{
	const char* name;                     /* null terminated module name */

	const char* owpl_plugin_version;      /* The version of the owpl_plugin system that the plugin was compiled against */

	OWPL_PLUGIN_CONTENT_TYPE * ct_info; /* The structure that describes what content type is handled
											 by the plugin and the callback function to handle it*/

	OWPL_PLUGIN_CMD * cmds;             /* null terminated array of the exported commands */


	OWPL_PLUGIN_CALLBACK * callbacks;             /* null terminated array of the callback functions*/

	OWPL_PLUGIN_PARAM * params;         /* null terminated array of the exported module parameters */

	owplPS_InitProc init_f;           /* Initialization function */

	owplPS_DestroyProc destroy_f;     /* function called when the module should
									   be "destroyed", can be null */
	owplPS_ParamGetter param_getter_f;
	owplPS_ParamSetter param_setter_f;
} OWPL_PLUGIN_EXPORT;

typedef struct owpl_plugin{
	const char* path;
	void* handle;
	struct owplPS_plugin_exports * exports;
	struct owpl_plugin * next;
} OWPL_PLUGIN;

extern OWPL_PLUGIN * owplPlugins; /* global module list*/

MY_DLLEXPORT int owplPluginLoad(const char* path);

MY_DLLEXPORT OWPL_PLUGIN * owplGetPlugin(const char* name);

MY_DLLEXPORT int owplPluginIsLoaded(const char* name);

MY_DLLEXPORT int owplPluginCallFunction(int *retValue, char * PluginName, char * FuncName, char * ArgFormat, ...);

MY_DLLEXPORT int owplPluginGetParam(void *retValue, int retSize, const char * PluginName, const char * ParamName);

MY_DLLEXPORT int owplPluginSetParam(const void *newValue, int valSize, const char * PluginName, const char * ParamName);

MY_DLLEXPORT int owplPluginGetParamInt(int *retValue, const char * PluginName, const char * ParamName);

MY_DLLEXPORT int owplPluginSetParamInt(int newValue, const char * PluginName, const char * ParamName);

MY_DLLEXPORT int owplPluginGetParamString(char *retValue, int bufSize, const char * PluginName, const char * ParamName);

MY_DLLEXPORT int owplPluginSetParamString(const char * newValue, int valSize, const char * PluginName, const char * ParamName);

MY_DLLEXPORT int owplPluginSetCallback(const char * PluginName, const char * CallbackName, owplPS_CommandProc CallbackProc);

MY_DLLEXPORT int __owplPluginCopyValue(void *dest, int destSize, const void *src, int srcSize);

/**************************************************************************************************/
/*								INTERNAL: Call & Plugin mapping routines									  */
/**************************************************************************************************/

OWPL_PLUGIN * owplGetPlugin4ContentType(const char * ContentType);

OWPL_PLUGIN * owplGetPlugin4Call(OWPL_CALL hCall);

void owplAssociateCall2Plugin(OWPL_CALL hCall, OWPL_PLUGIN * plugin);

void owplAssociateCall2PluginByContentType(OWPL_CALL hCall, const char * ContentType);

int owplCallIsBoundToSomePlugin(OWPL_CALL hCall);

#ifdef __cplusplus
} //extern "C" {
#endif



#endif
