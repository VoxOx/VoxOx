

import miniuadrvr
import unittest
import os
import time
import thread

SCSPROXY1=SCSPROXY2=SCSPROXY3=SCSPROXY4="192.168.10.23"

ALTPROXY=ALTREGISTRAR="192.168.10.31"

USERNAME1="3001"
USERID1=USERID2=USERID3=USERID4="stepmind"
REALM1=REALM2=REALM3=REALM4="stepmind.com"
PROXY1=PROXY2=PROXY3=PROXY4=SCSPROXY1
REGISTRAR1=REGISTRAR2=REGISTRAR3=REGISTRAR4=SCSPROXY1
PWD1=PWD2=PWD3=PWD4="12345678"

USERNAME2="3002"
USERNAME3="3003"
USERNAME4="3004"
MINIUA="test/bin/miniua -nomedia "

def startMiniua(cmd=MINIUA, verb=False):
    return miniuadrvr.Miniua(cmd, verbose=verb  )

def setupProxies(bias=""):

    PROXY1=SCSPROXY1+bias
    REGISTRAR1=SCSPROXY1+bias
    PROXY2=SCSPROXY2+bias        
    REGISTRAR2=SCSPROXY2+bias        
    PROXY3=SCSPROXY3+bias
    REGISTRAR3=SCSPROXY3+bias        
    PROXY4=SCSPROXY4+bias
    REGISTRAR4=SCSPROXY4+bias
    






class RegTestBase(unittest.TestCase):


    def setUp(self):
        self.m = m

    def basicreg(self):
        self.m.vla(USERNAME1, REGISTRAR1, PROXY1,  3600)

    def basicunreg(self):
        time.sleep(1)
        self.m.vla(USERNAME1, REGISTRAR1, PROXY1,  0)


    def basicrereg(self, t2=1200):
        time.sleep(1)
        self.m.vla(USERNAME1, REGISTRAR1, PROXY1,  3600)
        time.sleep(1)
        self.m.vla(USERNAME1, REGISTRAR1, PROXY1,  t2)




    def regrobustness(self):

        params = [ (USERNAME1, USERID1, PWD1, REALM1, REGISTRAR1, PROXY1),
                   (USERNAME2, USERID2, PWD2, REALM2, REGISTRAR2, PROXY2),
                   (USERNAME3, USERID3, PWD3, REALM3, REGISTRAR3, PROXY3),
                   (USERNAME4, USERID4, PWD4, REALM4, REGISTRAR4, PROXY4)]


        for u,i,p,r,rr,px in params:
            self.m.auta(u,i, p, r)
            time.sleep(1)
            self.m.vla(u, rr, px, 1200)
            time.sleep(1)




    def nonvldel(self):
        self.assertRaises(miniuadrvr.MiniuaNonZeroResult, self.m.vld, 1)

    def basicdelvl(self):
        time.sleep(1)
        self.m.vld(1)

        

    def delrobustness(self):

        time.sleep(1)
        params = [ (USERNAME1, USERID1, PWD1, REALM1, REGISTRAR1, PROXY1),
                   (USERNAME2, USERID2, PWD2, REALM2, REGISTRAR2, PROXY2),
                   (USERNAME3, USERID3, PWD3, REALM3, REGISTRAR3, PROXY3),
                   (USERNAME4, USERID4, PWD4, REALM4, REGISTRAR4, PROXY4)]

        lines = []

        for u,i,p,r,rr,px in params:
            self.m.auta(u,i, p, r)
            time.sleep(1)
            lines += [self.m.vla(u, rr, px, 1200)]
            time.sleep(1)


        for vl in lines:
            self.m.vld(vl)

        



class Regtests1_18(RegTestBase):


    def testreg_01(self):
        """reg_01: Test Simple Registration"""
        self.basicreg()
        
    def testreg_02(self):
        """reg_02: Test Simple UnRegistration"""
        self.basicunreg()

    def testreg_03(self):
        """reg_03: delete nonexisting vl"""
        self.assertRaises(miniuadrvr.MiniuaCommandError, self.m.vld, 33)


    def testreg_04(self):
        """reg_04: Test Simple Re-Registration"""
        self.basicrereg(1200)

    def testreg_05(self):
        """reg_05: Registration robustness tests"""
        self.regrobustness()


    def testreg_06(self):
        """reg_6: Test Delete Virtual Line"""
        self.basicdelvl()


    def testreg_07(self):
        """reg_07: Test Delete NON-EXISTENT Virtual Line"""
        self.nonvldel()

    def testreg_08(self):
        """reg_08: Delete Vline robustness test"""
        self.delrobustness()
    
    


    def testreg_09(self):
        """reg_09: Test Simple port biased Registration"""

        setupProxies(":5065")

        self.basicreg()

    def testreg_10(self):
        """reg_10: Test Simple port biased UnRegistration"""
        self.basicunreg()

    def testreg_11(self):
        """reg_11: Test Simple port biased Re-Registration"""
        self.basicrereg()


    def testreg_12(self):
        """reg_12: Registration port biased robustness tests"""
        self.regrobustness()


    def testreg_13(self):
        """reg_13: Test Delete port biased Virtual Line"""
        self.basicdelvl()

        


    def testreg_14(self):
        """reg_14: Delete port biased Vline robustness test"""
        self.delrobustness()


    def dualport(self, port1=None, port2=None, timeout=1200):
        time.sleep(1)
        params = [ (USERNAME1, USERID1, PWD1, REALM1, SCSPROXY1, SCSPROXY1) ]

        lines = []
        for u,i,p,r,rr,px in params:
            self.m.auta(u,i, p, r)
            time.sleep(1)
            rr1 = rr
            px1 = px
            if port1:
                rr1 += ":"+port1
                px1 += ":"+port1
                
            lines += [self.m.vla(u, rr1, px1, 3600)]
            time.sleep(1)

            rr2 = rr
            px2 = px
            if port2:
                rr2 += ":"+port2
                px2 += ":"+port2

            self.m.vla(u, rr2, px2, timeout)

        time.sleep(1)       


    def testreg_15(self):
        """reg_15: Reregistration default / non-default"""
        self.dualport(None, "5060")

    def testreg_16(self):
        """reg_16: Reregistration default / non-default"""
        self.dualport("5060", None)

        
    def testreg_17(self):
        """reg_17: Unregistration  default / non-default"""
        self.dualport(None, "5060", 0)

    def testreg_18(self):
        """reg_18: Unregistration  non-default / default"""
        self.dualport("5060", None, 0)

     


class Regtests19_30(RegTestBase):


    def testreg_19(self):
        """reg_19: Test Autentincated Registration"""
        setupProxies()
        self.basicreg()
        
    def testreg_20(self):
        """reg_20: Test Autentincated UnRegistration"""
        self.basicunreg()


    def testreg_21(self):
        """reg_21: Test Autentincated  Re-Registration"""
        self.basicrereg(1200)
        



    def testreg_22(self):
        """reg_22: Autentincated Registration robustness tests"""
        self.regrobustness()


    def testreg_23(self):
        """reg_23: Autentincated Test Delete Virtual Line"""
        self.basicdelvl()



    def testreg_24(self):
        """reg_24: Autentincated Delete Vline robustness test"""
        self.delrobustness()


    def testreg_25(self):
        """reg_25: Autentincated Delete Vline robustness test non std port"""
        setupProxies(":5065")
        self.basicreg()



    def testreg_26(self):
        """reg_26: Autentincated  Unregister  non std port"""
        self.basicunreg()


    def testreg_27(self):
        """reg_27: Autentincated  Regeregister non std port"""
        self.basicrereg()


    def testreg_28(self):
        """reg_28: Autentincated  registration robustneess non std port"""
        self.regrobustness()

    def testreg_29(self):
        """reg_29: Autentincated  delete vline  non std port"""
        self.basicdelvl()        


    def testreg_30(self):
        """reg_30: Autentincated  delete vline  robustneess non std port"""
        self.delrobustness()        


    def testreg_31(self):
        """reg_31: Autentincated  regsitration Bad user id"""
        self.assertRaises(miniuadrvr.RegistrationError, self.m.vla, "baduser1", REGISTRAR1, PROXY1, 3600)

    def testreg_32(self):
        """reg_32: regsitration NO password (using empty password)"""
        self.assertRaises(miniuadrvr.MiniuaNonZeroResult, self.m.auta, "3006", USERID1, "", REALM1)        


    def testreg_33(self):
        """reg_33: regsitration Bad passwd"""
        self.m.auta("3006", USERID1, "badpass", REALM1)        
        self.assertRaises(miniuadrvr.RegistrationError, self.m.vla, "3006", REGISTRAR1, PROXY1, 3600)
        
    def testreg_34(self):
        """reg_34: regsitration Bad realm"""
        self.m.auta("3007", USERID1, PWD1, "bad realm")        
        self.assertRaises(miniuadrvr.RegistrationError, self.m.vla, "3007", REGISTRAR1, PROXY1, 3600)

    def testreg_35(self):
        """reg_35: same user name on different vlines"""
        setupProxies()
        self.m.auta("3100", USERID1, PWD1, REALM1)        
        self.m.auta("3100", "stepmind2", "pwd2", "stepmind2.com")        
        self.m.vla("3100", REGISTRAR1, PROXY1, 3600)
        self.m.vla("3100", ALTREGISTRAR, ALTPROXY, 3600)



    def testreg_37(self):
        """reg_37: Test automatic rereg"""
        vl = self.m.vla(USERNAME1, REGISTRAR1, PROXY1, 8)

        
        results = self.m.collectRegResults(30)
        vlregs = [ vlid for vlid,status in results if vlid == vl]  
        self.assert_(len(vlregs) >= 3)


    def testreg_38(self):
        """reg_38: timeout update by server"""
        self.assert_("Unsupported yet" == "FAIL")


    def testreg_39(self):
        """reg_39: timeout update by server"""
        self.assert_("Unsupported yet" == "FAIL")


    def testreg_40(self):
        """reg_40: register bad proxy"""
        self.assertRaises(miniuadrvr.MiniuaError, self.m.vla, USERNAME1, "bad.com" , "bad.com", 1200, 45)               


    def testreg_41(self):
        """reg_41: register bad proxy (wron port)"""
        self.assertRaises(miniuadrvr.MiniuaError, self.m.vla, USERNAME1, REGISTRAR1+":3030" , PROXY1+":3030", 1200, 45)               

    def testreg_42(self):
        """reg_42: multiple re-restration robustenss test"""
        global m
    
        self.m.q()
        self.m = m = startMiniua()
        

        time.sleep(1)
        params = [ (USERNAME1, USERID1, PWD1, REALM1, REGISTRAR1, PROXY1),
                   (USERNAME2, USERID2, PWD2, REALM2, REGISTRAR2, PROXY2),
                   (USERNAME3, USERID3, PWD3, REALM3, REGISTRAR3, PROXY3),
                   (USERNAME4, USERID4, PWD4, REALM4, REGISTRAR4, PROXY4)]

        lines = []
        
        for u,i,p,r,rr,px in params:
            self.m.auta(u,i, p, r)
            time.sleep(1)
            lines += [self.m.vla(u, rr, px, 8)]
            time.sleep(1)


        results = self.m.collectRegResults(30)
        vlregs = [ vlid for vlid,status in results if vlid in lines and status == 0]  
        self.assert_(len(vlregs) >= 4*3)



        

        
class ProxyCallBase(unittest.TestCase):

    def setUp(self):
        self.m = m
        self.a = a



    def basiccall(self):
        
        vl = m.vla(USERNAME1, REGISTRAR1, PROXY1, 3600)
        self.a.autoans(True)
        self.a.autoring(True)
        self.a.autorej(False)
        
        self.a.auta(USERNAME2, USERID2, PWD2, REALM2)
        self.a.vla(USERNAME2, REGISTRAR2, PROXY2, 3600)

        ccid = self.m.lc(vl, USERNAME2+"@"+REGISTRAR2)

        acid,astate = self.a.waitInCall(15)

        time.sleep(2)

        self.m.h(ccid)
        self.a.waitCallResult(acid, 5, "CALLCLOSED")
        
        
    def peernoanswer(self):
        self.a.autoans(False)
        self.a.autoring(False)
        self.a.autorej(False)
        
        vl = m.vla(USERNAME1, REGISTRAR1, PROXY1, 3600)
        

        try:
            
            ccid = self.m.lc(vl, USERNAME2+"@"+REGISTRAR2, 10)

        except:
            ccid = False

            
        acid,astate = self.a.waitInCall(15)
        self.assert_(not ccid)



        
    def peerrejecting(self):
        self.a.autoans(False)
        self.a.autoring(False)
        self.a.autorej(True)
        
        vl = m.vla(USERNAME1, REGISTRAR1, PROXY1, 3600)

        self.assertRaises(miniuadrvr.MiniuaError, self.m.lc, vl, USERNAME2+"@"+REGISTRAR2, 10)



        
    def peernotresponding(self):

        
        self.a.autoans(False)
        self.a.autoring(False)
        self.a.autorej(False)

        self.a.exit()
        
        vl = m.vla(USERNAME1, REGISTRAR1, PROXY1, 3600)

        self.assertRaises(miniuadrvr.MiniuaError, self.m.lc, vl, USERNAME2+"@"+REGISTRAR2, 10)

        global a
        a = self.a = startMiniua(cmd = MINIUA + " -sipport 5061 ")

        self.a.auta(USERNAME2, USERID2, PWD2, REALM2)
        self.a.vla(USERNAME2, REGISTRAR2, PROXY2, 3600)



class ProxyCallTests01_04(ProxyCallBase):
    
    def testbc_01(self):
        """bc_01: simple proxied call"""
        self.basiccall()


    def testbc_02(self):
        """bc_02: simple proxied call, peer not answering"""
        self.peernoanswer()


    def testbc_03(self):
        """bc_03: simple proxied call, peer rejectig"""
        self.peerrejecting()

    def testbc_04(self):
        """bc_04: simple proxied call, peer registered but not responding"""
        self.peernotresponding()



class ProxyCallTests05_08(ProxyCallBase):
    
    def testbc_05(self):
        """bc_05: simple autenticated proxied call"""
        self.basiccall()


    def testbc_06(self):
        """bc_06: simple autenticated proxied call, answering"""
        self.basiccall()


    def testbc_07(self):
        """bc_07: simple autenticated proxied call, peer rejectig"""
        self.peerrejecting()

    def testbc_08(self):
        """bc_08: simple atenticated proxied call, peer registered but not responding"""
        self.peernotresponding()






if __name__ == "__main__":

    m = startMiniua()
    
#    ID1 = USERID1 = "wx_mbdsys_6"
#    PWD1 = "mbdsys_6"
#    REGISTRAR1="len1.host.wengo.fr"
#    PROXY1="proxy1.host.wengo.fr"

    m.auta(USERNAME1, USERID1, PWD1, REALM1)


    if 0:
        print "Set Proxy to NON authentication mode"
        os.sys.stdin.readline()
        suite1 = unittest.makeSuite(Regtests1_18)
    
        unittest.TextTestRunner(verbosity=2).run(suite1)
        

        print "Set Proxy to authentication mode"
        os.sys.stdin.readline()
        suite2 = unittest.makeSuite(Regtests19_30)

        unittest.TextTestRunner(verbosity=2).run(suite2)
    
        m.q()

        m = startMiniua()

    a = startMiniua(cmd = MINIUA + " -sipport 5061 ", verb = True)
    suite3 = unittest.makeSuite(ProxyCallTests01_04)

    unittest.TextTestRunner(verbosity=2).run(suite3)
    

    print "Set Proxy to authentication mode"
    os.sys.stdin.readline()

    suite4 = unittest.makeSuite(ProxyCallTests05_08)

    unittest.TextTestRunner(verbosity=2).run(suite4)

    
