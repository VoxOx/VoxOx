/*
 * C/Python wrapper for phapi
 *
 * Copyright (C) 2005-2007 Wengo SAS
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2,
 * or (at your option) any later version.
 *
 * This is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with dpkg; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/**
 * @author David Ferlier <david.ferlier@wengo.fr>
 * @author Mathieu Stute <mathieu.stute@wengo.com>
 */

/**
 *  TODO: debug video support
 *
 *  - conference functions
 */

#define MAX_URI_LENGTH 512
#define MAX_IP_LENGTH 64
#define MAX_PASSWORD_LENGTH 64
#define MAX_USERNAME_LENGTH 64

#include <Python.h>
#include <phapi.h>

#ifdef WIN32
    #include <winsock2.h>
    #undef DEBUG
    #undef _DEBUG

    #include <windows.h>
    #include <shlwapi.h>

    static void (*userNotify)(char *buf, int size);
    LONG unhandledExceptionFilter(struct _EXCEPTION_POINTERS * pExceptionInfo) {
        userNotify(NULL, NULL);
        return 0;
    }
#endif  //OS_WINDOWS

void pyphapi_lock_and_call(PyObject *callback, PyObject *args);
PyMODINIT_FUNC initpyphapi(void);

#if 0
/**
 * @brief Initializes the module
 */
static PyObject * pyphapi(PyObject* self) {
    PyEval_InitThreads();

    return Py_BuildValue("s","Hello");
}
#endif

/**
 * @brief Lock the Python GIL (Global Interpreter Lock) and call a function
 *
 * This function first locks the GIL, and then calls a python callback
 * with some arguments. Once called it decrements ref count of the
 * arguments variable, and then releases the GIL.
 *
 * @param   callback    The function to call
 * @param   args        The arguments to pass to the function
 *
 */
void pyphapi_lock_and_call(PyObject *callback, PyObject *args) {
    PyGILState_STATE gstate;

    if (callback) {
        gstate = PyGILState_Ensure();
        PyEval_CallObject(callback, args);

        Py_DECREF(args);
        PyGILState_Release(gstate);
    }
}


/**
 * @brief Wraps phConf
 */
static PyObject * PyPhConf(PyObject *self, PyObject *params) {
    int cid1, cid2;
    int ret = -1;

    if (PyArg_ParseTuple(params, "ii", &cid1, &cid2)) {
        printf("PyPhConf: %i / %i\n", cid1, cid2);
        ret = phConf(cid1, cid2);
    }
    return Py_BuildValue("i", ret);
}

static PyObject *pyowpl_callProgress = NULL;
static PyObject *pyowpl_registerProgress =  NULL;
static PyObject *pyowpl_messageProgress =  NULL;
static PyObject *pyowpl_subscriptionProgress =  NULL;
static PyObject *pyowpl_onNotify = NULL;
static PyObject *pyowpl_errorNotify = NULL;

static int phApiEventsHandler(OWPL_EVENT_CATEGORY category, void* pInfo, void* pUserData);

static void pyowpl_callback_callProgress(OWPL_CALLSTATE_INFO * info);
static void pyowpl_callback_registerProgress(OWPL_LINESTATE_INFO * info);
static void pyowpl_callback_messageProgress(OWPL_MESSAGE_INFO * info);
static void pyowpl_callback_subscriptionProgress(OWPL_SUBSTATUS_INFO * info);
static void pyowpl_callback_onNotify(OWPL_NOTIFICATION_INFO * info);
static void pyowpl_callback_errorNotify(OWPL_ERROR_INFO * info);


static int phApiEventsHandler(OWPL_EVENT_CATEGORY category, void* pInfo, void* pUserData) {

    switch(category) {
        case EVENT_CATEGORY_CALLSTATE :
            pyowpl_callback_callProgress((OWPL_CALLSTATE_INFO *)pInfo);
            break;

        case EVENT_CATEGORY_LINESTATE :
            pyowpl_callback_registerProgress((OWPL_LINESTATE_INFO *)pInfo);
            break;

        case EVENT_CATEGORY_MESSAGE :
            pyowpl_callback_messageProgress((OWPL_MESSAGE_INFO *)pInfo);
            break;

        case EVENT_CATEGORY_SUB_STATUS :
            pyowpl_callback_subscriptionProgress((OWPL_SUBSTATUS_INFO *)pInfo);
            break;

        case EVENT_CATEGORY_NOTIFY :
            pyowpl_callback_onNotify((OWPL_NOTIFICATION_INFO *)pInfo);
            break;

        case EVENT_CATEGORY_ERROR :
            pyowpl_callback_errorNotify((OWPL_ERROR_INFO *)pInfo);
            break;

        default :
            break;
    }
    return 0;
}

static void pyowpl_callback_callProgress(OWPL_CALLSTATE_INFO * info) {
    PyObject * cInfo;

    if(info && pyowpl_callProgress) {
        cInfo = Py_BuildValue("(iiiis)",
            info->event,
            info->cause,
            info->hLine,
            info->hCall,
            info->szRemoteIdentity);
        pyphapi_lock_and_call(pyowpl_callProgress, cInfo);
    }
}

static void pyowpl_callback_registerProgress(OWPL_LINESTATE_INFO * info) {
    PyObject *reg_info;

    if(info && pyowpl_registerProgress) {
        reg_info = Py_BuildValue("(iii)",
            info->event,
            info->cause,
            info->hLine);
        pyphapi_lock_and_call(pyowpl_registerProgress, reg_info);
    }
}

static void pyowpl_callback_messageProgress(OWPL_MESSAGE_INFO * info) {
    PyObject * mInfo;

    if(info && pyowpl_messageProgress) {
        mInfo = Py_BuildValue("(iiisssss)",
            info->event,
            info->cause,
            info->messageId,
            info->szContentType,
            info->szSubContentType,
            info->szLocalIdentity,
            info->szRemoteIdentity,
            info->szContent);

        pyphapi_lock_and_call(pyowpl_messageProgress, mInfo);
    }
}

static void pyowpl_callback_subscriptionProgress(OWPL_SUBSTATUS_INFO * info) {
    PyObject *sInfo;

    if(info) {
        sInfo = Py_BuildValue("(iiis)",
            info->state,
            info->cause,
            info->hSub,
            info->szRemoteIdentity);
        pyphapi_lock_and_call(pyowpl_subscriptionProgress, sInfo);
    }
}

static void pyowpl_callback_onNotify(OWPL_NOTIFICATION_INFO * info) {
    PyObject * nInfo;
    OWPL_NOTIFICATION_STATUS_INFO *stInfos;


    if (info->event == NOTIFICATION_PRESENCE) {
        stInfos = (OWPL_NOTIFICATION_STATUS_INFO *) info->Data.StatusInfo;
        //FIXME: parsing the content make Python crash
        //printf("content:%s\n", info->szXmlContent);
        nInfo = Py_BuildValue("(ds)",
                              info->event,
                              stInfos->szRemoteIdentity,
                              info->szXmlContent);
        pyphapi_lock_and_call(pyowpl_onNotify, nInfo);
    }
}

static void pyowpl_callback_errorNotify(OWPL_ERROR_INFO * info) {
    PyObject * eInfo;

    if(info) {
        eInfo = Py_BuildValue("(i)",
            info->event);
        pyphapi_lock_and_call(pyowpl_errorNotify, eInfo);
    }
}

static PyObject * PyOwplEventListenerAdd(PyObject * self, PyObject * params) {
    PyObject *result = NULL;
    PyObject * callback_callProgress;
    PyObject * callback_registerProgress;
    PyObject * callback_messageProgress;
    PyObject * callback_subscriptionProgress;
    PyObject * callback_onNotify;
    PyObject * callback_errorNotify;

    int pycode;

    pycode = PyArg_ParseTuple(params, "OOOOOO",
        &callback_callProgress,
        &callback_registerProgress,
        &callback_messageProgress,
        &callback_subscriptionProgress,
        &callback_onNotify,
        &callback_errorNotify);

    if(!pycode) {
        return NULL;
    }

    if (!PyCallable_Check(callback_callProgress)) {
        PyErr_SetString(PyExc_TypeError, "parameter must be callable");
        return NULL;
    }

    if (!PyCallable_Check(callback_registerProgress)) {
        PyErr_SetString(PyExc_TypeError, "parameter must be callable");
        return NULL;
    }

    if (!PyCallable_Check(callback_messageProgress)) {
        PyErr_SetString(PyExc_TypeError, "parameter must be callable");
        return NULL;
    }

    if (!PyCallable_Check(callback_subscriptionProgress)) {
        PyErr_SetString(PyExc_TypeError, "parameter must be callable");
        return NULL;
    }

    if (!PyCallable_Check(callback_onNotify)) {
        PyErr_SetString(PyExc_TypeError, "parameter must be callable");
        return NULL;
    }

    if (!PyCallable_Check(callback_errorNotify)) {
        PyErr_SetString(PyExc_TypeError, "parameter must be callable");
        return NULL;
    }

    Py_XINCREF(callback_callProgress);
    Py_XDECREF(pyowpl_callProgress);

    Py_XINCREF(callback_registerProgress);
    Py_XDECREF(pyowpl_registerProgress);

    Py_XINCREF(callback_messageProgress);
    Py_XDECREF(pyowpl_messageProgress);

    Py_XINCREF(callback_subscriptionProgress);
    Py_XDECREF(pyowpl_subscriptionProgress);

    Py_XINCREF(callback_onNotify);
    Py_XDECREF(pyowpl_onNotify);

    Py_XINCREF(callback_errorNotify);
    Py_XDECREF(pyowpl_errorNotify);

    Py_INCREF(Py_None);
    result = Py_None;

    pyowpl_callProgress = callback_callProgress;
    pyowpl_registerProgress = callback_registerProgress;
    pyowpl_messageProgress = callback_messageProgress;
    pyowpl_subscriptionProgress = callback_subscriptionProgress;
    pyowpl_onNotify = callback_onNotify;
    pyowpl_errorNotify = callback_errorNotify;

    owplEventListenerAdd(phApiEventsHandler, NULL);

    return result;
}

/**
 * @brief Wraps owplInit()
 */
static PyObject * PyOwplInit(PyObject * self, PyObject * params) {
    const int asyncCallbackMode;
    const int udpPort;
    const int tcpPort;
    const int tlsPort;
    const char* szBindToAddr;
    const int bUserSequentialPorts;

    int pycode, ret;

    pycode = PyArg_ParseTuple(params,
        "iiiisi",
        &asyncCallbackMode,
        &udpPort,
        &tcpPort,
        &tlsPort,
        &szBindToAddr,
        &bUserSequentialPorts);

    if(!pycode) {
        return Py_None;
    }

    ret = owplInit(asyncCallbackMode,
        udpPort,
        tcpPort,
        tlsPort,
        szBindToAddr,
        bUserSequentialPorts);
    return Py_BuildValue("i", ret);
}

/**
 * @brief Wraps owplShutdown()
 */
static PyObject * PyOwplShutdown(PyObject * self, PyObject * params) {
    int ret;
    ret = owplShutdown();
    return Py_BuildValue("i", ret);
}

/**
 * @brief Wraps owplConfigSetLocalHttpProxy()
 */
static PyObject * PyOwplConfigSetLocalHttpProxy(PyObject *self, PyObject *params) {
    const char* szLocalProxyAddr;
    const int LocalProxyPort;
    const char* szLocalProxyUserName;
    const char* szLocalProxyPasswd;

    int pycode, ret;

    pycode = PyArg_ParseTuple(params,
        "siss",
        &szLocalProxyAddr,
        &LocalProxyPort,
        &szLocalProxyUserName,
        &szLocalProxyPasswd);

    if (!pycode) {
        return Py_None;
    }

    ret = owplConfigSetLocalHttpProxy(szLocalProxyAddr,
        LocalProxyPort,
        szLocalProxyUserName,
        szLocalProxyPasswd);
    return Py_BuildValue("i", ret);
}

/**
 * @brief Wraps owplConfigSetHttpTunnel()
 */
static PyObject * PyOwplConfigSetHttpTunnel(PyObject *self, PyObject *params) {
    const char *address;
    unsigned int port;
    unsigned int timeout;

    int pycode, ret;

    pycode = PyArg_ParseTuple(params,
        "sii",
        &address,
        &port,
        &timeout);

    if (!pycode) {
        return Py_None;
    }

    ret = owplConfigSetHttpTunnel(address, (short)port, timeout);
    return Py_BuildValue("i", ret);
}

/**
 * @brief Wraps owplConfigEnableHttpTunnel()
 */
static PyObject * PyOwplConfigEnableHttpTunnel(PyObject *self, PyObject *params) {
    unsigned int tunnelMode;
	unsigned int enableHttps;

    int pycode, ret;

    pycode = PyArg_ParseTuple(params,
        "ii",
        &tunnelMode,
        &enableHttps);

    if (!pycode) {
        return Py_None;
    }

    ret = owplConfigEnableHttpTunnel(tunnelMode, enableHttps);
    return Py_BuildValue("i", ret);
}


/**
 * @brief Wraps owplConfigAddAudioCodecByName
 */
static PyObject * PyOwplConfigAddAudioCodecByName(PyObject *self, PyObject *params) {
    const char* szCodecName;

    int pycode, ret;

    pycode = PyArg_ParseTuple(params,
        "s",
        &szCodecName);

    if (!pycode) {
        return Py_None;
    }

    ret = owplConfigAddAudioCodecByName(szCodecName);
    return Py_BuildValue("i", ret);
}

/**
 * @brief Wraps owplConfigSetAudioCodecs
 */
static PyObject * PyOwplConfigSetAudioCodecs(PyObject *self, PyObject *params) {
    const char* szCodecList;

    int pycode, ret;

    pycode = PyArg_ParseTuple(params,
        "s",
        &szCodecList);

    if (!pycode) {
        return Py_None;
    }

    ret = owplConfigSetAudioCodecs(szCodecList);
    return Py_BuildValue("i", ret);
}

/**
 * @brief Wraps owplConfigAddVideoCodecByName
 */
static PyObject * PyOwplConfigAddVideoCodecByName(PyObject *self, PyObject *params) {
    const char* szCodecName;

    int pycode, ret;

    pycode = PyArg_ParseTuple(params,
        "s",
        &szCodecName);

    if (!pycode) {
        return Py_None;
    }

    ret = owplConfigAddVideoCodecByName(szCodecName);
    return Py_BuildValue("i", ret);
}

/**
 * @brief Wraps owplConfigSetVideoCodecs
 */
static PyObject * PyOwplConfigSetVideoCodecs(PyObject *self, PyObject *params) {
    const char* szCodecList;

    int pycode, ret;

    pycode = PyArg_ParseTuple(params,
        "s",
        &szCodecList);

    if (!pycode) {
        return Py_None;
    }

    ret = owplConfigSetVideoCodecs(szCodecList);
    return Py_BuildValue("i", ret);
}

/**
 * @brief Wraps owplConfigGetBoundLocalAddr
 */
static PyObject * PyOwplConfigGetBoundLocalAddr(PyObject *self, PyObject *params) {
    char szLocalAddr[MAX_URI_LENGTH]; // not to be passed by Python

    int ret;

    ret = owplConfigGetBoundLocalAddr(szLocalAddr, sizeof(szLocalAddr));

    if(ret == OWPL_RESULT_SUCCESS) {
        return Py_BuildValue("s", szLocalAddr);
    }

    return Py_None;
}

/**
 * @brief Wraps owplConfigLocalHttpProxyGetAddr
 */
static PyObject * PyOwplConfigLocalHttpProxyGetAddr(PyObject *self, PyObject *params) {
    char szLocalProxyAddr[MAX_IP_LENGTH]; // not to be passed by Python

    int ret;

    ret = owplConfigGetBoundLocalAddr(szLocalProxyAddr,
        sizeof(szLocalProxyAddr));

    if(ret == OWPL_RESULT_SUCCESS) {
        return Py_BuildValue("s", szLocalProxyAddr);
    }

    return Py_None;
}

/**
 * @brief Wraps owplConfigLocalHttpProxyGetPasswd
 */
static PyObject * PyOwplConfigLocalHttpProxyGetPasswd(PyObject *self, PyObject *params) {
    char szLocalProxyPasswd[MAX_PASSWORD_LENGTH]; // not to be passed by Python

    int ret;

    ret = owplConfigGetBoundLocalAddr(szLocalProxyPasswd,
        sizeof(szLocalProxyPasswd));

    if(ret == OWPL_RESULT_SUCCESS) {
        return Py_BuildValue("s", szLocalProxyPasswd);
    }

    return Py_None;
}

/**
 * @brief Wraps owplConfigLocalHttpProxyGetPort
 */
static PyObject * PyOwplConfigLocalHttpProxyGetPort(PyObject *self, PyObject *params) {
    unsigned short LocalProxyPort; // not to be passed by Python

    int pycode, ret;

    pycode = PyArg_ParseTuple(params,
        "i",
        &LocalProxyPort);

    if (!pycode) {
        return Py_None;
    }

    ret = owplConfigLocalHttpProxyGetPort(&LocalProxyPort);

    if(ret == OWPL_RESULT_SUCCESS) {
        return Py_BuildValue("i", LocalProxyPort);
    }

    return Py_None;
}

/**
 * @brief Wraps owplConfigLocalHttpProxyGetUserName
 */
static PyObject * PyOwplConfigLocalHttpProxyGetUserName(PyObject *self, PyObject *params) {
    char szLocalProxyUserName[MAX_USERNAME_LENGTH]; // not to be passed by Python

    int ret;

    ret = owplConfigLocalHttpProxyGetUserName(szLocalProxyUserName,
        sizeof(szLocalProxyUserName));

    if(ret == OWPL_RESULT_SUCCESS) {
        return Py_BuildValue("s", szLocalProxyUserName);
    }

    return Py_None;
}

/**
 * @brief Wraps owplAudioSetConfigString
 */
static PyObject * PyOwplAudioSetConfigString(PyObject *self, PyObject *params) {
    const char* szAudioConfig;

    int pycode, ret;

    pycode = PyArg_ParseTuple(params,
        "s",
        &szAudioConfig);

    if (!pycode) {
        return Py_None;
    }

    ret = owplAudioSetConfigString(szAudioConfig);
    return Py_BuildValue("i", ret);
}

/**
 * @brief Wraps owplLineAdd
 */
static PyObject * PyOwplLineAdd(PyObject *self, PyObject *params) {
    const char *displayname;
    const char *sipUsername;
    const char *sipServer;
    const char *sipProxy;
    OWPL_TRANSPORT_PROTOCOL sipTransport;
    int regTimeout;
    OWPL_LINE hLine; // not to be passed by Python

    int pycode, ret;

    pycode = PyArg_ParseTuple(params,
        "ssssii",
        &displayname,
        &sipUsername,
        &sipServer,
        &sipProxy,
        &sipTransport,
        &regTimeout);

    if (!pycode) {
        return Py_None;
    }

    ret = owplLineAdd(displayname,
        sipUsername,
        sipServer,
        sipProxy,
        sipTransport,
        regTimeout,
        &hLine);

    if(ret == OWPL_RESULT_SUCCESS) {
        return Py_BuildValue("i", hLine);
    }

    return Py_None;
}

/**
 * @brief Wraps owplLineDelete
 */
static PyObject * PyOwplLineDelete(PyObject *self, PyObject *params) {
    const OWPL_LINE hLine;
    const unsigned short skipUnregister;

    int pycode, ret;

    pycode = PyArg_ParseTuple(params,
        "ii",
        &hLine,
        &skipUnregister);

    if (!pycode) {
        return Py_None;
    }

    ret = owplLineDelete(hLine, skipUnregister);
    return Py_BuildValue("i", ret);
}

/**
 * @brief Wraps owplLineGetProxy
 */
static PyObject * PyOwplLineGetProxy(PyObject *self, PyObject *params) {
    const OWPL_LINE hLine;
    char szBuffer[MAX_IP_LENGTH]; // not to be passed by Python
    int nBuffer; // not to be passed by Python

    int pycode, ret;

    pycode = PyArg_ParseTuple(params,
        "i",
        &hLine);

    if (!pycode) {
        return Py_None;
    }

    ret = owplLineGetProxy(hLine,
        szBuffer,
        &nBuffer);

    if(ret == OWPL_RESULT_SUCCESS) {
        return Py_BuildValue("s", szBuffer);
    }

    return Py_None;
}

/**
 * @brief Wraps owplLineGetLocalUserName
 */
static PyObject * PyOwplLineGetLocalUserName(PyObject *self, PyObject *params) {
    const OWPL_LINE hLine;
    char szLocalUserName[MAX_USERNAME_LENGTH]; // not to be passed by Python
    int nBuffer = sizeof(szLocalUserName); // not to be passed by Python

    int pycode, ret;

    pycode = PyArg_ParseTuple(params,
        "i",
        &hLine);

    if (!pycode) {
        return Py_None;
    }

    ret = owplLineGetLocalUserName(hLine,
        szLocalUserName,
        &nBuffer);

    if(ret == OWPL_RESULT_SUCCESS) {
        return Py_BuildValue("s", szLocalUserName);
    }
    return Py_None;
}

/**
 * @brief Wraps owplLineSetAdapter
 */
static PyObject * PyOwplLineSetAdapter(PyObject *self, PyObject *params) {
    const OWPL_LINE hLine;
    const char * adapterName;
    void * lineConfigurationHookUserData;
    void * sipMessageFilterUserData;
    int pycode, ret;

    pycode = PyArg_ParseTuple(params,
        "isOO",
        &hLine,
        &adapterName,
        &lineConfigurationHookUserData,
        &sipMessageFilterUserData);

    if (!pycode) {
        return Py_None;
    }

    ret = owplLineSetAdapter(hLine,
        adapterName,
        lineConfigurationHookUserData,
        sipMessageFilterUserData);
    return Py_BuildValue("i", ret);
}

/**
 * @brief Wraps owplLineRegister
 */
static PyObject * PyOwplLineRegister(PyObject *self, PyObject *params) {
    const OWPL_LINE hLine;
    const int bRegister;

    int pycode, ret;

    pycode = PyArg_ParseTuple(params,
        "ii",
        &hLine,
        &bRegister);

    if (!pycode) {
        return Py_None;
    }

    ret = owplLineRegister(hLine,
        bRegister);
    return Py_BuildValue("i", ret);
}

/**
 * @brief Wraps owplLineSetOpts
 */
static PyObject * PyOwplLineSetOpts(PyObject *self, PyObject *params) {
    const OWPL_LINE hLine;
    LineOptEnum Opt;
    const void *Data;

    int pycode, ret;

    pycode = PyArg_ParseTuple(params,
        "ii",
        &hLine,
        &Opt,
        &Data);

    if (!pycode) {
        return Py_None;
    }

    ret = owplLineSetOpts(hLine,
        Opt,
        Data);
    return Py_BuildValue("i", ret);
}

/**
 * @brief Wraps owplLineGetOpts (not yet implemented)
 */
/*
static PyObject * PyOwplLineGetOpts(PyObject *self, PyObject *params) {
    const OWPL_LINE hLine;
    LineOptEnum Opt;
    const void *OutBuff = NULL; // not to be passed by Python
    const int BuffSize;

    int pycode, ret;

    pycode = PyArg_ParseTuple(params,
        "iii",
        &hLine,
        &Opt,
        &BuffSize);

    if (!pycode) {
        return Py_None;
    }

    ret = owplLineGetOpts(hLine,
        Opt,
        OutBuff,
        BuffSize);

    if(ret == OWPL_RESULT_SUCCESS) {
        return Py_BuildValue("O", OutBuff);
    }
    return Py_None;
}
*/

/**
 * @brief Wraps owplLineGetUri
 */
static PyObject * PyOwplLineGetUri(PyObject *self, PyObject *params) {
    const OWPL_LINE hLine;
    char szBuffer[MAX_URI_LENGTH]; // not to be passed by Python
    int nBuffer = sizeof(szBuffer); // not to be passed by Python

    int pycode, ret;

    pycode = PyArg_ParseTuple(params,
        "i",
        &hLine);

    if (!pycode) {
        return Py_None;
    }

    ret = owplLineGetUri(hLine,
        szBuffer,
        &nBuffer);

    if(ret == OWPL_RESULT_SUCCESS) {
        return Py_BuildValue("s", szBuffer);
    }
    return Py_None;
}

/**
 * @brief Wraps owplLineAddCredential()
 */
static PyObject * PyOwplLineAddCredential(PyObject *self, PyObject *params) {
    const OWPL_LINE hLine;
    const char* szUserID;
    const char* szPasswd;
    const char* szRealm;

    int pycode, ret;

    pycode = PyArg_ParseTuple(params,
        "isss",
        &hLine,
        &szUserID,
        &szPasswd,
        &szRealm);

    if (!pycode) {
        return Py_None;
    }

    ret = owplLineAddCredential(hLine,
        szUserID,
        szPasswd,
        szRealm);
    return Py_BuildValue("i", ret);
}

/**
 * @brief Wraps owplLineSetBusy()
 */
static PyObject * PyOwplLineSetBusy(PyObject *self, PyObject *params) {
    const OWPL_LINE hLine;
    const unsigned short bBusy;

    int pycode, ret;

    pycode = PyArg_ParseTuple(params,
        "ii",
        &hLine,
        &bBusy);

    if (!pycode) {
        return Py_None;
    }

    ret = owplLineSetBusy(hLine,
        bBusy);
    return Py_BuildValue("i", ret);
}

/**
 * @brief Wraps owplLineIsBusy()
 */
static PyObject * PyOwplLineIsBusy(PyObject *self, PyObject *params) {
    const OWPL_LINE hLine;
    unsigned int bBusy;

    int pycode, ret;

    pycode = PyArg_ParseTuple(params,
        "i",
        &hLine);

    ret = owplLineIsBusy(hLine, &bBusy);

    if(ret == OWPL_RESULT_SUCCESS) {
        return Py_BuildValue("i", bBusy);
    }
    return Py_None;
}

/**
 * @brief Wraps owplLineSetAutoKeepAlive()
 */
static PyObject * PyOwplLineSetAutoKeepAlive(PyObject *self, PyObject *params) {
    const OWPL_LINE hLine;
    unsigned int Enabled;
    unsigned int Period;

    int pycode, ret;

    pycode = PyArg_ParseTuple(params,
        "iii",
        &hLine,
        &Enabled,
        &Period);

    if (!pycode) {
        return Py_None;
    }

    ret = owplLineSetAutoKeepAlive(hLine, Enabled, Period);

    return Py_BuildValue("i", ret);
}

/**
 * @brief Wraps owplCallCreate
 */
static PyObject * PyOwplCallCreate(PyObject *self, PyObject *params) {
    const OWPL_LINE hLine;
    OWPL_CALL hCall; // not to be passed by Python

    int pycode, ret;

    pycode = PyArg_ParseTuple(params,
        "i",
        &hLine);

    if (!pycode) {
        return Py_None;
    }

    ret = owplCallCreate(hLine,
        &hCall);

    if(ret == OWPL_RESULT_SUCCESS) {
        return Py_BuildValue("i", hCall);
    }
    return Py_None;
}

/**
 * @brief Wraps owplCallConnect
 */
static PyObject * PyOwplCallConnect(PyObject *self, PyObject *params) {
    const OWPL_CALL hCall;
    const char* szAddress;
    int mediaStreams;

    int pycode, ret;

    pycode = PyArg_ParseTuple(params,
        "isi",
        &hCall,
        &szAddress,
        &mediaStreams);

    if (!pycode) {
        return Py_None;
    }

    ret = owplCallConnect(hCall,
        szAddress,
        mediaStreams);
    return Py_BuildValue("i", ret);
}

/**
 * @brief Wraps owplCallConnectWithBody
 */
static PyObject * PyOwplCallConnectWithBody(PyObject *self, PyObject *params) {
    const OWPL_CALL hCall;
    const char* szAddress;
    const char* szContentType;
    const char* szBody;
    int BodySize;

    int pycode, ret;

    pycode = PyArg_ParseTuple(params,
        "isssi",
        &hCall,
        &szAddress,
        &szContentType,
        &szBody,
        &BodySize);

    if (!pycode) {
        return Py_None;
    }

    ret = owplCallConnectWithBody(hCall,
        szAddress,
        szContentType,
        szBody,
        BodySize);
    return Py_BuildValue("i", ret);
}

/**
 * @brief Wraps owplCallAccept
 */
static PyObject * PyOwplCallAccept(PyObject *self, PyObject *params) {
    const OWPL_CALL hCall;
    int mediaStreams;

    int pycode, ret;

    pycode = PyArg_ParseTuple(params,
        "ii",
        &hCall,
        &mediaStreams);

    if (!pycode) {
        return Py_None;
    }

    ret = owplCallAccept(hCall,
        mediaStreams);
    return Py_BuildValue("i", ret);
}

/**
 * @brief Wraps owplCallAnswer
 */
static PyObject * PyOwplCallAnswer(PyObject *self, PyObject *params) {
    const OWPL_CALL hCall;
    int mediaStreams;

    int pycode, ret;

    pycode = PyArg_ParseTuple(params,
        "ii",
        &hCall,
        &mediaStreams);

    if (!pycode) {
        return Py_None;
    }

    ret = owplCallAnswer(hCall,
        mediaStreams);
    return Py_BuildValue("i", ret);
}

/**
 * @brief Wraps owplCallAnswerWithBody
 */
static PyObject * PyOwplCallAnswerWithBody(PyObject *self, PyObject *params) {
    const OWPL_CALL hCall;
    const char* szContentType;
    const char* szBody;
    int BodySize;

    int pycode, ret;

    pycode = PyArg_ParseTuple(params,
        "issi",
        &hCall,
        &szContentType,
        &szBody,
        &BodySize);

    if (!pycode) {
        return Py_None;
    }

    ret = owplCallAnswerWithBody(hCall,
        szContentType,
        szBody,
        BodySize);
    return Py_BuildValue("i", ret);
}

/**
 * @brief Wraps owplCallReject
 */
static PyObject * PyOwplCallReject(PyObject *self, PyObject *params) {
    const OWPL_CALL hCall;
    const int errorCode;
    const char* szErrorText;

    int pycode, ret;

    pycode = PyArg_ParseTuple(params,
        "iis",
        &hCall,
        &errorCode,
        &szErrorText);

    if (!pycode) {
        return Py_None;
    }

    ret = owplCallReject(hCall,
        errorCode,
        szErrorText);
    return Py_BuildValue("i", ret);
}

/**
 * @brief Wraps owplCallRejectWithPredefinedReason
 */
static PyObject * PyOwplCallRejectWithPredefinedReason(PyObject *self, PyObject *params) {
    const OWPL_CALL hCall;
    OWPL_CALL_REFUSED_REASON Reason;

    int pycode, ret;

    pycode = PyArg_ParseTuple(params,
        "ii",
        &hCall,
        &Reason);

    if (!pycode) {
        return Py_None;
    }

    ret = owplCallRejectWithPredefinedReason(hCall,
        Reason);
    return Py_BuildValue("i", ret);
}

/**
 * @brief Wraps owplCallHold
 */
static PyObject * PyOwplCallHold(PyObject *self, PyObject *params) {
    const OWPL_CALL hCall;

    int pycode, ret;

    pycode = PyArg_ParseTuple(params,
        "i",
        &hCall);

    if (!pycode) {
        return Py_None;
    }

    ret = owplCallHold(hCall);
    return Py_BuildValue("i", ret);
}

/**
 * @brief Wraps owplCallHoldWithBody
 */
static PyObject * PyOwplCallHoldWithBody(PyObject *self, PyObject *params) {
    const OWPL_CALL hCall;
    const char* szContentType;
    const char* szBody;
    int BodySize;

    int pycode, ret;

    pycode = PyArg_ParseTuple(params,
        "issi",
        &hCall,
        &szContentType,
        &szBody,
        &BodySize);

    if (!pycode) {
        return Py_None;
    }

    ret = owplCallHoldWithBody(hCall,
        szContentType,
        szBody,
        BodySize);
    return Py_BuildValue("i", ret);
}

/**
 * @brief Wraps owplCallUnhold
 */
static PyObject * PyOwplCallUnhold(PyObject *self, PyObject *params) {
    const OWPL_CALL hCall;

    int pycode, ret;

    pycode = PyArg_ParseTuple(params,
        "i",
        &hCall);

    if (!pycode) {
        return Py_None;
    }

    ret = owplCallUnhold(hCall);
    return Py_BuildValue("i", ret);
}

/**
 * @brief Wraps owplCallUnholdWithBody
 */
static PyObject * PyOwplCallUnholdWithBody(PyObject *self, PyObject *params) {
    const OWPL_CALL hCall;
    const char* szContentType;
    const char* szBody;
    int BodySize;

    int pycode, ret;

    pycode = PyArg_ParseTuple(params,
        "issi",
        &hCall,
        &szContentType,
        &szBody,
        &BodySize);

    if (!pycode) {
        return Py_None;
    }

    ret = owplCallUnholdWithBody(hCall,
        szContentType,
        szBody,
        BodySize);
    return Py_BuildValue("i", ret);
}

/**
 * @brief Wraps owplCallDisconnect
 */
static PyObject * PyOwplCallDisconnect(PyObject *self, PyObject *params) {
    const OWPL_CALL hCall;

    int pycode, ret;

    pycode = PyArg_ParseTuple(params,
        "i",
        &hCall);

    if (!pycode) {
        return Py_None;
    }

    ret = owplCallDisconnect(hCall);
    return Py_BuildValue("i", ret);
}

/**
 * @brief Wraps owplCallGetLocalID (not yet implemented)
 */
/*
static PyObject * PyOwplCallGetLocalID(PyObject *self, PyObject *params) {
    const OWPL_CALL hCall;
    char szId[MAX_URI_LENGTH]; // not to be passed by Python

    int pycode, ret;

    pycode = PyArg_ParseTuple(params,
        "i",
        &hCall);

    if (!pycode) {
        return Py_None;
    }

    ret = owplCallGetLocalID(hCall,
        szId,
        sizeof(szId));

    if(ret == OWPL_RESULT_SUCCESS) {
        return Py_BuildValue("s", szId);
    }
    return Py_None;
}
*/

/**
 * @brief Wraps owplCallGetRemoteID (not yet implemented)
 */
/*
static PyObject * PyOwplCallGetRemoteID(PyObject *self, PyObject *params) {
    const OWPL_CALL hCall;
    char szId[MAX_URI_LENGTH]; // not to be passed by Python

    int pycode, ret;

    pycode = PyArg_ParseTuple(params,
        "i",
        &hCall);

    if (!pycode) {
        return Py_None;
    }

    ret = owplCallGetRemoteID(hCall,
        szId,
        sizeof(szId));

    if(ret == OWPL_RESULT_SUCCESS) {
        return Py_BuildValue("s", szId);
    }
    return Py_None;
}
*/

/**
 * @brief Wraps owplCallSetAssertedId (not yet implemented)
 */
/*
static PyObject * PyOwplCallSetAssertedId(PyObject *self, PyObject *params) {
    const OWPL_CALL hCall;
    const char* szPAssertedId;
    const int bSignalNow;

    int pycode, ret;

    pycode = PyArg_ParseTuple(params,
        "isi",
        &hCall,
        &szPAssertedId,
        &bSignalNow);

    if (!pycode) {
        return Py_None;
    }

    ret = owplCallSetAssertedId(hCall,
        szPAssertedId,
        bSignalNow);
    return Py_BuildValue("i", ret);
}
*/

/**
 * @brief Wraps owplCallGetRemoteContact (not yet implemented)
 */
/*
static PyObject * PyOwplCallGetRemoteContact(PyObject *self, PyObject *params) {
    const OWPL_CALL hCall;
    char szContact[MAX_USERNAME_LENGTH]; // not to be passed by Python

    int pycode, ret;

    pycode = PyArg_ParseTuple(params,
        "i",
        &hCall);

    if (!pycode) {
        return Py_None;
    }

    ret = owplCallGetRemoteContact(hCall,
        szContact,
        sizeof(szContact));


    if(ret == OWPL_RESULT_SUCCESS) {
        return Py_BuildValue("s", szContact);
    }
    return Py_None;
}
*/

/**
 * @brief Wraps owplCallToneStart (not yet implemented)
 */
/*
static PyObject * PyOwplCallToneStart(PyObject *self, PyObject *params) {
    const OWPL_CALL hCall;
    const OWPL_TONE_ID toneId;
    const int bLocal;
    const int bRemote;

    int pycode, ret;

    pycode = PyArg_ParseTuple(params,
        "iiii",
        &hCall,
        &toneId,
        &bLocal,
        &bRemote);

    if (!pycode) {
        return Py_None;
    }

    ret = owplCallToneStart(hCall,
        toneId,
        bLocal,
        bRemote);
    return Py_BuildValue("i", ret);
}
*/

/**
 * @brief Wraps owplCallToneStop (not yet implemented)
 */
/*
static PyObject * PyOwplCallToneStop(PyObject *self, PyObject *params) {
    const OWPL_CALL hCall;

    int pycode, ret;

    pycode = PyArg_ParseTuple(params,
        "i",
        &hCall);

    if (!pycode) {
        return Py_None;
    }

    ret = owplCallToneStop(hCall);
    return Py_BuildValue("i", ret);
}
*/

/**
 * @brief Wraps owplCallTonePlay (not yet implemented)
 */
/*
static PyObject * PyOwplCallTonePlay(PyObject *self, PyObject *params) {
    const OWPL_CALL hCall;
    unsigned long Miliseconds;

    int pycode, ret;

    pycode = PyArg_ParseTuple(params,
        "ik",
        &hCall,
        &Miliseconds);

    if (!pycode) {
        return Py_None;
    }

    ret = owplCallTonePlay(hCall,
        Miliseconds);
    return Py_BuildValue("i", ret);
}
*/

/**
 * @brief Wraps owplCallAudioPlayFileStart (not yet implemented)
 */
/*
static PyObject * PyOwplCallAudioPlayFileStart(PyObject *self, PyObject *params) {
    const OWPL_CALL hCall;
    const char* szFile;
    const int bRepeat;
    const int bLocal;
    const int bRemote;

    int pycode, ret;

    pycode = PyArg_ParseTuple(params,
        "isiii",
        &hCall,
        &szFile,
        &bRepeat,
        &bLocal,
        &bRemote);

    if (!pycode) {
        return Py_None;
    }

    ret = owplCallAudioPlayFileStart(hCall,
        szFile,
        bRepeat,
        bLocal,
        bRemote);
    return Py_BuildValue("i", ret);
}
*/

/**
 * @brief Wraps owplCallAudioPlayFileStop (not yet implemented)
 */
/*
static PyObject * PyOwplCallAudioPlayFileStop(PyObject *self, PyObject *params) {
    const OWPL_CALL hCall;

    int pycode, ret;

    pycode = PyArg_ParseTuple(params,
        "i",
        &hCall);

    if (!pycode) {
        return Py_None;
    }

    ret = owplCallAudioPlayFileStop(hCall);
    return Py_BuildValue("i", ret);
}
*/

/**
 * @brief Wraps owplCallSendInfo  (not yet implemented)
 */
/*
static PyObject * PyOwplCallSendInfo(PyObject *self, PyObject *params) {
    const OWPL_CALL hCall;
    const char* szContentType;
    const char* szContent;
    const int nContentLength;

    int pycode, ret;

    pycode = PyArg_ParseTuple(params,
        "issi",
        &hCall,
        &szContentType,
        &szContent,
        &nContentLength);

    if (!pycode) {
        return Py_None;
    }

    ret = owplCallSendInfo(hCall,
        szContentType,
        szContent,
        nContentLength);
    return Py_BuildValue("i", ret);
}
*/

/**
 * @brief Wraps owplPresenceSubscribe
 */
static PyObject * PyOwplPresenceSubscribe(PyObject *self, PyObject *params) {
    OWPL_LINE  hLine;
    const char* szUri;
    const int winfo;
    OWPL_SUB hSub; // not to be passed by Python

    int pycode, ret;

    pycode = PyArg_ParseTuple(params,
        "isi",
        &hLine,
        &szUri,
        &winfo);

    if (!pycode) {
        return Py_None;
    }

    ret = owplPresenceSubscribe(hLine,
        szUri,
        winfo,
        &hSub);

    if(ret == OWPL_RESULT_SUCCESS) {
        return Py_BuildValue("i", hSub);
    }
    return Py_None;
}

/**
 * @brief Wraps owplPresenceUnsubscribe
 */
static PyObject * PyOwplPresenceUnsubscribe(PyObject *self, PyObject *params) {
    OWPL_SUB hSub;

    int pycode, ret;

    pycode = PyArg_ParseTuple(params,
        "i",
        &hSub);

    if (!pycode) {
        return Py_None;
    }

    ret = owplPresenceUnsubscribe(hSub);
    return Py_BuildValue("i", ret);
}

/**
 * @brief Wraps owplPresenceUnsubscribe
 */
static PyObject * PyOwplPresenceUnsubscribeFromUri(PyObject *self, PyObject *params) {
    OWPL_LINE  hLine;
    const char * szRemoteUri;

    int pycode, ret;

    pycode = PyArg_ParseTuple(params,
        "is",
        &hLine,
        &szRemoteUri);

    if (!pycode) {
        return Py_None;
    }

    ret = owplPresenceUnsubscribeFromUri(hLine, szRemoteUri);
    return Py_BuildValue("i", ret);
}



/**
 * @brief Wraps owplPresencePublish
 */
static PyObject * PyOwplPresencePublish(PyObject *self, PyObject *params) {
    OWPL_LINE  hLine;
    const int Online;
    const char * szStatus;
    OWPL_PUB hPub; // not to be passed by Python

    int pycode, ret;

    pycode = PyArg_ParseTuple(params,
        "iis",
        &hLine,
        &Online,
        &szStatus);

    if (!pycode) {
        return Py_None;
    }

    ret = owplPresencePublish(hLine,
        Online,
        szStatus,
        &hPub);

    if(ret == OWPL_RESULT_SUCCESS) {
        return Py_BuildValue("i", hPub);
    }
    return Py_None;
}

/**
 * @brief Wraps owplMessageSend
 */
static PyObject * PyOwplMessageSend(PyObject *self, PyObject *params) {
    OWPL_LINE hLine;
    const char * szRemoteUri;
    const char * szContent;
    const char * szMIME;
    int messageId; // not to be passed by Python

    int pycode, ret;

    pycode = PyArg_ParseTuple(params,
        "isss",
        &hLine,
        &szRemoteUri,
        &szContent,
        &szMIME);

    if (!pycode) {
        return Py_None;
    }

    ret = owplMessageSend(hLine,
        szRemoteUri,
        szContent,
        szMIME,
        &messageId);

    if(ret == OWPL_RESULT_SUCCESS) {
        return Py_BuildValue("i", messageId);
    }
    return Py_None;
}

/**
 * @brief Wraps owplMessageSendPlainText
 */
static PyObject * PyOwplMessageSendPlainText(PyObject *self, PyObject *params) {
    OWPL_LINE hLine;
    const char * szRemoteUri;
    const char * szContent;
    int messageId; // not to be passed by Python

    int pycode, ret;

    pycode = PyArg_ParseTuple(params,
        "iss",
        &hLine,
        &szRemoteUri,
        &szContent);

    if (!pycode) {
        return Py_None;
    }

    ret = owplMessageSendPlainText(hLine,
        szRemoteUri,
        szContent,
        &messageId);

    if(ret == OWPL_RESULT_SUCCESS) {
        return Py_BuildValue("i", messageId);
    }
    return Py_None;
}

/**
 * @brief Wraps owplMessageSendTypingState
 */
static PyObject * PyOwplMessageSendTypingState(PyObject *self, PyObject *params) {
    OWPL_LINE hLine;
    const char * szRemoteUri;
    OWPL_TYPING_STATE state;
    int messageId; // not to be passed by Python

    int pycode, ret;

    pycode = PyArg_ParseTuple(params,
        "isi",
        &hLine,
        &szRemoteUri,
        &state);

    if (!pycode) {
        return Py_None;
    }

    ret = owplMessageSendTypingState(hLine,
        szRemoteUri,
        state,
        &messageId);

    if(ret == OWPL_RESULT_SUCCESS) {
        return Py_BuildValue("i", messageId);
    }
    return Py_None;
}

/**
 * @brief Wraps owplMessageSendIcon
 */
static PyObject * PyOwplMessageSendIcon(PyObject *self, PyObject *params) {
    OWPL_LINE hLine;
    const char * szRemoteUri;
    const char * szIconFileName;
    int messageId;

    int pycode, ret;

    pycode = PyArg_ParseTuple(params,
        "iss",
        &hLine,
        &szRemoteUri,
        &szIconFileName);

    if (!pycode) {
        return Py_None;
    }

    ret = owplMessageSendIcon(hLine,
        szRemoteUri,
        szIconFileName,
        &messageId);

    if(ret == OWPL_RESULT_SUCCESS) {
        return Py_BuildValue("i", messageId);
    }
    return Py_None;
}

/*****************************************************************************
 *                                   PLUGINS API                             *
 *****************************************************************************/

#include <owpl_plugin.h>

void init_plugin_cb(const char *path);
void init_sfp_cb(void);

// SFP callbacks
static void pyphapi_callback_sfp_incomingFile(int callId, char * username,
        char * fileName, char * fileType, char * fileSize);
static PyObject *pyphapi_sfp_incomingFile = NULL;

static void pyphapi_callback_sfp_inviteToTransfer(int callId, char * username,
        char * fileName, char * fileType, char * fileSize);
static PyObject *pyphapi_sfp_inviteToTransfer = NULL;

static void pyphapi_callback_sfp_waitingForAnswer(int cid, char * uri);
static PyObject *pyphapi_sfp_waitingForAnswer = NULL;

static void pyphapi_callback_sfp_canceled(int cid, char * short_filename,
        char * file_type, char * file_size);
static PyObject *pyphapi_sfp_canceled = NULL;

static void pyphapi_callback_sfp_canceledByPeer(int cid, char * username,
        char * short_filename, char * file_type, char * file_size);
static PyObject *pyphapi_sfp_canceledByPeer = NULL;

static void pyphapi_callback_sfp_sendingFileBegin(int cid, char * username,
        char * short_filename, char * file_type, char * file_size);
static PyObject *pyphapi_sfp_sendingFileBegin = NULL;

static void pyphapi_callback_receivingFileBegin(int cid, char * username,
        char * short_filename, char * file_type, char * file_size);
static PyObject *pyphapi_sfp_receivingFileBegin = NULL;

static void pyphapi_callback_transferFromPeerFinished(int cid, char * username,
        char * short_filename, char * file_type, char * file_size);
static PyObject *pyphapi_sfp_transferFromPeerFinished = NULL;

static void pyphapi_callback_transferToPeerFinished(int cid, char * username,
        char * short_filename, char * file_type, char * file_size);
static PyObject *pyphapi_sfp_transferToPeerFinished = NULL;

static void pyphapi_callback_transferFromPeerFailed(int cid, char * username,
        char * short_filename, char * file_type, char * file_size);
static PyObject *pyphapi_sfp_transferFromPeerFailed = NULL;

static void pyphapi_callback_transferToPeerFailed(int cid, char * username,
        char * short_filename, char * file_type, char * file_size);
static PyObject *pyphapi_sfp_transferToPeerFailed = NULL;

static void pyphapi_callback_transferFromPeerStopped(int cid, char * username,
        char * short_filename, char * file_type, char * file_size);
static PyObject *pyphapi_sfp_transferFromPeerStopped = NULL;

static void pyphapi_callback_transferToPeerStopped(int cid, char * username,
        char * short_filename, char * file_type, char * file_size);
static PyObject *pyphapi_sfp_transferToPeerStopped = NULL;

static void pyphapi_callback_transferProgression(int cid, int percentage);
static PyObject *pyphapi_sfp_transferProgression = NULL;

static void pyphapi_callback_transferPausedByPeer(int cid, char * username,
        char * short_filename, char * file_type, char * file_size);
static PyObject *pyphapi_sfp_transferPausedByPeer = NULL;

static void pyphapi_callback_transferPaused(int cid, char * username,
        char * short_filename, char * file_type, char * file_size);
static PyObject *pyphapi_sfp_transferPaused = NULL;

static void pyphapi_callback_transferResumedByPeer(int cid, char * username,
        char * short_filename, char * file_type, char * file_size);
static PyObject *pyphapi_sfp_transferResumedByPeer = NULL;

static void pyphapi_callback_transferResumed(int cid, char * username,
        char * short_filename, char * file_type, char * file_size);
static PyObject *pyphapi_sfp_transferResumed = NULL;

static void pyphapi_callback_peerNeedsUpgrade(const char * username);
static PyObject *pyphapi_sfp_peerNeedsUpgrade = NULL;

static void pyphapi_callback_needUpgrade(void);
static PyObject *pyphapi_sfp_needUpgrade = NULL;
//// SFP callbacks


/**
 * @brief Wraps owplPluginLoad
 */
static PyObject * PyOwplPluginLoad(PyObject *self, PyObject *params) {
    const char * path;

    int pycode, ret;

    pycode = PyArg_ParseTuple(params,
        "s",
        &path);

    if (!pycode) {
        return Py_None;
    }

    ret = owplPluginLoad(path);

    init_plugin_cb(path);

    return Py_BuildValue("i", ret);

}

/**
 * @brief init plugins callback
 */
void init_plugin_cb(const char *path){
    char *str = "sfp-plugin", *ptr;

    ptr = strstr(path, str);
    if (ptr) {
        init_sfp_cb();
    }
}


/**
 * @brief Wraps owplPluginSetParam
 */
static PyObject * PyOwplPluginSetParam(PyObject *self, PyObject *params) {
    const char *newValue;
    int valSize;
    const char *pluginName;
    const char *paramName;

    int pycode, ret = 0;

    pycode = PyArg_ParseTuple(params,
        "siss",
        &newValue,
        &valSize,
        &pluginName,
        &paramName);

    if (!pycode) {
        return Py_None;
    }

    ret = owplPluginSetParam(newValue, valSize, pluginName, paramName);
    return Py_BuildValue("i", ret);
}

/**
 * @brief "Wraps" owplPluginSetCallback
 */
static PyObject * PyOwplPluginSetCallback(PyObject *self, PyObject *params) {
    const char * PluginName;
    const char * CallbackName;
    PyObject * CallbackProc;

    int pycode, ret = 0;

    pycode = PyArg_ParseTuple(params,
        "ssO",
        &PluginName,
        &CallbackName,
        &CallbackProc);

    if (!pycode) {
        return Py_None;
    }

    if ((strcmp(CallbackName, "newIncomingFile")) == 0) {
      Py_XINCREF(CallbackProc);
      Py_XDECREF(pyphapi_sfp_incomingFile);
      pyphapi_sfp_incomingFile = CallbackProc;
    } else if ((strcmp(CallbackName, "inviteToTransfer")) == 0) {
        Py_XINCREF(CallbackProc);
        Py_XDECREF(pyphapi_sfp_inviteToTransfer);
        pyphapi_sfp_inviteToTransfer = CallbackProc;
    } else if ((strcmp(CallbackName, "waitingForAnswer")) == 0) {
        Py_XINCREF(CallbackProc);
        Py_XDECREF(pyphapi_sfp_waitingForAnswer);
        pyphapi_sfp_waitingForAnswer = CallbackProc;
    } else if ((strcmp(CallbackName, "transferCancelled")) == 0) {
        Py_XINCREF(CallbackProc);
        Py_XDECREF(pyphapi_sfp_canceled);
        pyphapi_sfp_canceled = CallbackProc;
    } else if ((strcmp(CallbackName, "transferCancelledByPeer")) == 0) {
        Py_XINCREF(CallbackProc);
        Py_XDECREF(pyphapi_sfp_canceledByPeer);
        pyphapi_sfp_canceledByPeer = CallbackProc;
    } else if ((strcmp(CallbackName, "sendingFileBegin")) == 0) {
        Py_XINCREF(CallbackProc);
        Py_XDECREF(pyphapi_sfp_sendingFileBegin);
        pyphapi_sfp_sendingFileBegin = CallbackProc;
    } else if ((strcmp(CallbackName, "receivingFileBegin")) == 0) {
        Py_XINCREF(CallbackProc);
        Py_XDECREF(pyphapi_sfp_receivingFileBegin);
        pyphapi_sfp_receivingFileBegin = CallbackProc;
    } else if ((strcmp(CallbackName, "transferFromPeerFinished")) == 0) {
        Py_XINCREF(CallbackProc);
        Py_XDECREF(pyphapi_sfp_transferFromPeerFinished);
        pyphapi_sfp_transferFromPeerFinished = CallbackProc;
    } else if ((strcmp(CallbackName, "transferToPeerFinished")) == 0) {
        Py_XINCREF(CallbackProc);
        Py_XDECREF(pyphapi_sfp_transferToPeerFinished);
        pyphapi_sfp_transferToPeerFinished = CallbackProc;
    } else if ((strcmp(CallbackName, "transferFromPeerFailed")) == 0) {
        Py_XINCREF(CallbackProc);
        Py_XDECREF(pyphapi_sfp_transferFromPeerFailed);
        pyphapi_sfp_transferFromPeerFailed = CallbackProc;
    } else if ((strcmp(CallbackName, "transferToPeerFailed")) == 0) {
        Py_XINCREF(CallbackProc);
        Py_XDECREF(pyphapi_sfp_transferToPeerFailed);
        pyphapi_sfp_transferToPeerFailed = CallbackProc;
    } else if ((strcmp(CallbackName, "transferFromPeerStopped")) == 0) {
        Py_XINCREF(CallbackProc);
        Py_XDECREF(pyphapi_sfp_transferFromPeerStopped);
        pyphapi_sfp_transferFromPeerStopped = CallbackProc;
    } else if ((strcmp(CallbackName, "transferToPeerStopped")) == 0) {
        Py_XINCREF(CallbackProc);
        Py_XDECREF(pyphapi_sfp_transferToPeerStopped);
        pyphapi_sfp_transferToPeerStopped = CallbackProc;
    } else if ((strcmp(CallbackName, "transferProgression")) == 0) {
        Py_XINCREF(CallbackProc);
        Py_XDECREF(pyphapi_sfp_transferProgression);
        pyphapi_sfp_transferProgression = CallbackProc;
    } else if ((strcmp(CallbackName, "transferPausedByPeer")) == 0) {
        Py_XINCREF(CallbackProc);
        Py_XDECREF(pyphapi_sfp_transferPausedByPeer);
        pyphapi_sfp_transferPausedByPeer = CallbackProc;
    } else if ((strcmp(CallbackName, "transferPaused")) == 0) {
        Py_XINCREF(CallbackProc);
        Py_XDECREF(pyphapi_sfp_transferPaused);
        pyphapi_sfp_transferPaused = CallbackProc;
    } else if ((strcmp(CallbackName, "transferResumedByPeer")) == 0) {
        Py_XINCREF(CallbackProc);
        Py_XDECREF(pyphapi_sfp_transferResumedByPeer);
        pyphapi_sfp_transferResumedByPeer = CallbackProc;
    } else if ((strcmp(CallbackName, "transferResumed")) == 0) {
        Py_XINCREF(CallbackProc);
        Py_XDECREF(pyphapi_sfp_transferResumed);
        pyphapi_sfp_transferResumed = CallbackProc;
    } else if ((strcmp(CallbackName, "peerNeedUpgrade")) == 0) {
        Py_XINCREF(CallbackProc);
        Py_XDECREF(pyphapi_sfp_peerNeedsUpgrade);
        pyphapi_sfp_peerNeedsUpgrade = CallbackProc;
    } else if ((strcmp(CallbackName, "needUpgrade")) == 0) {
        Py_XINCREF(CallbackProc);
        Py_XDECREF(pyphapi_sfp_needUpgrade);
        pyphapi_sfp_needUpgrade = CallbackProc;
    }

    return Py_BuildValue("i", ret);
}

/*****************************************************************************
 *                                   SFP                                     *
 *****************************************************************************/

/**
 * init SFP callback
 */
void init_sfp_cb(void) {

    owplPluginSetCallback(
        "SFPPlugin",
        "newIncomingFile",
        (owplPS_CommandProc)pyphapi_callback_sfp_incomingFile
    );
    owplPluginSetCallback(
        "SFPPlugin",
        "inviteToTransfer",
        (owplPS_CommandProc)pyphapi_callback_sfp_inviteToTransfer
    );
    owplPluginSetCallback(
        "SFPPlugin",
        "waitingForAnswer",
        (owplPS_CommandProc)pyphapi_callback_sfp_waitingForAnswer
    );
    owplPluginSetCallback(
        "SFPPlugin",
        "transferCancelled",
        (owplPS_CommandProc)pyphapi_callback_sfp_canceled
    );
    owplPluginSetCallback(
        "SFPPlugin",
        "transferCancelledByPeer",
        (owplPS_CommandProc)pyphapi_callback_sfp_canceledByPeer
    );
    owplPluginSetCallback(
        "SFPPlugin",
        "sendingFileBegin",
        (owplPS_CommandProc)pyphapi_callback_sfp_sendingFileBegin
    );
    owplPluginSetCallback(
        "SFPPlugin",
        "receivingFileBegin",
        (owplPS_CommandProc)pyphapi_callback_receivingFileBegin
    );
    owplPluginSetCallback(
        "SFPPlugin",
        "transferFromPeerFinished",
        (owplPS_CommandProc)pyphapi_callback_transferFromPeerFinished
    );
    owplPluginSetCallback(
        "SFPPlugin",
        "transferToPeerFinished",
        (owplPS_CommandProc)pyphapi_callback_transferToPeerFinished
    );
    owplPluginSetCallback(
        "SFPPlugin",
        "transferFromPeerFailed",
        (owplPS_CommandProc)pyphapi_callback_transferFromPeerFailed
    );
    owplPluginSetCallback(
        "SFPPlugin",
        "transferFromPeerFailed",
        (owplPS_CommandProc)pyphapi_callback_transferToPeerFailed
    );
    owplPluginSetCallback(
        "SFPPlugin",
        "transferFromPeerStopped",
        (owplPS_CommandProc)pyphapi_callback_transferFromPeerStopped
    );
    owplPluginSetCallback(
        "SFPPlugin",
        "transferToPeerStopped",
        (owplPS_CommandProc)pyphapi_callback_transferToPeerStopped
    );
    owplPluginSetCallback(
        "SFPPlugin",
        "transferProgression",
        (owplPS_CommandProc)pyphapi_callback_transferProgression
    );
    owplPluginSetCallback(
        "SFPPlugin",
        "transferPausedByPeer",
        (owplPS_CommandProc)pyphapi_callback_transferPausedByPeer
    );
    owplPluginSetCallback(
        "SFPPlugin",
        "transferPaused",
        (owplPS_CommandProc)pyphapi_callback_transferPaused
    );
    owplPluginSetCallback(
        "SFPPlugin",
        "transferResumedByPeer",
        (owplPS_CommandProc)pyphapi_callback_transferResumedByPeer
    );
    owplPluginSetCallback(
        "SFPPlugin",
        "transferResumed",
        (owplPS_CommandProc)pyphapi_callback_transferResumed
    );
    owplPluginSetCallback(
        "SFPPlugin",
        "peerNeedUpgrade",
        (owplPS_CommandProc)pyphapi_callback_peerNeedsUpgrade
    );
    owplPluginSetCallback(
        "SFPPlugin",
        "needUpgrade",
        (owplPS_CommandProc)pyphapi_callback_needUpgrade
    );
}

/**
 * C callback for newIncomingFile Event
 */
static void pyphapi_callback_sfp_incomingFile(int callId, char * username,
        char * fileName, char * fileType, char * fileSize) {

    PyObject * cInfo;

    if (pyphapi_sfp_incomingFile) {
        cInfo = Py_BuildValue("(issss)",
            callId,
            username,
            fileName,
            fileType,
            fileSize);
        pyphapi_lock_and_call(pyphapi_sfp_incomingFile, cInfo);
    }
}

/**
 * C callback for inviteToTransfer Event
 */
static void pyphapi_callback_sfp_inviteToTransfer(int callId, char * username,
        char * fileName, char * fileType, char * fileSize) {
    PyObject * cInfo;

    if (pyphapi_sfp_inviteToTransfer) {
        cInfo = Py_BuildValue("(issss)",
            callId,
            username,
            fileName,
            fileType,
            fileSize);
        pyphapi_lock_and_call(pyphapi_sfp_inviteToTransfer, cInfo);
    }
}

/**
 * C callback for waitingForAnswer Event
 */
static void pyphapi_callback_sfp_waitingForAnswer(int cid, char * uri) {
    PyObject * cInfo;

    if (pyphapi_sfp_inviteToTransfer) {
        cInfo = Py_BuildValue("(is)",
            cid,
            uri);
        pyphapi_lock_and_call(pyphapi_sfp_waitingForAnswer, cInfo);
    }
}

/**
 * C callback for transferCancelled Event
 */
static void pyphapi_callback_sfp_canceled(int cid, char * short_filename,
        char * file_type, char * file_size) {
    PyObject * cInfo;

    if (pyphapi_sfp_inviteToTransfer) {
        cInfo = Py_BuildValue("(isss)",
            cid,
            short_filename,
            file_type,
            file_size);
        pyphapi_lock_and_call(pyphapi_sfp_canceled, cInfo);
    }
}


/**
 * C callback for transferCancelledByPeer Event
 */
static void pyphapi_callback_sfp_canceledByPeer(int cid, char * username,
        char * short_filename, char * file_type, char * file_size) {
    PyObject * cInfo;

    if (pyphapi_sfp_inviteToTransfer) {
        cInfo = Py_BuildValue("(issss)",
            cid,
            username,
            short_filename,
            file_type,
            file_size);
        pyphapi_lock_and_call(pyphapi_sfp_canceledByPeer, cInfo);
    }
}

/**
 * C callback for sendingFileBegin Event
 */
static void pyphapi_callback_sfp_sendingFileBegin(int cid, char * username,
        char * short_filename, char * file_type, char * file_size) {
    PyObject * cInfo;

    if (pyphapi_sfp_inviteToTransfer) {
        cInfo = Py_BuildValue("(issss)",
            cid,
            username,
            short_filename,
            file_type,
            file_size);
        pyphapi_lock_and_call(pyphapi_sfp_sendingFileBegin, cInfo);
    }
}

/**
 * C callback for receivingFileBegin Event
 */
static void pyphapi_callback_receivingFileBegin(int cid, char * username,
        char * short_filename, char * file_type, char * file_size) {
    PyObject * cInfo;

    if (pyphapi_sfp_inviteToTransfer) {
        cInfo = Py_BuildValue("(issss)",
            cid,
            username,
            short_filename,
            file_type,
            file_size);
        pyphapi_lock_and_call(pyphapi_sfp_receivingFileBegin, cInfo);
    }
}

/**
 * C callback for transferFromPeerFinished Event
 */
static void pyphapi_callback_transferFromPeerFinished(int cid, char * username,
        char * short_filename, char * file_type, char * file_size) {
    PyObject * cInfo;

    if (pyphapi_sfp_inviteToTransfer) {
        cInfo = Py_BuildValue("(issss)",
            cid,
            username,
            short_filename,
            file_type,
            file_size);
        pyphapi_lock_and_call(pyphapi_sfp_transferFromPeerFinished, cInfo);
    }
}

/**
 * C callback for transferToPeerFinished Event
 */
static void pyphapi_callback_transferToPeerFinished(int cid, char * username,
        char * short_filename, char * file_type, char * file_size) {
    PyObject * cInfo;

    if (pyphapi_sfp_inviteToTransfer) {
        cInfo = Py_BuildValue("(issss)",
            cid,
            username,
            short_filename,
            file_type,
            file_size);
        pyphapi_lock_and_call(pyphapi_sfp_transferToPeerFinished, cInfo);
    }
}

/**
 * C callback for transferFromPeerFailed Event
 */
static void pyphapi_callback_transferFromPeerFailed(int cid, char * username,
        char * short_filename, char * file_type, char * file_size) {
    PyObject * cInfo;

    if (pyphapi_sfp_inviteToTransfer) {
        cInfo = Py_BuildValue("(issss)",
            cid,
            username,
            short_filename,
            file_type,
            file_size);
        pyphapi_lock_and_call(pyphapi_sfp_transferFromPeerFailed, cInfo);
    }
}

/**
 * C callback for transferToPeerFailed Event
 */
static void pyphapi_callback_transferToPeerFailed(int cid, char * username,
        char * short_filename, char * file_type, char * file_size) {
    PyObject * cInfo;

    if (pyphapi_sfp_inviteToTransfer) {
        cInfo = Py_BuildValue("(issss)",
            cid,
            username,
            short_filename,
            file_type,
            file_size);
        pyphapi_lock_and_call(pyphapi_sfp_transferToPeerFailed, cInfo);
    }
}

/**
 * C callback for transferFromPeerStopped Event
 */
static void pyphapi_callback_transferFromPeerStopped(int cid, char * username,
        char * short_filename, char * file_type, char * file_size) {
    PyObject * cInfo;

    if (pyphapi_sfp_inviteToTransfer) {
        cInfo = Py_BuildValue("(issss)",
            cid,
            username,
            short_filename,
            file_type,
            file_size);
        pyphapi_lock_and_call(pyphapi_sfp_transferFromPeerStopped, cInfo);
    }
}

/**
 * C callback for transferToPeerStopped Event
 */
static void pyphapi_callback_transferToPeerStopped(int cid, char * username,
        char * short_filename, char * file_type, char * file_size) {
    PyObject * cInfo;

    if (pyphapi_sfp_inviteToTransfer) {
        cInfo = Py_BuildValue("(issss)",
            cid,
            username,
            short_filename,
            file_type,
            file_size);
        pyphapi_lock_and_call(pyphapi_sfp_transferToPeerStopped, cInfo);
    }
}

/**
 * C callback for transferProgression Event
 */
static void pyphapi_callback_transferProgression(int cid, int percentage) {
    PyObject * cInfo;

    if (pyphapi_sfp_inviteToTransfer) {
        cInfo = Py_BuildValue("(ii)",
            cid,
            percentage);
        pyphapi_lock_and_call(pyphapi_sfp_transferProgression, cInfo);
    }
}

/**
 * C callback for transferPausedByPeer Event
 */
static void pyphapi_callback_transferPausedByPeer(int cid, char * username,
        char * short_filename, char * file_type, char * file_size) {
    PyObject * cInfo;

    if (pyphapi_sfp_inviteToTransfer) {
        cInfo = Py_BuildValue("(issss)",
            cid,
            username,
            short_filename,
            file_type,
            file_size);
        pyphapi_lock_and_call(pyphapi_sfp_transferPausedByPeer, cInfo);
    }
}

/**
 * C callback for transferPaused Event
 */
static void pyphapi_callback_transferPaused(int cid, char * username,
        char * short_filename, char * file_type, char * file_size) {
    PyObject * cInfo;

    if (pyphapi_sfp_inviteToTransfer) {
        cInfo = Py_BuildValue("(issss)",
            cid,
            username,
            short_filename,
            file_type,
            file_size);
        pyphapi_lock_and_call(pyphapi_sfp_transferPaused, cInfo);
    }
}

/**
 * C callback for transferResumedByPeer Event
 */
static void pyphapi_callback_transferResumedByPeer(int cid, char * username,
        char * short_filename, char * file_type, char * file_size) {
    PyObject * cInfo;

    if (pyphapi_sfp_inviteToTransfer) {
        cInfo = Py_BuildValue("(issss)",
            cid,
            username,
            short_filename,
            file_type,
            file_size);
        pyphapi_lock_and_call(pyphapi_sfp_transferResumedByPeer, cInfo);
    }
}

/**
 * C callback for transferResumed Event
 */
static void pyphapi_callback_transferResumed(int cid, char * username,
        char * short_filename, char * file_type, char * file_size) {
    PyObject * cInfo;

    if (pyphapi_sfp_inviteToTransfer) {
        cInfo = Py_BuildValue("(issss)",
            cid,
            username,
            short_filename,
            file_type,
            file_size);
        pyphapi_lock_and_call(pyphapi_sfp_transferResumed, cInfo);
    }
}

/**
 * C callback for peerNeedsUpgrade Event
 */
static void pyphapi_callback_peerNeedsUpgrade(const char * username) {
    PyObject * cInfo;

    if (pyphapi_sfp_inviteToTransfer) {
        cInfo = Py_BuildValue("(s)",
            username);
        pyphapi_lock_and_call(pyphapi_sfp_peerNeedsUpgrade, cInfo);
    }
}

/**
 * C callback for needUpgrade Event
 */
static void pyphapi_callback_needUpgrade(void) {
    pyphapi_lock_and_call(pyphapi_sfp_needUpgrade, Py_None);
}

/**
 * @brief Wraps PhApiSFPWrapper::sendFile
 */
static PyObject * PyOwplsendFile(PyObject *self, PyObject *params) {
    int lineId;
    const char * contactUri;
    const char * filename;
    const char * shortFilename;
    const char * fileType;
    const char * filesize;

    int pycode, ret;

    pycode = PyArg_ParseTuple(params,
        "isssss",
        &lineId,
        &contactUri,
        &filename,
        &shortFilename,
        &fileType,
        &filesize);

    if (!pycode) {
        return Py_None;
    }

    owplPluginCallFunction(&ret,
                           "SFPPlugin",
                           "sfp_send_file",
                           "%d%s%s%s%s%s",
                           lineId,
                           contactUri,
                           filename,
                           shortFilename,
                           fileType,
                           filesize
                          );

    return Py_BuildValue("i", ret);
}

/**
 * @brief Wraps PhApiSFPWrapper::receiveFile
 */
static PyObject * PyOwplreceiveFile(PyObject *self, PyObject *params) {
    int callId;
    const char * filename;

    int pycode, ret, retVal;

    pycode = PyArg_ParseTuple(params,
        "is",
        &callId,
        &filename);

    if (!pycode) {
        return Py_None;
    }

    ret = owplPluginCallFunction(&retVal,
                                 "SFPPlugin",
                                 "sfp_receive_file",
                                 "%d%s",
                                 callId,
                                 filename
                                );

    if (ret == OWPL_RESULT_SUCCESS) {
        return Py_BuildValue("i", ret);
    }
    return Py_None;
}


/**
 * @brief Wraps PhApiSFPWrapper::cancelTransfer
 */
static PyObject * PyOwplcancelTransfer(PyObject *self, PyObject *params) {
    int callId;

    int pycode, ret, retVal;

    pycode = PyArg_ParseTuple(params,
        "i",
        &callId);

    if (!pycode) {
        return Py_None;
    }

    ret = owplPluginCallFunction(&retVal,
                                 "SFPPlugin",
                                 "sfp_cancel_transfer",
                                 "%d",
                                 callId
                                );

    if(ret == OWPL_RESULT_SUCCESS) {
        return Py_BuildValue("i", ret);
    }
    return Py_None;
}

/**
 * @brief Wraps PhApiSFPWrapper::pauseTransfer
 */
static PyObject * PyOwplpauseTransfer(PyObject *self, PyObject *params) {
    int callId;

    int pycode, ret, retVal;

    pycode = PyArg_ParseTuple(params,
        "i",
        &callId);

    if (!pycode) {
        return Py_None;
    }

    ret = owplPluginCallFunction(&retVal,
                                 "SFPPlugin",
                                 "sfp_pause_transfer",
                                 "%d",
                                 callId
                                );

    if(ret == OWPL_RESULT_SUCCESS) {
        return Py_BuildValue("i", ret);
    }
    return Py_None;
}

/**
 * @brief Wraps PhApiSFPWrapper::resumeTransfer
 */
static PyObject * PyOwplresumeTransfer(PyObject *self, PyObject *params) {
    int callId;

    int pycode, ret, retVal;

    pycode = PyArg_ParseTuple(params,
        "i",
        &callId);

    if (!pycode) {
        return Py_None;
    }

    ret = owplPluginCallFunction(&retVal,
                                 "SFPPlugin",
                                 "sfp_resume_transfer",
                                 "%d",
                                 callId
                                );

    if(ret == OWPL_RESULT_SUCCESS) {
        return Py_BuildValue("i", ret);
    }
    return Py_None;
}

/*****************************************************************************
 *                                  NORTEL                                   *
 *****************************************************************************/

#include <nortel.h>

/**
 * @brief Wraps owplAdapterNortelInitialize
 */
static PyObject * PyOwplAdapterNortelInitialize(PyObject *self, PyObject *params) {
    const char * adapterName;
    int pycode, ret;

    pycode = PyArg_ParseTuple(params,
        "s",
        &adapterName);

    if (!pycode) {
        return Py_None;
    }

    ret = owplAdapterNortelInitialize(adapterName);
    return Py_BuildValue("i", ret);
}

/*****************************************************************************
 *                                   WLM                                     *
 *****************************************************************************/

#include <wlm.h>

/**
 * @brief Wraps owplAdapterWlmInitialize
 */
static PyObject * PyOwplAdapterWlmInitialize(PyObject *self, PyObject *params) {
    const char * adapterName;
    int pycode, ret;

    pycode = PyArg_ParseTuple(params,
        "s",
        &adapterName);

    if (!pycode) {
        return Py_None;
    }

    ret = owplAdapterWlmInitialize(adapterName);
    return Py_BuildValue("i", ret);
}

/*****************************************************************************
 *                                   OWSL                                    *
 *****************************************************************************/

/**
 * @brief Wraps owsl_initialize
 */
// static PyObject * PyOwsl_initialize(PyObject *self, PyObject *params) {
//     int ret = owsl_initialize();
// 
//     return Py_BuildValue("i", ret);
// }

/**
 * @brief Wraps owsl_terminate
 */
// static PyObject * PyOwsl_terminate(PyObject *self, PyObject *params) {
//     int ret = owsl_terminate();
// 
//     return Py_BuildValue("i", ret);
// }

/**
 * @brief Wraps owsl_socket_type_add
 */
// static PyObject * PyOwsl_socket_type_add(PyObject *self, PyObject *params) {
//     int type;
// 
//     int pycode, ret;
// 
//     pycode = PyArg_ParseTuple(params,
//         "i",
//         &type);
// 
//     if (!pycode) {
//         return Py_None;
//     }
// 
//     ret = owsl_socket_type_add(type);
// 
//     return Py_BuildValue("i", ret);
// }

/**
 * @brief Wraps owsl_socket_type_remove
 */
// static PyObject * PyOwsl_socket_type_remove(PyObject *self, PyObject *params) {
//     int type;
//
//     int pycode, ret;
//
//     pycode = PyArg_ParseTuple(params,
//         "i",
//         &type);
//
//     if (!pycode) {
//         return Py_None;
//     }
//
//     ret = owsl_socket_type_remove(type);
//
//     return Py_BuildValue("i", ret);
// }


/*****************************************************************************
 *                                   HELPERS                                 *
 *****************************************************************************/

/**
 * @brief wrap access to phcfg
 */
static PyObject * PyPhCfgSetS(PyObject *self, PyObject *params) {
    char * field;
    char * value_string;
    phConfig_t *cfg;

    cfg = phGetConfig();

    if (PyArg_ParseTuple(params, "ss", &field, &value_string)) {
        if (strcmp(field, "local_rtp_port")==0) {strncpy(cfg->local_rtp_port, value_string,16);}
        if (strcmp(field, "local_audio_rtcp_port")==0) {strncpy(cfg->local_audio_rtcp_port, value_string,16);}
        if (strcmp(field, "local_video_rtp_port")==0) {strncpy(cfg->local_video_rtp_port, value_string,16);}
        if (strcmp(field, "local_video_rtcp_port")==0) {strncpy(cfg->local_video_rtcp_port, value_string,16);}
        //if (strcmp(field, "sipport")==0) {strncpy(cfg->sipport, value_string,16);}
        //if (strcmp(field, "nattype")==0) {strncpy(cfg->nattype, value_string,16);}
        if (strcmp(field, "audio_codecs")==0) {strncpy(cfg->audio_codecs, value_string,128);}
        if (strcmp(field, "video_codecs")==0) {strncpy(cfg->video_codecs, value_string,128);}
        if (strcmp(field, "audio_dev")==0) {strncpy(cfg->audio_dev, value_string,64);}
        //if (strcmp(field, "stunserver")==0) {strncpy(cfg->stunserver, value_string,128);}
        //if (strcmp(field, "httpt_server")==0) {strncpy(cfg->httpt_server, value_string,128);}
        //if (strcmp(field, "http_proxy")==0) {strncpy(cfg->http_proxy, value_string,128);}
        //if (strcmp(field, "http_proxy_user")==0) {strncpy(cfg->http_proxy_user, value_string,128);}
        //if (strcmp(field, "http_proxy_passwd")==0) {strncpy(cfg->http_proxy_passwd, value_string,128);}
        if (strcmp(field, "plugin_path")==0) {strncpy(cfg->plugin_path, value_string,256);}
    }
    return Py_None;
}


/**
 * @brief wrap access to phcfg
 */
static PyObject * PyPhCfgSetI(PyObject *self, PyObject *params) {
    char * field;
    int value_int;
    phConfig_t *cfg;

    cfg = phGetConfig();

    if (PyArg_ParseTuple(params, "si", &field, &value_int)) {
        if (strcmp(field, "asyncmode") == 0) {cfg->asyncmode=value_int;}
        if (strcmp(field, "nomedia") == 0) {cfg->nomedia=value_int;}
        if (strcmp(field, "noaec") == 0) {cfg->noaec=value_int;}
        if (strcmp(field, "vad") == 0) {cfg->vad=value_int;}
        if (strcmp(field, "cng") == 0) {cfg->cng=value_int;}
        if (strcmp(field, "hdxmode") == 0) {cfg->hdxmode=value_int;}
        if (strcmp(field, "nat_refresh_time") == 0) {cfg->nat_refresh_time=value_int;}
        if (strcmp(field, "jitterdepth") == 0) {cfg->jitterdepth=value_int;}
        if (strcmp(field, "autoredir") == 0) {cfg->autoredir=value_int;}
        //if( strcmp(field, "use_tunnel" ) == 0) {cfg->use_tunnel=value_int;}
        //if( strcmp(field, "httpt_server_port" ) == 0) {cfg->httpt_server_port=value_int;}
        //if( strcmp(field, "http_proxy_port" ) == 0) {cfg->http_proxy_port=value_int;}
    }
    return Py_None;
}

/*****************************************************************************
 *                         PYTHON MODULE DECLARATION                         *
 *****************************************************************************/

/*
 * Convenient define to declare a function with variable arguments in
 * the module's API table
 *
 */
#define PY_PHAPI_FUNCTION_DECL(n,f) \
    { n, (PyCFunction) f, METH_VARARGS, "" }

#define PY_PHAPI_FUNCTION_DECL_NULL \
    { NULL, NULL, 0, NULL }

/**
 * @brief Declaration of the module API table
 */
static PyMethodDef pyphapi_funcs[] = {
    /*{ "pyphapi",(PyCFunction) pyphapi, METH_NOARGS,  "Python Module of phApi"},*/

    PY_PHAPI_FUNCTION_DECL("owplInit",                              PyOwplInit),
    PY_PHAPI_FUNCTION_DECL("owplShutdown",                          PyOwplShutdown),
    PY_PHAPI_FUNCTION_DECL("owplEventListenerAdd",                  PyOwplEventListenerAdd),
    PY_PHAPI_FUNCTION_DECL("owplConfigSetLocalHttpProxy",           PyOwplConfigSetLocalHttpProxy),
    PY_PHAPI_FUNCTION_DECL("owplConfigAddAudioCodecByName",         PyOwplConfigAddAudioCodecByName),
    PY_PHAPI_FUNCTION_DECL("owplConfigSetAudioCodecs",              PyOwplConfigSetAudioCodecs),
    PY_PHAPI_FUNCTION_DECL("owplConfigAddVideoCodecByName",         PyOwplConfigAddVideoCodecByName),
    PY_PHAPI_FUNCTION_DECL("owplConfigSetVideoCodecs",              PyOwplConfigSetVideoCodecs),
    PY_PHAPI_FUNCTION_DECL("owplConfigGetBoundLocalAddr",           PyOwplConfigGetBoundLocalAddr),
    PY_PHAPI_FUNCTION_DECL("owplConfigLocalHttpProxyGetAddr",       PyOwplConfigLocalHttpProxyGetAddr),
    PY_PHAPI_FUNCTION_DECL("owplConfigLocalHttpProxyGetPasswd",     PyOwplConfigLocalHttpProxyGetPasswd),
    PY_PHAPI_FUNCTION_DECL("owplConfigLocalHttpProxyGetPort",       PyOwplConfigLocalHttpProxyGetPort),
    PY_PHAPI_FUNCTION_DECL("owplConfigLocalHttpProxyGetUserName",   PyOwplConfigLocalHttpProxyGetUserName),
    PY_PHAPI_FUNCTION_DECL("owplConfigSetHttpTunnel",               PyOwplConfigSetHttpTunnel),
    PY_PHAPI_FUNCTION_DECL("owplConfigEnableHttpTunnel",            PyOwplConfigEnableHttpTunnel),

    PY_PHAPI_FUNCTION_DECL("owplAudioSetConfigString",              PyOwplAudioSetConfigString),
    PY_PHAPI_FUNCTION_DECL("owplLineAdd",                           PyOwplLineAdd),
    PY_PHAPI_FUNCTION_DECL("owplLineDelete",                        PyOwplLineDelete),
    PY_PHAPI_FUNCTION_DECL("owplLineGetProxy",                      PyOwplLineGetProxy),
    PY_PHAPI_FUNCTION_DECL("owplLineGetLocalUserName",              PyOwplLineGetLocalUserName),
    PY_PHAPI_FUNCTION_DECL("owplLineSetAdapter",                    PyOwplLineSetAdapter),
    PY_PHAPI_FUNCTION_DECL("owplLineRegister",                      PyOwplLineRegister),
    PY_PHAPI_FUNCTION_DECL("owplLineSetOpts",                       PyOwplLineSetOpts),
    /*PY_PHAPI_FUNCTION_DECL("owplLineGetOpts",                     PyOwplLineGetOpts), // not yet implemented */
    PY_PHAPI_FUNCTION_DECL("owplLineGetUri",                        PyOwplLineGetUri),
    PY_PHAPI_FUNCTION_DECL("owplLineAddCredential",                 PyOwplLineAddCredential),
    PY_PHAPI_FUNCTION_DECL("owplLineSetBusy",                       PyOwplLineSetBusy),
    PY_PHAPI_FUNCTION_DECL("owplLineIsBusy",                        PyOwplLineIsBusy),
    PY_PHAPI_FUNCTION_DECL("owplLineSetAutoKeepAlive",              PyOwplLineSetAutoKeepAlive),
    PY_PHAPI_FUNCTION_DECL("owplCallCreate",                        PyOwplCallCreate),
    PY_PHAPI_FUNCTION_DECL("owplCallConnect",                       PyOwplCallConnect),
    PY_PHAPI_FUNCTION_DECL("owplCallConnectWithBody",               PyOwplCallConnectWithBody),
    PY_PHAPI_FUNCTION_DECL("owplCallAccept",                        PyOwplCallAccept),
    PY_PHAPI_FUNCTION_DECL("owplCallAnswer",                        PyOwplCallAnswer),
    PY_PHAPI_FUNCTION_DECL("owplCallAnswerWithBody",                PyOwplCallAnswerWithBody),
    PY_PHAPI_FUNCTION_DECL("owplCallReject",                        PyOwplCallReject),
    PY_PHAPI_FUNCTION_DECL("owplCallRejectWithPredefinedReason",    PyOwplCallRejectWithPredefinedReason),
    PY_PHAPI_FUNCTION_DECL("owplCallHold",                          PyOwplCallHold),
    PY_PHAPI_FUNCTION_DECL("owplCallHoldWithBody",                  PyOwplCallHoldWithBody),
    PY_PHAPI_FUNCTION_DECL("owplCallUnhold",                        PyOwplCallUnhold),
    PY_PHAPI_FUNCTION_DECL("owplCallUnholdWithBody",                PyOwplCallUnholdWithBody),
    PY_PHAPI_FUNCTION_DECL("owplCallDisconnect",                    PyOwplCallDisconnect),
    /*PY_PHAPI_FUNCTION_DECL("owplCallGetLocalID",                  PyOwplCallGetLocalID), // not yet implemented */
    /*PY_PHAPI_FUNCTION_DECL("owplCallGetRemoteID",                 PyOwplCallGetRemoteID), // not yet implemented */
    /*PY_PHAPI_FUNCTION_DECL("owplCallSetAssertedId",               PyOwplCallSetAssertedId), // not yet implemented */
    /*PY_PHAPI_FUNCTION_DECL("owplCallGetRemoteContact",            PyOwplCallGetRemoteContact), // not yet implemented */
    /*PY_PHAPI_FUNCTION_DECL("owplCallToneStart",                   PyOwplCallToneStart), // not yet implemented */
    /*PY_PHAPI_FUNCTION_DECL("owplCallToneStop",                    PyOwplCallToneStop), // not yet implemented */
    /*PY_PHAPI_FUNCTION_DECL("owplCallTonePlay",                    PyOwplCallTonePlay), // not yet implemented */
    /*PY_PHAPI_FUNCTION_DECL("owplCallAudioPlayFileStart",          PyOwplCallAudioPlayFileStart), // not yet implemented */
    /*PY_PHAPI_FUNCTION_DECL("owplCallAudioPlayFileStop",           PyOwplCallAudioPlayFileStop), // not yet implemented */
    /*PY_PHAPI_FUNCTION_DECL("owplCallSendInfo",                    PyOwplCallSendInfo), // not yet implemented */
    PY_PHAPI_FUNCTION_DECL("owplPresenceSubscribe",                 PyOwplPresenceSubscribe),
    PY_PHAPI_FUNCTION_DECL("owplPresenceUnsubscribe",               PyOwplPresenceUnsubscribe),
    PY_PHAPI_FUNCTION_DECL("owplPresenceUnsubscribeFromUri",        PyOwplPresenceUnsubscribeFromUri),
    PY_PHAPI_FUNCTION_DECL("owplPresencePublish",                   PyOwplPresencePublish),
    PY_PHAPI_FUNCTION_DECL("owplMessageSend",                       PyOwplMessageSend),
    PY_PHAPI_FUNCTION_DECL("owplMessageSendPlainText",              PyOwplMessageSendPlainText),
    PY_PHAPI_FUNCTION_DECL("owplMessageSendTypingState",            PyOwplMessageSendTypingState),
    PY_PHAPI_FUNCTION_DECL("owplMessageSendIcon",                   PyOwplMessageSendIcon),

    PY_PHAPI_FUNCTION_DECL("owplPluginLoad",                        PyOwplPluginLoad),
    PY_PHAPI_FUNCTION_DECL("owplPluginSetCallback",                 PyOwplPluginSetCallback),
    PY_PHAPI_FUNCTION_DECL("owplPluginSetParam",                    PyOwplPluginSetParam),

    PY_PHAPI_FUNCTION_DECL("owplsendFile",                          PyOwplsendFile),
    PY_PHAPI_FUNCTION_DECL("owplreceiveFile",                       PyOwplreceiveFile),
    PY_PHAPI_FUNCTION_DECL("owplcancelTransfer",                    PyOwplcancelTransfer),
    PY_PHAPI_FUNCTION_DECL("owplpauseTransfer",                     PyOwplpauseTransfer),
    PY_PHAPI_FUNCTION_DECL("owplresumeTransfer",                    PyOwplresumeTransfer),

    PY_PHAPI_FUNCTION_DECL("owplAdapterNortelInitialize",           PyOwplAdapterNortelInitialize),
    PY_PHAPI_FUNCTION_DECL("owplAdapterWlmInitialize",              PyOwplAdapterWlmInitialize),

//     PY_PHAPI_FUNCTION_DECL("owsl_initialize",                       PyOwsl_initialize),
//     PY_PHAPI_FUNCTION_DECL("owsl_terminate",                        PyOwsl_terminate),
//     PY_PHAPI_FUNCTION_DECL("owsl_socket_type_add",                  PyOwsl_socket_type_add),
//     PY_PHAPI_FUNCTION_DECL("owsl_socket_type_remove",               PyOwsl_socket_type_remove),

    PY_PHAPI_FUNCTION_DECL("phConf",                                PyPhConf),
    PY_PHAPI_FUNCTION_DECL("phCfgSetS",                             PyPhCfgSetS),
    PY_PHAPI_FUNCTION_DECL("phCfgSetI",                             PyPhCfgSetI),

    PY_PHAPI_FUNCTION_DECL_NULL,
};

/**
 * @brief Function called on import
 */
PyMODINIT_FUNC initpyphapi(void) {
#ifdef WIN32
    SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)unhandledExceptionFilter);
#endif
    Py_InitModule3("pyphapi", pyphapi_funcs, "");
    PyEval_InitThreads();
}
