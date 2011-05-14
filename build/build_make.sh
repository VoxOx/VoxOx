#!/bin/bash
#
# Script to build WengoPhone on UNIX.
#
# Copyright (c) 2006-2007 Andreas Schneider <mail@cynapses.org>
#

SOURCE_DIR=".."

LANG=C
export LANG

SCRIPT="$0"
COUNT=0
while [ -L "${SCRIPT}" ]
do
	SCRIPT=$(readlink ${SCRIPT})
	COUNT=$(expr ${COUNT} + 1)
	if [ ${COUNT} -gt 100 ]; then
		echo "Too many symbolic links"
		exit 1
	fi
done
BUILDDIR=$(dirname ${SCRIPT})

function cleanup_and_exit() {
	if test "$1" = 0 -o -z "$1" ; then
		exit 0
	else
		exit $1
	fi
}

function configure() {
	cmake "$@" ${SOURCE_DIR} || cleanup_and_exit $?
}

function compile() {
	CPUCOUNT=$(grep -c processor /proc/cpuinfo)
	if [ "${CPUCOUNT}" -gt "1" ]; then
		make -j${CPUCOUNT} $1 || cleanup_and_exit $?
	else
		make $1 || exit $?
	fi
}

function clean_build_dir() {
	find ! -path "*.svn*" ! -name "*.bat" ! -name "*.sh" ! -name "." -print0 | xargs -0 rm -rf
}

function usage () {
echo "Usage: `basename $0` [--prefix /install_prefix|--build [debug|final]|--clean|--verbose|--libsuffix (32|64)|--help]"
    cleanup_and_exit
}

cd ${BUILDDIR}

OPTIONS="--graphviz=${BUILDDIR}/wengophone.dot"

while test -n "$1"; do
	PARAM="$1"
	ARG="$2"
	shift
	case ${PARAM} in
		*-*=*)
		ARG=${PARAM#*=}
		PARAM=${PARAM%%=*}
		set -- "----noarg=${PARAM}" "$@"
	esac
	case ${PARAM} in
		*-help|-h)
			#echo_help
			usage
			cleanup_and_exit
		;;
		*-build)
			DOMAKE="1"
			BUILD_TYPE="${ARG}"
			test -n "${BUILD_TYPE}" && shift
		;;
		*-clean)
			clean_build_dir
			cleanup_and_exit
		;;
		*-verbose)
			DOVERBOSE="1"
		;;
		*-libsuffix)
			OPTIONS="${OPTIONS} -DLIB_SUFFIX=${ARG}"
			shift
		;;
		*-prefix)
			OPTIONS="${OPTIONS} -DCMAKE_INSTALL_PREFIX=${ARG}"
			shift
		;;
		----noarg)
			echo "$ARG does not take an argument"
			cleanup_and_exit
		;;
		-*)
			echo Unknown Option "$PARAM". Exit.
			cleanup_and_exit 1
		;;
		*)
			usage
		;;
	esac
done

case ${BUILD_TYPE} in
	debug)
		OPTIONS="${OPTIONS} -DCMAKE_BUILD_TYPE=Debug"
	;;
	release)
		OPTIONS="${OPTIONS} -DCMAKE_BUILD_TYPE=Release"
	;;
esac

if [ -n "${DOVERBOSE}" ]; then
	OPTIONS="${OPTIONS} -DCMAKE_VERBOSE_MAKEFILE=1"
else
	OPTIONS="${OPTIONS} -DCMAKE_VERBOSE_MAKEFILE=0"
fi

test -f "${BUILDDIR}/.build.log" && rm -f ${BUILDDIR}/.build.log
touch ${BUILDDIR}/.build.log
# log everything from here to .build.log
exec 1> >(exec -a 'build logging tee' tee -a ${BUILDDIR}/.build.log) 2>&1
echo "${HOST} started build at $(date)."
echo

configure ${OPTIONS} "$@"

if [ -n "${DOMAKE}" ]; then
	test -n "${DOVERBOSE}" && compile VERBOSE=1 || compile
fi

DOT=$(which dot 2>/dev/null)
if [ -n "${DOT}" ]; then
	${DOT} -Tpng -o${BUILDDIR}/wengophone.png ${BUILDDIR}/wengophone.dot
	${DOT} -Tsvg -o${BUILDDIR}/wengophone.svg ${BUILDDIR}/wengophone.dot
fi

exec >&0 2>&0		# so that the logging tee finishes
sleep 1			# wait till tee terminates

cleanup_and_exit 0

