

import signal
import os
import popen2
import re
import time

SCSPROXY1="192.168.10.23"
PHAPISERVER="192.168.10.37"
# MINIUACMD="strace -o minua.st.log test/bin/miniua  -nomedia" 
MINIUACMD="test/bin/miniua  -nomedia" 



class MiniuaError(Exception):
    pass

class Timeout(MiniuaError):
    pass


class MiniuaCommandFailed(MiniuaError):
    pass


class MiniuaNonZeroResult(MiniuaCommandFailed):
    pass

class MiniuaNegativeResult(MiniuaCommandFailed):
    pass



class RegistrationError(MiniuaCommandFailed):
    pass


class CallError(MiniuaCommandFailed):
    pass

class XferError(MiniuaCommandFailed):
    pass




def timeoutHandler(a, b):
    raise Timeout

def startTimer(timeout):
    signal.signal(signal.SIGALRM, timeoutHandler)
    signal.alarm(timeout)
    
def stopTimer():
    signal.alarm(0)



finalStates = ["CALLOK", "CALLCLOSED", "NOANSWER", "BUSY", "ERROR", "CALLREDIRECTED", "CALLREPLACED"]
nonFinalStates = [ "Dialing", "INCALL", "DIALING", "RINGING", "CALLHELD", "CALLRESUMED", "HOLDOK",
                       "RESUMEOK", "DTMF",  "XFERREQ", "XFERPROGRESS", "XFEROK"]

callEvents = finalStates + nonFinalStates


class Call:
    def __init__(self):
        self.state = "NONE"

            



class MiniuaBase:
    def __init__(self):
        self.calls = {}
        self.vlines = {}
        

        
    def readFromChild(self):
        l = self.cout.readline()
        if self.verbose: print "<<miniua: " + l.rstrip()
        return l

    def writeToChild(self, l):
        self.cin.write(l)
        self.cin.flush()
        if self.verbose: print ">>miniua: " + l.rstrip() 

    def do(self, cmd, waitResult=None, mustBeZero=None, mustBePositive=None):
        """Execute the command and optionally wait for result message"""
        self.writeToChild(cmd + "\n")
        if not waitResult:
            return

        
        l = self.readFromChild()
        while not l.startswith("result ") and not l.startswith("Call"):
            self.parseLine(l)
            l = self.readFromChild()

        s = l.split()
        if s[0] == "result":
            r = int(s[2])
        else:
            r = int(s[1])

        if mustBeZero and r != 0:
           raise MiniuaNonZeroResult,r

        if mustBePositive and r < 0:
           raise MiniuaNegativeResult,r


        return r


    def parseRegResult(self, l, dontraise=False):
       s = re.split("[= ,]", l)
       vl = int(s[2])
       if vl in self.vlines:
           oldtm = self.vlines[vl]["tm"]
       else:
           oldtm = 0


       p = (vl, int(s[5]))
       if int(s[5]):
           if dontraise:
               return p
           
           raise RegistrationError, p

       if s[0] == "UNREG":
           self.vlines[vl] = { "state": "UNREG", "tm" : 0 }
       else:
           self.vlines[vl] = { "state": "REG", "tm" : oldtm }

       return p

  

    def parseCallResult(self, l):

       s = re.split("[= ,]", l)

       if s[0] not in callEvents:
           return (None,None,None)


       lid = None
       if s[0] in ["Dialing", "DIALING" , "INCALL"]:
           cid = int(s[4])
           lid = int(s[2])
       else:
           cid = int(s[2])


       if s[0] == "INCALL":
           self.calls[cid]=Call()

       self.calls[cid].state = s[0]

       return (cid,s[0],lid)
       
           
        
    
    def parseLine(self, l):
       if "REG" in l:
           self.parseRegResult(l)
       else:
           self.parseCallResult(l)
   


    def collectRegResults(self, timeout, maxcount=None):
        """return list of tuples (vlid, regresult) from regitration callbacks
        occured during given period"""
        r = []
        try:
            
            startTimer(timeout)
            limit = maxcount or 1000000
            while limit >  len(r):
               l = self.readFromChild()
               if "REG" in l:
                   vl, s = self.parseRegResult(l, dontraise=True)
                   r += [(vl, s)]
               else:
                   self.parseCallResult(l)

        except:
            pass

        stopTimer()
        return r


    def waitRegResult(self, vl, timeout):

        try:
            
            startTimer(timeout)

            while not vl in self.vlines or  self.vlines[vl]["state"] == "PENDING":
                l = self.readFromChild()
                self.parseLine(l)

        except:
            stopTimer()
            raise RegistrationError

        stopTimer()
        

    def waitInCall(self, timeout, expect="INCALL"):
        
        try:
            
            startTimer(timeout)

            while True:
                l = self.readFromChild()
                if "REG" in l:
                    self.parseLine(l)
                else:
                   cid,state,lid = self.parseCallResult(l)
                   if not expect or expect == state:
                       stopTimer()
                       return cid,state

        except:
            stopTimer()
            raise CallError



                

    def waitCallResult(self, cid, timeout, expect="CALLOK"):

        try:
            
            startTimer(timeout)

            while not self.calls[cid].state in finalStates:
                l = self.readFromChild()
                self.parseLine(l)
                

        except:
            stopTimer()
            raise CallError,cid

        stopTimer()

        if self.calls[cid].state != expect:
            raise  CallError 
                

    def waitXferResult(self, cid, timeout, expect="XFEROK"):        
        try:
            waitCallResult(cid, timeout, expect)
        except:
            raise XferError
        
    
    

    def auta(self, username, userid, passwd, realm):
        cmd = "auta %s %s %s %s" % (username, userid, passwd, realm)
        self.do(cmd, True, True)

    
    def vla(self, username, host, proxy=None, timeout=0, waitTimeout=5):
        cmd = "vla %s %s" % (username, host)
        if proxy:  cmd += " p=" + proxy
        cmd += " t=" + str(timeout)

                
        vl = self.do(cmd, True, False, True)

        if not vl in self.vlines:
            oldtm = -1
            needWait = (timeout != 0)
        else:
            oldtm = self.vlines[vl]["tm"]
            needWait = oldtm != timeout


        if needWait:
            self.waitRegResult(vl, waitTimeout)

        return vl

    addVline = vla

    def vld(self, vl):
        self.vlines[vl]["state"] = "PENDING"
        oldtm = self.vlines[vl]["tm"]
        cmd = "vld " + str(vl)
        self.do(cmd, True, True, False)
        if oldtm:
            self.waitRegResult(vl, 5)

        

    def lc(self, vl, to, timeout=60):
        cmd = "lc %d %s" % (vl, to)
        cid = self.do(cmd, True, False, True)
        self.calls[cid] = Call()
        self.waitCallResult(cid, timeout)
        return cid

    placeCall = lc

    def n(self, cid):
        """Send RINGING"""
        cmd = "n " + str(cid)
        self.do(cmd, True, True, False)

    ringing = n
    
    def h(self, cid):
        """HANGUP"""
        cmd = "h " + str(cid)
        self.do(cmd, True, True, False)
        self.waitCallResult(cid, 3, "CALLCLOSED")

    hangup = h
    
    def a(self, cid):
        """ACCEPT """
        cmd = "a " + str(cid)
        self.do(cmd, True, True, False)
        self.waitCallResult(cid, 5, "CALLOK")

    accept = a

    def o(self, cid):
        """HOLD """
        cmd = "o " + str(cid)
        self.do(cmd, True, True, False)
        self.waitCallResult(cid, 5, "HOLDOK")

    hold = o

    def u(self, cid):
        """RESUME """
        cmd = "o " + str(cid)
        self.do(cmd, True, True, False)
        self.waitCallResult(cid, 5, "HOLDOK")

    
    resume = u

    def  dtmf(self, sig, mode):
        cmd = "dtmfmode " + str(mode)
        self.do(cmd, False, False, False)
        cmd = "m "  + sig[0]
        self.do(cmd, True, True, False)
    

    def bx(self, cid, to):
        cmd = "bx %d %s" % (cid, to)
        self.do(cmd, True, True, False)
        self.waitXferResult(cid)


    blindXfer = bx

    def ax(self, cid, cidto):
        cmd = "ax %d %d" % (cid, cidto)
        self.do(cmd, True, True, False)
        self.waitXferResult(cid)


    def exit(self, code=0):
        cmd = "exit " + str(code)
        self.do(cmd, False, False, False)
        
    def autoans(self, active=True):
        cmd = "autoans "
        if active:
            cmd += "1"
        else:
            cmd += "0"

        self.do(cmd, False, False, False)

    def autorej(self, active=True):
        cmd = "autorej "
        if active:
            cmd += "1"
        else:
            cmd += "0"

        self.do(cmd, False, False, False)

    def autoring(self, active=True):
        cmd = "autoring "
        if active:
            cmd += "1"
        else:
            cmd += "0"
        
        self.do(cmd, False, False, False)        

    def q(self):
        cmd = "q"
        self.do(cmd, False, False, False)


    def r(self):
        cmd = "r"
        self.do(cmd, True, True, False)        

    rejectCall = r
    
        



class Miniua(MiniuaBase):
    def __init__(self, cmd=MINIUACMD, sipport=None, verbose=False):
        MiniuaBase.__init__(self)

        if sipport : cmd += " -sipport " + str(sipport)
        self.cin, self.cout, e = os.popen3(cmd, "t", 512)
        self.verbose = verbose
        e.readline()
        while True:
            l = self.readFromChild()
            if "Welcome" in l:
                return


class PhapiServer:
    def __init__(self, cmd):
        self.cin, self.cout, e = os.popen3(cmd, "t", 512)
        e.readline()

    def terminate(self):
        self.cin.close()
        self.cout.close()

class TelnetPhapiServer:

    from telnetlib import Telnet

    def __init__(self, target, cmd="phapiserver", killold=True):
        self.tn = Telnet(target);
        if killoold:
            self.tn.write("killall phapiserver\r\nexit\r\n")

    def terminate(self):
        self.tn.read_all()


    




if __name__ == '__main__':
    id="wx_mbdsys_6"
    pw="mbdsys_6"
    m = Miniua()
    m.auta(id, id, pw, "len1.host.wengo.fr")
    vl = m.addVline(id, "len1.host.wengo.fr", "proxy1.host.wengo.fr", 3600) 
    print "VL=",vl   
    cid = m.placeCall(vl, "0686444487@len1.host.wengo.fr")
    print "CID=", cid
    time.sleep(5)
    m.hangup(cid)


    
    
            

