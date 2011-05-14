MINIUA=test/bin/miniua
USER1=vad@192.168.10.13
USER2=hal@192.168.10.31
USER3=vad@192.168.10.23

$MINIUA -verbose $1 -codecs PCMU -f sip:$USER1 <<EOF
c $USER2
sleep 15
bx 1 $USER3
sleep 25
q
EOF

