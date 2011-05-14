
#
SCSPROXY1=192.168.10.23
PHAPISERVER=192.168.10.37

MINIUA=test/bin/miniua
REGSLEEP=3
REGTIMEOUT=3600
EXITSLEEP='#'
#



# build the authenication command string
AUTHCMD="auta $USERNAME $USERID $PASSWD $REALM"

# rebuild the authenication command string
newauth() {
AUTHCMD="auta $USERNAME $USERID  $PASSWD $REALM"
}

# disable the authentication
noauth() {
AUTHCMD='# NO AUTHENTICATION'
}


# prepare to use PHAPISERVER
usephapiserver() {
MINIUA="test/bin/miniuacl -s $PHAPISERVER"
USEPHAPISERVER=1
}


# run phapiserver application using TELNET server on the
# target machine
runphserver0() {
  python  <<EOF
from telnetlib import Telnet
print "connecting to PHAPISERVER=" + "$PHAPISERVER"
tn = Telnet("$PHAPISERVER")
tn.write("phapiserver\r\nexit\r\n")
print tn.read_all()
EOF
}


stopphserver0() {
  python  <<EOF
from telnetlib import Telnet
print "STOPPING PHAPISERVER=" + "$PHAPISERVER"
tn = Telnet("$PHAPISERVER")
tn.write("killall phapiserver\r\nexit\r\n")
print tn.read_all()
EOF
}




# run phapiserver application using SSH server on the
# target machine
runphserver1() {
  ssh $PHAPISERVER 'killall phapiserver; cd wifo; test/bin/phapiserver'
}

runphserver() {
if [ $USEPHAPISERVER == 1 ]; then
    stopphserver0
    runphserver0&
    sleep 2 
fi
}


stopphserver() {
echo stopping server
}


# execute registration
regtest() {
TN=$1
OK=$2
FAILED=$3
TRESULT=$OK
trap TRESULT=$FAILED  ERR
$MINIUA -verbose <<EOF
$AUTHCMD
vla $USERNAME $REGISTRAR p=$PROXY t=$REGTIMEOUT
sleep $REGSLEEP
EBREAK 5
$EXITSLEEP
q
EOF
echo TEST REG_$TN $TRESULT | tee -a regtests.log
}

# execute 4 registrations
reg4test() {
TN=$1
OK=$2
FAILED=$3
TRESULT=$OK
trap TRESULT=$FAILED  ERR
$MINIUA -verbose <<EOF
auta $USERNAME1 $USERID1  $PASSWD1 $REALM1
auta $USERNAME2 $USERID2  $PASSWD2 $REALM2
auta $USERNAME3 $USERID3  $PASSWD3 $REALM3
auta $USERNAME4 $USERID4  $PASSWD4 $REALM4
vla $USERNAME1 $REGISTRAR1 p=$PROXY1 t=$REGTIMEOUT1
vla $USERNAME2 $REGISTRAR2 p=$PROXY2 t=$REGTIMEOUT2
vla $USERNAME3 $REGISTRAR3 p=$PROXY3 t=$REGTIMEOUT3
vla $USERNAME4 $REGISTRAR4 p=$PROXY4 t=$REGTIMEOUT4
sleep $REGSLEEP
EBREAK 5
$EXITSLEEP
q
EOF
echo TEST REG_$TN $TRESULT | tee -a regtests.log
}


# execute registration followed by another one with different timeout
reregtest0() {
TN=$1
OK=$2
FAILED=$3
TRESULT=$OK
trap TRESULT=$FAILED  ERR
$MINIUA -verbose <<EOF
$AUTHCMD
vla $USERNAME $REGISTRAR p=$PROXY t=$REGTIMEOUT
sleep 3
EBREAK 5
vla $USERNAME $REGISTRAR2 p=$PROXY2 t=$4
sleep 3
EBREAK 5
q
EOF
echo TEST REG_$TN $TRESULT | tee -a regtests.log
}





 

reregtest() {
REGISTRAR2=$REGISTRAR
PROXY2=$PROXY
reregtest0 $1 $2 $3 1200
}

unregtest() {
REGISTRAR2=$REGISTRAR
PROXY2=$PROXY
reregtest0 $1 $2 $3 0
}


reregtest2() {
TN=$1
OK=$2
FAILED=$3
TRESULT=$OK
trap TRESULT=$FAILED  ERR
$MINIUA -verbose <<EOF
$AUTHCMD
vla $USERNAME $REGISTRAR p=$PROXY t=$REGTIMEOUT
sleep 3
EBREAK 5
vla $USERNAME $REGISTRAR p=$PROXY t=$4
sleep 3
EBREAK 5
vla $USERNAME $REGISTRAR p=$PROXY t=$REGTIMEOUT
sleep 3
EBREAK 5
vla $USERNAME $REGISTRAR p=$PROXY t=$4
sleep 3
EBREAK 5
vla $USERNAME $REGISTRAR p=$PROXY t=$REGTIMEOUT
sleep 3
EBREAK 5
vla $USERNAME $REGISTRAR p=$PROXY t=$4
sleep 3
EBREAK 5
q
EOF
echo TEST REG_$TN $TRESULT | tee -a regtests.log
}



regrobustnestest() {
reregtest2 $1 $2 $3 0
}



# add virtual line and then delete it
delrobustnesttest() {
TN=$1
OK=$2
TRESULT=$OK
FAILED=$3
trap TRESULT=$FAILED  ERR
$MINIUA -verbose <<EOF
$AUTHCMD
vla $USERNAME $REGISTRAR p=$PROXY t=$REGTIMEOUT
sleep 3
EBREAK 5
vld 1
EBREAK 5
sleep 3
EBREAK 5
vla $USERNAME $REGISTRAR p=$PROXY t=$REGTIMEOUT
sleep 3
EBREAK 5
vld 1
EBREAK 5
sleep 3
EBREAK 5
vla $USERNAME $REGISTRAR p=$PROXY t=$REGTIMEOUT
sleep 3
EBREAK 5
vld 1
EBREAK 5
sleep 3
EBREAK 5
vla $USERNAME $REGISTRAR p=$PROXY t=$REGTIMEOUT
sleep 3
EBREAK 5
vld 1
EBREAK 5
sleep 3
EBREAK 5
vla $USERNAME $REGISTRAR p=$PROXY t=$REGTIMEOUT
sleep 3
EBREAK 5
vld 1
EBREAK 5
sleep 3
EBREAK 5
q
EOF
echo TEST REG_$TN $TRESULT | tee -a regtests.log
}

LINE1=1
delregtest() {
TN=$1
OK=$2
TRESULT=$OK
FAILED=$3
trap TRESULT=$FAILED  ERR
$MINIUA -verbose <<EOF
$AUTHCMD
vla $USERNAME $REGISTRAR p=$PROXY t=$REGTIMEOUT
sleep 3
EBREAK 5
vld $LINE1
EBREAK 5
sleep 3
EBREAK 5
q
EOF
echo TEST REG_$TN $TRESULT | tee -a regtests.log
}



defregporttest1() {
REGISTRAR=$SCSPROXY1
PROXY=$SCSPROXY1
REGISTRAR2=$SCSPROXY1:5060
PROXY2=$SCSPROXY1:5060
reregtest0  $1 $2 $3 1200
}

defregporttest2() {
REGISTRAR=$SCSPROXY1:5060
PROXY=$SCSPROXY1:5060
REGISTRAR2=$SCSPROXY1
PROXY2=$SCSPROXY1
reregtest0  $1 $2 $3 1200
}


unregtest2() {
REGISTRAR=$SCSPROXY1:5060
PROXY=$SCSPROXY1:5060
REGISTRAR2=$SCSPROXY1
PROXY2=$SCSPROXY1
reregtest0 $1 $2 $3 0
}


unregtest3() {
REGISTRAR=$SCSPROXY1
PROXY=$SCSPROXY1
REGISTRAR2=$SCSPROXY1:5060
PROXY2=$SCSPROXY1:5060
reregtest0 $1 $2 $3 0
}


USERNAME=3002
REGISTRAR=$SCSPROXY1
PROXY=$SCSPROXY1


tests_reg1_18() {
echo "CONFIGURE PROXY without athentication"
read a

noauth
runphserver
regtest 1 OK FAILED
runphserver
unregtest 2 OK FAILED
runphserver
reregtest 4 OK FAILED
runphserver
regrobustnestest 5 OK FAILED
runphserver
delregtest 6 OK FAILED
LINE1=4
runphserver
delregtest 7 FAILED OK
LINE1=1
runphserver
delrobustnesttest 8 OK FAILED


PROXY=$SCSPROXY1:5065
REGISTRAR=$SCSPROXY1:5065

runphserver
regtest 9 OK FAILED
runphserver
unregtest 10 OK FAILED
runphserver
reregtest 11 OK FAILED

runphserver
regrobustnestest 12 OK FAILED
runphserver
delregtest 13 OK FAILED
runphserver
delrobustnesttest 14 OK FAILED

runphserver
defregporttest1 15 OK FAILED
runphserver
defregporttest2 16 OK FAILED
runphserver
unregtest2 17 OK FAILED
runphserver
unregtest3 18 OK FAILED
}


tests_reg19_30() {

echo configure proxy with authentication
read a

USERNAME=3002
USERID=stepmind
PASSWD=12345678
REALM=stepmind.com
PROXY=$SCSPROXY1
REGISTRAR=$SCSPROXY1
newauth


runphserver
regtest 19 OK FAILED
runphserver
unregtest 20 OK FAILED
runphserver
reregtest 21 OK FAILED
runphserver
regrobustnestest 22 OK FAILED
runphserver
delregtest 23 OK FAILED
runphserver
delrobustnesttest 24 OK FAILED



PROXY=$SCSPROXY1:5065
REGISTRAR=$SCSPROXY1:5065


runphserver
regtest 25 OK FAILED
runphserver
unregtest 26 OK FAILED
runphserver
reregtest 27 OK FAILED
runphserver
regrobustnestest 28 OK FAILED
runphserver
delregtest 29 OK FAILED
runphserver
delrobustnesttest 30 OK FAILED
}



PROXY=$SCSPROXY1
REGISTRAR=$SCSPROXY1


test_reg31() {
USERID=steppmind
PASSWD=badpass
newauth
regtest 31 FAILED OK
}


test_reg33() {
USERID=stepmind
PASSWD=badpass
newauth
regtest 33 FAILED OK
}


test_reg34() {
USERID=stepmind
PASSWD=12345678
REALM=bad.com
newauth
regtest 34 FAILED OK
}

test_reg37() {
echo RUNNIG TEST REG_37
USERID=stepmind
PASSWD=12345678
REALM=stepmind.com
newauth
e0=$EXITSLEEP
r0=$REGTIMEOUT
EXITSLEEP='sleep 10'
REGTIMEOUT=8
regtest xx OK FAILED >regtest37.log
# we should have 3 lines with status=0
a=`cat regtest35.log | grep status=0 | wc -l`
grep -v OK <regtest37.log
if [ $a == 3 ]; then
echo TEST REG_37 OK | tee -a regtests.log
else
echo TEST REG_37 FAILED | tee -a regtests.log
fi
EXITSLEEP=$e0
REGTIMEOUT=$r0
}





test_reg38() {
echo TEST REG_38 FAILED | tee -a regtests.log
}

test_reg39() {
echo TEST REG_39 FAILED | tee -a regtests.log
}


test_reg40() {
PROXY=badhost.com
REGSLEEP=30
regtest 40 FAILED OK
REGSLEEP=3
}

test_reg41() {
PROXY=$SCSPROXY1:7070
REGISTRAR=$SCSPROXY1:7070
REGSLEEP=40
regtest 41 FAILED OK
REGSLEEP=3
}

test_reg42() {
PROXY1=$SCSPROXY1
REGISTRAR1=$SCSPROXY1
USERNAME1=3001
USERID1=stepmind
REALM1=stepmind.com
REGTIMEOUT1=10
PASSWD1=12345678

PROXY2=$SCSPROXY1
REGISTRAR2=$SCSPROXY1
USERNAME2=3002
USERID2=stepmind
REALM2=stepmind.com
REGTIMEOUT2=10
PASSWD2=12345678

PROXY3=$SCSPROXY1
REGISTRAR3=$SCSPROXY1
USERNAME3=3003
USERID3=stepmind
REALM3=stepmind.com
REGTIMEOUT3=10
PASSWD3=12345678

PROXY4=$SCSPROXY1
REGISTRAR4=$SCSPROXY1
USERNAME4=3004
USERID4=stepmind
REALM4=stepmind.com
REGTIMEOUT4=10
PASSWD4=12345678

REGSLEEP=10
reg4test xx OK FAILED >regtest42.log
# we should have 8 lines with status=0
a=`cat regtest42.log | grep status=0 | wc -l`
grep -v OK <regtest42.log
if [ $a == 8 ]; then
echo TEST REG_42 OK | tee -a regtests.log
else
echo TEST REG_42 FAILED | tee -a regtests.log
fi
}


tests_reg31_42() {
runphserver
test_reg31
runphserver
test_reg32
runphserver
test_reg34
runphserver
test_reg37
#runphserver
test_reg38
#runphserver
test_reg39
runphserver
test_reg40
runphserver
test_reg41
runphserver
test_reg42
}


date | tee  regtests.log

usephapiserver
tests_reg1_18
tests_reg19_30
tests_reg31_42


grep -v xx regtests.log 








