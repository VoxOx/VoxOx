#!/bin/sh

SCRIPT="$0"
COUNT=0
while [ -L "${SCRIPT}" ]
do
	SCRIPT=$(readlink ${SCRIPT})
	COUNT=$(expr ${COUNT} + 1)
	if [ ${COUNT} -gt 100 ]
	then
		echo "Too many symbolic links"
		exit 1
	fi
done
APPDIR=$(dirname ${SCRIPT})

export LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:${APPDIR}"

cd ${APPDIR}

# Hardcode the Plastique style for now, because the audio sliders looks wrong
# with other styles.
exec ./@BINARY_NAME@ -style plastique "$@"
