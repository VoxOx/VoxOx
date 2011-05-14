# - <package>-config module for CMake
#
# Defines the following macros:
#
# CONFIG(package-config includedir libdir linkflags cflags)
#

MACRO(CONFIG _package-config _link_FLAGS _cflags)
  FIND_PROGRAM(CONFIG_EXECUTABLE
    NAMES
      ${_package-config}
    PATHS
    /usr/bin
    /usr/local/bin
    /opt/kde3/bin
    /opt/kde4/bin
    /opt/gnome/bin
  )
# reset the variables at the beginning
  SET(${_link_FLAGS})
  SET(${_cflags})

  # if pkg-config has been found
  IF(CONFIG_EXECUTABLE)

    EXECUTE_PROCESS(
        COMMAND
          ${CONFIG_EXECUTABLE} --cflags
        OUTPUT_VARIABLE
          _config_cflags_OUTPUT_VARIABLE
    )

    EXECUTE_PROCESS(
        COMMAND
          ${CONFIG_EXECUTABLE} --libs
        OUTPUT_VARIABLE
          _config_libs_OUTPUT_VARIABLE
    )

    SET(_link_FLAGS ${_config_libs_OUTPUT_VARIABLE})
    SET(_cflags ${_config_cflags_OUTPUT_VARIABLE})

  ENDIF(CONFIG_EXECUTABLE)

ENDMACRO(CONFIG _link_FLAGS _cflags)

