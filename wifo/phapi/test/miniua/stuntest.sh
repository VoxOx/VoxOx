MINIUA=test/bin/miniua
STUN=192.168.10.21

$MINIUA -n auto -stun $STUN <<EOF
sleep 5
natinfo
q
EOF
