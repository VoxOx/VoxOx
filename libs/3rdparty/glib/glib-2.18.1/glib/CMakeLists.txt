ow_create_shared_library(glib)


ow_use_private_libraries(
	libintl
)

ow_add_private_libraries(
		ws2_32
)

ow_add_public_include_dirs(
	${CMAKE_CURRENT_SOURCE_DIR}/
	${CMAKE_CURRENT_SOURCE_DIR}/..
	
)

ow_add_private_include_dirs(
	${CMAKE_CURRENT_SOURCE_DIR}/..
	${CMAKE_CURRENT_SOURCE_DIR}/../config
)

ow_add_private_definitions(
	-DWIN32
	-D_WINDOWS
	-D_USRDLL
	-DGLIB_COMPILATION
	-DDLL_EXPORT
	-DHAVE_CONFIG_H
	-DG_DISABLE_DEPRECATED
	-DPCRE_STATIC
	-DLINK_SIZE=2
	-DMAX_NAME_SIZE=32
	-DMAX_NAME_COUNT=10000
	-DNEWLINE=-1
	-DPOSIX_MALLOC_THRESHOLD=10
	-DMATCH_LIMIT=10000000
	-DMATCH_LIMIT_RECURSION=10000000
	-FI "msvc_recommended_pragmas.h"	
)

ow_add_private_definitions(
	-DLIBDIR=\\\\\\\\\\\\\\\\\\\\"/irrelevant/lib\\\\\\\\\\\\\\\\\\"
#	-DG_LOG_DOMAIN=\\\\\\\\\\\\\\\\\\\\"Glib\\\\\\\\\\\\\\\\\\\\"
	-DG_LOG_DOMAIN=\\\\\\\\\\\\\\\\\\\\"0\\\\\\\\\\\\\\\\\\\\"

)

if (CMAKE_BUILD_TYPE MATCHES Debug)
ow_add_private_definitions(
	-DG_ENABLE_DEBUG	
)
endif (CMAKE_BUILD_TYPE MATCHES Debug)
	
#configure_file(
#	${CMAKE_CURRENT_SOURCE_DIR}/glib.rc
#	${BUILD_DIR}/glib.rc
#)

ow_add_sources(
	glib.def
	
	gnulib/asnprintf.c
	gnulib/printf-args.c
	gnulib/printf-parse.c
	gnulib/printf.c
	gnulib/vasnprintf.c
	libcharset/localcharset.c
	pcre/pcre_chartables.c
	pcre/pcre_compile.c
	pcre/pcre_config.c
	pcre/pcre_dfa_exec.c
	pcre/pcre_exec.c
	pcre/pcre_fullinfo.c
	pcre/pcre_get.c
	pcre/pcre_globals.c
	pcre/pcre_info.c
	pcre/pcre_maketables.c
	pcre/pcre_newline.c
	pcre/pcre_ord2utf8.c
	pcre/pcre_refcount.c
	pcre/pcre_study.c
	pcre/pcre_tables.c
	pcre/pcre_try_flipped.c
	pcre/pcre_ucp_searchfuncs.c
	pcre/pcre_valid_utf8.c
	pcre/pcre_version.c
	pcre/pcre_xclass.c
	garray.c
	gasyncqueue.c
	gatomic.c
	gbacktrace.c
	gbase64.c
	gbookmarkfile.c
	gcache.c
	gchecksum.c
	gcompletion.c
	gconvert.c
	gdataset.c
	gdate.c
	gdir.c
	gerror.c
	gfileutils.c
	ghash.c
	ghook.c
	giochannel.c
	giowin32.c
	gkeyfile.c
	glist.c
	gmain.c
	gmappedfile.c
	gmarkup.c
	gmem.c
	gmessages.c
	gnode.c
	goption.c
	gpattern.c
	gprimes.c
	gprintf.c
	gqsort.c
	gqueue.c
	grand.c
	gregex.c
	grel.c
	gscanner.c
	gsequence.c
	gshell.c
	gslice.c
	gslist.c
	gspawn-win32.c
	gstdio.c
	gstrfuncs.c
	gstring.c
	gtestutils.c
	gthread.c
	gthreadpool.c
	gtimer.c
	gtree.c
	gunibreak.c
	gunicollate.c
	gunidecomp.c
	guniprop.c
	gurifuncs.c
	gutf8.c
	gutils.c
	gwin32.c
)

ow_create_project_binary()