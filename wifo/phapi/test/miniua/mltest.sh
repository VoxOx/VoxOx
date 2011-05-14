USERNAME=wx_mbdsys_6
USERID=wx_mbdsys_6
REALM=len1.host.wengo.fr
REGISTRAR=len1.host.wengo.fr
PROXY=proxy1.host.wengo.fr
PASSWD=$1
MINIUA=test/bin/miniua
LOCALUSER=vad
HOST=192.168.10.13
PHONE=0686444487@len1.host.wengo.fr
PCUSER=vad@192.168.10.23

$MINIUA -verbose <<EOF
auta $USERID $USERNAME $PASSWD $REALM
vla $USERNAME $REGISTRAR p=$PROXY t=3600
sleep 5
vla $LOCALUSER $HOST
id $USERNAME@$REGISTRAR
c $PHONE
sleep 30
id $LOCALUSER@$HOST
c $PCUSER
sleep 15
q
EOF

