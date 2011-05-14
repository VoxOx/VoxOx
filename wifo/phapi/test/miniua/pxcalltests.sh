
#
SCSPROXY1=192.168.10.23

USERNAME=wx_mbdsys_6
USERID=wx_mbdsys_6
REALM=len1.host.wengo.fr
REGISTRAR=len1.host.wengo.fr
PROXY=proxy1.host.wengo.fr
REGISTRAR2=$REGISTRAR
PROXY2=$PROXY2
MINIUA=test/bin/miniua
LOCALUSER=vad
HOST=192.168.10.13
PHONE=0686444487@len1.host.wengo.fr
PCUSER=vad@192.168.10.23
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


# execute registration
startanswerer() {
exec $MINIUA -sipport 5061 -verbose -nomedia $1 $2 <<EOF
auta $USERNAME2 $USERID2  $PASSWD2 $REALM2
vla $USERNAME2 $REGISTRAR2 p=$PROXY2 t=$REGTIMEOUT2
sleep $REGSLEEP2
EBREAK 5
q
EOF
}

startrejector() {
exec $MINIUA -sipport 5061 -verbose -nomedia $1 $2 <<EOF
auta $USERNAME2 $USERID2  $PASSWD2 $REALM2
vla $USERNAME2 $REGISTRAR2 p=$PROXY2 t=$REGTIMEOUT2
sleep $REGSLEEP2
r 1
q
EOF
}



startcaller() {
TN=$1
OK=$2
FAILED=$3
TRESULT=$OK
trap TRESULT=$FAILED  ERR
$MINIUA -verbose -nomedia <<EOF
auta $USERNAME1 $USERID1  $PASSWD1 $REALM1
vla $USERNAME1 $REGISTRAR1 p=$PROXY1 t=$REGTIMEOUT1
sleep $REGSLEEP1
lc 1 $USERNAME2@$REGISTRAR2
sleep $CALLSLEEP1
EBREAK 3
h  1
sleep $HUPSLEEP1
EBREAK 5
EOF
echo TEST PROXY_BC_$TN $TRESULT | tee -a proxybctests.log
}


startcaller2() {
TN=$1
OK=$2
FAILED=$3
TRESULT=$OK
trap TRESULT=$FAILED  ERR
$MINIUA -verbose -nomedia <<EOF
auta $USERNAME1 $USERID1  $PASSWD1 $REALM1
vla $USERNAME1 $REGISTRAR1 p=$PROXY1 t=$REGTIMEOUT1
sleep $REGSLEEP1
lc 1 $USERNAME2@$REGISTRAR2
sleep $CALLSLEEP1
EBREAK 3
EOF
echo TEST PROXY_BC_$TN $TRESULT | tee -a proxybctests.log
}


setup_pxy_bc_tests() {
PROXY1=$SCSPROXY1
REGISTRAR1=$SCSPROXY1
USERNAME1=3001
USERID1=stepmind
REALM1=stepmind.com
REGTIMEOUT1=3200
PASSWD1=12345678
REGSLEEP1=3
HUPSLEEP=3
CALLSLEEP1=5

PROXY2=$SCSPROXY1
REGISTRAR2=$SCSPROXY1
USERNAME2=3002
USERID2=stepmind
REALM2=stepmind.com
REGTIMEOUT2=3200
PASSWD2=12345678
REGSLEEP2=30

}


test_pxy_bc_01() {
setup_pxy_bc_tests
REGSLEEP1=3
CALLSLEEP1=5

REGSLEEP2=30


startanswerer -autoring -autoans&
anspid=$! 

startcaller $1 OK FAILED
echo killing $anspid
kill -9 $anspid
}



test_pxy_bc_02() {
setup_pxy_bc_tests
CALLSLEEP1=45

REGSLEEP2=30


startanswerer&
anspid=$! 
sleep 3
echo killing $anspid
kill -9 $anspid
startcaller2 $1 FAILED OK
}


test_pxy_bc_03() {
setup_pxy_bc_tests
CALLSLEEP1=10

REGSLEEP2=10


startrejector&
startcaller2 $1 FAILED OK
}

test_pxy_bc_05() {
setup_pxy_bc_tests
CALLSLEEP1=90

REGSLEEP2=120


startrejector -autoring&
anspid=$!
startcaller2 $1 FAILED OK
kill -9 $anspid
}

date >proxybctests.log

echo DISABLE authenitcation at proxy $SCSPROXY1
read ans


test_pxy_bc_01  01
test_pxy_bc_02  02
test_pxy_bc_03  03
test_pxy_bc_05  05


echo ENABLE authenitcation at proxy $SCSPROXY1
read ans

test_pxy_bc_01  06


grep -v xx proxybctests.log 








