ow_create_shared_library(gobject)

ow_use_private_libraries(
	glib
)

ow_add_public_include_dirs(	
	${CMAKE_CURRENT_SOURCE_DIR}/
)

ow_add_private_include_dirs(
	${CMAKE_CURRENT_SOURCE_DIR}/../config
)

ow_add_private_definitions(
	-DHAVE_CONFIG_H
	-DG_LOG_DOMAIN=\\\\\\\\\\\\\\\\\\\\"Glib-GObject\\\\\\\\\\\\\\\\\\\\"
	-DG_DISABLE_DEPRECATED
	-DGOBJECT_COMPILATION
)

ow_add_sources(
	gobject.def
	gboxed.c
	gclosure.c
	genums.c
	gobject.c
	gparam.c
	gparamspecs.c
	gsignal.c
	gsourceclosure.c
	gtype.c
	gtypemodule.c
	gtypeplugin.c
	gvalue.c
	gvaluearray.c
	gvaluetransform.c
	gvaluetypes.c
)

ow_create_project_binary()

