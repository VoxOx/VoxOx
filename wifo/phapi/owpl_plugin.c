#ifdef WIN32
#include <winsock2.h>
#endif /* WIN32 */

#include <owpl_plugin.h>
#include <phcall.h>
#include <phapi-util/phSharedLib.h>
#include <assert.h>

#ifdef WIN32

#else
#include <dlfcn.h>
#include <strings.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#endif

#define DBG	owpl_debug
#define LOG	owpl_log
#define L_ERR	1
#define L_WARN	2

OWPL_PLUGIN * owplPlugins=0;

#ifdef STATIC_SDP
extern OWPL_PLUGIN_EXPORT * sdp_exports;
#endif


static void owpl_debug(const char * format, ...);
static void owpl_log(int level, ...);

static void owpl_debug(const char * format, ...) {

}

static void owpl_log(int level, ...) {

}


/* initializes statically built (compiled in) modules*/
static int owplRegisterBuiltinModules()
{
	int ret;

	ret=0;
#ifdef STATIC_SDP
	ret=owplRegisterPlugin(sdp_exports,"built-in", 0);
	if (ret<0) return ret;
#endif
	return ret;
}

/* registers a module,  register_f= module register  functions
 * returns <0 on error, 0 on success */
static int owplRegisterPlugin(OWPL_PLUGIN_EXPORT * e, const char* path, void* handle)
{
	int ret;
	OWPL_PLUGIN * mod;

	ret=-1;

	/* add module to the list */
	if ((mod = malloc(sizeof(OWPL_PLUGIN)))==0){
		LOG(L_ERR, "load_module: memory allocation failure\n");
		ret = -1;
		goto error;
	}
	memset(mod,0, sizeof(OWPL_PLUGIN));
	mod->path=path;
	mod->handle=handle;
	mod->exports=e;
	mod->next = owplPlugins;
	owplPlugins=mod;
	return 0;
error:
	return ret;
}

#ifndef DLSYM_PREFIX
/* define it to null */
#define DLSYM_PREFIX
#endif

/* returns 0 on success , <0 on error */
int owplPluginLoad(const char* path)
{
	void* handle;
	char* error;
	OWPL_PLUGIN_EXPORT * exp;
	OWPL_PLUGIN * t;

	handle=dlopen(path, RTLD_NOW); /* resolve all symbols now */
	if (handle==0){
		LOG(L_ERR, "ERROR: load_module: could not open module <%s>: %s\n",
					path, dlerror() );
		goto error;
	}

	for(t=owplPlugins;t; t=t->next){
		if (t->handle==handle){
			LOG(L_WARN, "WARNING: load_module: attempting to load the same"
						" module twice (%s)\n", path);
			goto skip;
		}
	}

	/* launch register */
	exp = (OWPL_PLUGIN_EXPORT*)dlsym(handle, DLSYM_PREFIX "exports");
	if (strcmp(exp->owpl_plugin_version, OWPL_PLUGIN_VERSION) != 0)
	{
		LOG(L_ERR, "ERROR: load_module: Version mismatch\n");
		goto error1;
	}
	if ( (error =(char*)dlerror())!=0 ){
		LOG(L_ERR, "ERROR: load_module: %s\n", error);
		goto error1;
	}

	// Call module's init function
	if (exp->init_f) {
		exp->init_f();
	}
	// Register the module with the system
	if (owplRegisterPlugin(exp, path, handle)<0) {
		goto error1;
	}

	return 0;

error1:
	dlclose(handle);
error:
skip:
	return -1;
}

/*
 * owplIsPluginLoaded
 * Verify if a plugin with the specified name is loaded in memory
 */

int owplPluginIsLoaded(const char* name)
{
	OWPL_PLUGIN * t;

	t = owplGetPlugin(name);
	return t != 0;
}

/*
 * owplIsPluginLoaded
 * Get the handle to the plugin with the specified name
 */

OWPL_PLUGIN * owplGetPlugin(const char* name)
{
	OWPL_PLUGIN * t;

	for(t=owplPlugins;t;t=t->next){
		if (strcmp(name, t->exports->name)==0)
		{
			return t;
		}
	}
	return 0;
}

/* searches the module list and returns pointer to the "name" function or
 * 0 if not found
 * flags parameter is OR value of all flags that must match
 */
static owplPS_CommandProc owplPluginFindCommand(char* module_name, char* name, int param_no, int flags)
{
	OWPL_PLUGIN * t;
	OWPL_PLUGIN_CMD * cmd;

	for(t=owplPlugins;t;t=t->next){
		if (strcmp(module_name, t->exports->name)==0)
		{
			for(cmd=t->exports->cmds; cmd && cmd->name; cmd++){
				if((strcmp(name, cmd->name)==0)&&
				   (cmd->param_no==param_no) &&
				   ((cmd->flags & flags) == flags)
				  ){
					return cmd->function;
				}
			}
		}
	}
	DBG("find_export: <%s>:<%s> not found \n", module_name, name);
	return 0;
}



/*
 * searches the module list and returns pointer to "name" function in module "mod"
 * 0 if not found
 * flags parameter is OR value of all flags that must match
 */
static owplPS_CommandProc find_mod_export(char* mod, char* name, int param_no, int flags)
{
	OWPL_PLUGIN * t;
	OWPL_PLUGIN_CMD * cmd;

	for (t = owplPlugins; t; t = t->next) {
		if (strcmp(t->exports->name, mod) == 0) {
			for (cmd = t->exports->cmds;  cmd && cmd->name; cmd++) {
				if ((strcmp(name, cmd->name) == 0) &&
				    (cmd->param_no == param_no) &&
				    ((cmd->flags & flags) == flags)
				   ){
					DBG("find_mod_export: found <%s> in module %s [%s]\n",
					    name, t->exports->name, t->path);
					return cmd->function;
				}
			}
		}
	}

	DBG("find_mod_export: <%s> in module %s not found\n", name, mod);
	return 0;
}

static void owplPluginDestroy()
{
	OWPL_PLUGIN * t, *foo;

	t=owplPlugins;
	while(t) {
		foo=t->next;
		if ((t->exports)&&(t->exports->destroy_f)) t->exports->destroy_f();
		free(t);
		t=foo;
	}
	owplPlugins=0;
}

/*
 * Initialize all loaded modules, the initialization
 * is done *AFTER* the configuration file is parsed
 */
static int owplPluginInit(void)
{
	OWPL_PLUGIN * t;

	for(t = owplPlugins; t; t = t->next) {
		if ((t->exports) && (t->exports->init_f))
			if (t->exports->init_f() != 0) {
				LOG(L_ERR, "init_modules(): Error while initializing"
							" module %s\n", t->exports->name);
				return -1;
			}
	}
	return 0;
}

int CallCommandFunction(int * ret, void *func, int param_count, va_list ap)
{
	int i;
	char * params[10];
	for (i = 0; i < param_count; i++)
	{
		params[i] = va_arg(ap, char *);
	}

	switch (param_count)
	{
		case 0:
				*ret = ((owplPS_CommandProc0) func)();
				break;
		case 1:
				*ret = ((owplPS_CommandProc1) func)(params[0]);
				break;
		case 2:
				*ret = ((owplPS_CommandProc2) func)(params[0], params[1]);
				break;
		case 3:
				*ret = ((owplPS_CommandProc3) func)(params[0], params[1], params[2]);
				break;
		case 4:
				*ret = ((owplPS_CommandProc4) func)(params[0], params[1], params[2], params[3]);
				break;
		case 5:
				*ret = ((owplPS_CommandProc5) func)(params[0], params[1], params[2], params[3],
													params[4]);
				break;
		case 6:
				*ret = ((owplPS_CommandProc6) func)(params[0], params[1], params[2], params[3],
													params[4], params[5]);
				break;
		case 7:
				*ret = ((owplPS_CommandProc7) func)(params[0], params[1], params[2], params[3],
													params[4], params[5], params[6]);
				break;
		case 8:
				*ret = ((owplPS_CommandProc8) func)(params[0], params[1], params[2], params[3],
													params[4], params[5], params[6], params[7]);
				break;
		case 9:
				*ret = ((owplPS_CommandProc9) func)(params[0], params[1], params[2], params[3],
													params[4], params[5], params[6], params[7],
													params[8]);
				break;
		case 10:
				*ret = ((owplPS_CommandProc10) func)(params[0], params[1], params[2], params[3],
													 params[4], params[5], params[6], params[7],
													 params[8], params[9]);
				break;
		default:
				assert(0);
				return -1;
				break;
	}
	return 0;
}

int owplPluginCallFunction(int *retValue, char * PluginName, char * FuncName, char * ArgFormat, ...)
{
	char *ch;
	int n = 0;
	va_list ap;
	owplPS_CommandProc cmdProc;

	for (ch = ArgFormat; *ch; ch++)
	{
		if (*ch == '%') {
			n++;
		}
	}

	cmdProc = owplPluginFindCommand(PluginName, FuncName, n, 0);
	if (!cmdProc) {
		return -1;
	}

	va_start(ap, ArgFormat);
	//Call the plugin function
	CallCommandFunction(retValue, cmdProc, n, ap);
	va_end(ap);

	return 0;
}

/*************************************************************************************************/
/*										Param helper functions								     */
/*************************************************************************************************/

// Local helper function
static OWPL_PLUGIN_PARAM * GetPluginParam(const char * PluginName, const char * ParamName)
{
	OWPL_PLUGIN * Plugin;
	OWPL_PLUGIN_PARAM *param;

	Plugin = owplGetPlugin(PluginName);
	if (Plugin) {
		for (param = Plugin->exports->params;  param && param->name; param++) {
			if ((strcmp(ParamName, param->name) == 0)){
				return param;
			}
		}
	}
	return 0;
}

static OWPL_PLUGIN_PARAM * GetPluginParam2(OWPL_PLUGIN * Plugin, const char * ParamName)
{
	OWPL_PLUGIN_PARAM *param;
	for (param = Plugin->exports->params;  param && param->name; param++) {
		if ((strcmp(ParamName, param->name) == 0)){
			return param;
		}
	}
	return 0;
}

int owplPluginGetParam(void *retValue, int retSize, const char * PluginName, const char * ParamName)
{
	OWPL_PLUGIN_PARAM *param;
	OWPL_PLUGIN * p = owplGetPlugin(PluginName);
	if (p) {
		if (p->exports && p->exports->param_getter_f)
		{
			p->exports->param_getter_f(ParamName, retValue, retSize);
		}
		else
		{
			param = GetPluginParam2(p, ParamName);
			if (param) {
				return __owplPluginCopyValue(retValue, retSize, param->param_pointer, param->size);
			}
		}
	}
	return -1;
}

int owplPluginSetParam(const void *newValue, int valSize, const char * PluginName, const char * ParamName)
{
	OWPL_PLUGIN_PARAM *param;
	OWPL_PLUGIN * p = owplGetPlugin(PluginName);
	if (p) {
		if (p->exports && p->exports->param_getter_f)
		{
			p->exports->param_setter_f(ParamName, newValue, valSize);
		}
		else
		{
			param = GetPluginParam2(p, ParamName);
			if (param) {
				return __owplPluginCopyValue(param->param_pointer, param->size, newValue, valSize);
			}
		}
	}
	return -1;
}

int owplPluginGetParamInt(int *retValue, const char * PluginName, const char * ParamName)
{
	OWPL_PLUGIN_PARAM *param;
	param = GetPluginParam(PluginName, ParamName);
	if (param->type == OWPL_PARAM_INT)
	{
		return owplPluginGetParam(retValue, sizeof(int), PluginName, ParamName);
	}
	return -1;
}

int owplPluginSetParamInt(int newValue, const char * PluginName, const char * ParamName)
{
	OWPL_PLUGIN_PARAM *param;
	param = GetPluginParam(PluginName, ParamName);
	if (param->type == OWPL_PARAM_INT)
	{
		return owplPluginSetParam(&newValue, sizeof(int), PluginName, ParamName);
	}
	return -1;
}

int owplPluginGetParamString(char *retValue, int bufSize, const char * PluginName, const char * ParamName)
{
	OWPL_PLUGIN_PARAM *param;
	param = GetPluginParam(PluginName, ParamName);
	if (param->type == OWPL_PARAM_STR)
	{
		return owplPluginGetParam(retValue, bufSize, PluginName, ParamName);
	}
	return -1;
}

int owplPluginSetParamString(const char * newValue, int valSize, const char * PluginName, const char * ParamName)
{
	OWPL_PLUGIN_PARAM *param;
	param = GetPluginParam(PluginName, ParamName);
	if (param->type == OWPL_PARAM_STR)
	{
		return owplPluginSetParam(newValue, valSize, PluginName, ParamName);
	}
	return -1;
}


int __owplPluginCopyValue(void *dest, int destSize, const void *src, int srcSize)
{
	if (destSize >= srcSize)
	{
		memcpy(dest, src, srcSize);
		return 0;
	}
	return -1;
}

/**************************************************************************************************/
/*								Call & Plugin mapping routines									  */
/**************************************************************************************************/

OWPL_PLUGIN_CALLBACK * GetPluginCallback(const char * PluginName, const char * CallbackName)
{
	OWPL_PLUGIN * Plugin;
	OWPL_PLUGIN_CALLBACK *cb;

	Plugin = owplGetPlugin(PluginName);
	if (Plugin) {
		for (cb = Plugin->exports->callbacks;  cb && cb->name; cb++) {
			if ((strcmp(CallbackName, cb->name) == 0)){
				return cb;
			}
		}
	}
	return 0;
}


int owplPluginSetCallback(const char * PluginName, const char * CallbackName, owplPS_CommandProc CallbackProc)
{
	OWPL_PLUGIN_CALLBACK *cb = GetPluginCallback(PluginName, CallbackName);
	if (cb)
	{
		*cb->function = CallbackProc;
		return 0;
	}
	return -1;
}

/**************************************************************************************************/
/* INTERNAL:						Call & Plugin mapping routines								  */
/**************************************************************************************************/

OWPL_PLUGIN * owplGetPlugin4ContentType(const char * ContentType)
{
	OWPL_PLUGIN * t;

	for(t = owplPlugins; t; t = t->next) {
		if (strcmp(t->exports->ct_info->content_type, ContentType) == 0)
			return t;
	}
	return 0;
}

OWPL_PLUGIN * owplGetPlugin4Call(OWPL_CALL hCall)
{
	phcall_t *ca = ph_locate_call_by_cid(hCall);
	return ca->owplPlugin;
}

void owplAssociateCall2Plugin(OWPL_CALL hCall, OWPL_PLUGIN * plugin)
{
	phcall_t *ca = ph_locate_call_by_cid(hCall);
	ca->owplPlugin = plugin;
}

void owplAssociateCall2PluginByContentType(OWPL_CALL hCall, const char * ContentType)
{
	OWPL_PLUGIN * t = owplGetPlugin4ContentType(ContentType);
	if (t) {
		owplAssociateCall2Plugin(hCall, t);
	}
}

int owplCallIsBoundToSomePlugin(OWPL_CALL hCall)
{
	phcall_t *ca = ph_locate_call_by_cid(hCall);
	return ca->owplPlugin != 0;
}
