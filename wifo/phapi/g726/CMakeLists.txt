ow_create_static_library(g726)


ow_add_public_include_dirs(
	${CMAKE_CURRENT_SOURCE_DIR}/dummy
)

ow_add_private_definitions(
	-DHAVE_MATH_H=1
)

ow_add_sources(
	g726.c
    bitstream.c
    bit_operations.c
    bitstream.h
    bit_operations.h
    dc_restore.h
    spang711.h
    g726.h
    inttypes.h
    telephony.h
    tgmath.h
    unistd.h
)

ow_create_project_binary()
