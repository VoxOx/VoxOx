#!/usr/bin/env python
#
# Tests several register to Wengo platform with PhApi
#
# @author Philippe Bernery

from pyphapi import *
import sys, time

USAGE="""
usage: phapi_register_test.py <sip_fullid> <sip_password>
e.g: phapi_register_test.py test@voip.wengo.fr myPassword
"""

OWPL_RESULT_SUCCESS = 0
OWPL_TRANSPORT_UDP = 0
REGISTER_INTERVAL = 3
REGISTER_TIMEOUT = 49 * 60

LINESTATE_UNKNOWN = -1
LINESTATE_REGISTERING = 20000
LINESTATE_REGISTERED = 21000
LINESTATE_UNREGISTERING = 22000
LINESTATE_UNREGISTERED = 23000
LINESTATE_REGISTER_FAILED = 24000
LINESTATE_UNREGISTER_FAILED = 25000
LINESTATE_PROVISIONED = 26000
LINESTATE_DELETING = 27000

LINESTATE_CAUSE_UNKNOWN = 0
LINESTATE_CAUSE_NORMAL = 1
LINESTATE_CAUSE_COULD_NOT_CONNECT = 2
LINESTATE_CAUSE_NOT_AUTHORIZED = 3
LINESTATE_CAUSE_TIMEOUT = 4
LINESTATE_CAUSE_NOT_FOUND = 5

def get_login_server(fullId):
    """
    @return the login and the sip server extracted from the
    fullId.
    e.g: "test@voip.wengo.fr" will return "test", "voip.wengo.fr"
    """
    split = fullId.split("@")
    return split[0], split[1]

def callback_callProgress(event, cause, vLineId, callId, remoteId):
    print "** callback_callProgress"

def callback_registerProgress(event, cause, vLineId):
    print "** callback_registerProgress"

    if event == LINESTATE_UNKNOWN :
        pass
    elif event == LINESTATE_REGISTERING:
        if cause == LINESTATE_CAUSE_NORMAL:
            print "** LINESTATE_REGISTERING - LINESTATE_CAUSE_NORMAL"
        else:
            print "** LINESTATE_REGISTERING - no cause"
    elif event == LINESTATE_REGISTERED:
        if cause == LINESTATE_CAUSE_NORMAL:
            print "** LINESTATE_REGISTERED - LINESTATE_CAUSE_NORMAL"
        else:
            print "** LINESTATE_REGISTERED - no cause"
    elif event == LINESTATE_UNREGISTERING:
        if cause == LINESTATE_CAUSE_NORMAL:
            print "** LINESTATE_UNREGISTERING - LINESTATE_CAUSE_NORMAL"
        else:
            print "** LINESTATE_UNREGISTERING - no cause"
    elif event == LINESTATE_UNREGISTERED:
        if cause == LINESTATE_CAUSE_NORMAL:
            print "** LINESTATE_UNREGISTERED - LINESTATE_CAUSE_NORMAL"
        else:
            print "** LINESTATE_UNREGISTERED - no cause"
    elif event == LINESTATE_REGISTER_FAILED:
        if cause == LINESTATE_CAUSE_COULD_NOT_CONNECT:
            print "!! LINESTATE_REGISTER_FAILED - LINESTATE_CAUSE_COULD_NOT_CONNECT: no response from server"
        elif cause == LINESTATE_CAUSE_NOT_AUTHORIZED:
            print "!! LINESTATE_REGISTER_FAILED - LINESTATE_CAUSE_NOT_AUTHORIZED: bad login or password"
        elif cause == LINESTATE_CAUSE_TIMEOUT:
            print "!! LINESTATE_REGISTER_FAILED - LINESTATE_CAUSE_TIMEOUT: no response from server"
        elif cause == LINESTATE_CAUSE_NOT_FOUND:
            print "!! LINESTATE_REGISTER_FAILED - LINESTATE_CAUSE_NOT_FOUND: bad login or password"
        else:
            print "!! LINESTATE_REGISTER_FAILED - no cause: no response from server"
    elif event == LINESTATE_UNREGISTER_FAILED:
        if cause == LINESTATE_CAUSE_COULD_NOT_CONNECT:
            print "!! LINESTATE_REGISTER_FAILED - LINESTATE_CAUSE_COULD_NOT_CONNECT"
        elif cause == LINESTATE_CAUSE_NOT_AUTHORIZED:
            print "!! LINESTATE_REGISTER_FAILED - LINESTATE_CAUSE_NOT_AUTHORIZED"
        elif cause == LINESTATE_CAUSE_TIMEOUT:
            print "!! LINESTATE_REGISTER_FAILED - LINESTATE_CAUSE_TIMEOUT"
        else:
            print "!! LINESTATE_REGISTER_FAILED - no cause: no response from server"
    elif event == LINESTATE_PROVISIONED:
        if cause == LINESTATE_CAUSE_NORMAL:
            print "** LINESTATE_PROVISIONED - LINESTATE_CAUSE_NORMAL"
        else:
            print "** LINESTATE_PROVISIONED - no cause"

def callback_messageProgress(event, cause, messageId, contentType, subContentType, localId, remoteId, content):
    print "** callback_messageProgress"

def callback_subscriptionProgress(state, cause, hSub, remoteId):
    print "** callback_subscriptionProgress"

def callback_onNotify(event, remoteId, xmlContent):
    print "** callback_onNotify"

def callback_errorNotify(event):
    print "** callback_errorNotify"

def sip_register(login, password, server):
    """
    Registers on a sip server using PhApi.
    """
    if owplInit(1, 0, -1, -1, "", 0) == OWPL_RESULT_SUCCESS:
        print "** PhApi initialized"

        owplEventListenerAdd(callback_callProgress,
            callback_registerProgress,
            callback_messageProgress,
            callback_subscriptionProgress,
            callback_onNotify,
            callback_errorNotify)

        vLineId = owplLineAdd(login, login, server, server, OWPL_TRANSPORT_UDP, REGISTER_TIMEOUT)
        if vLineId == None or vLineId == -1:
            print "!! owplLineAdd failed"
            sys.exit(1)

        if owplLineAddCredential(vLineId, login, password, server) != OWPL_RESULT_SUCCESS:
            print "!! owplLineAddCredential failed"
            sys.exit(1)

        owplLineSetAutoKeepAlive(vLineId, 1, 30);

        if owplLineRegister(vLineId, 1) != OWPL_RESULT_SUCCESS:
            print "!! owplLineRegister failed"

        # Let some time to register.
        time.sleep(3)

        owplLineDelete(vLineId, False);

        # Let some time to unregister.
        time.sleep(3)

        owplShutdown()
    else:
        print "!! cannot initialize PhApi"

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print USAGE
        sys.exit(1)

    login, server = get_login_server(sys.argv[1])
    password = sys.argv[2]

    ## Register/Unregister 5 times with an intervall of 3 secs.
    for i in range(5):
        print "** registering %d/5" % (i + 1)
        sip_register(login, password, server)
        time.sleep(REGISTER_INTERVAL)
    ####
