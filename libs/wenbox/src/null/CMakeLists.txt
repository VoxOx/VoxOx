project(nullwenbox)

# needed include directories to build nullwenbox
set(NULLWENBOX_INCLUDE_DIRS
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${OWUTIL_INCLUDE_DIRS}
  ${WENBOX_INCLUDE_DIRS}
)

set(NULLWENBOX_LIBRARY
  yealinkwenbox
)

# nullwenbox lib and dependencies
set(NULLWENBOX_LINK_LIBRARIES
  ${NULLWENBOX_LIBRARY}
  ${OWUTIL_LIBRARY}
  ${WENBOX_LIBRARY}
)

set(NULLWENBOX_DEFINITIONS
  -DWENBOX_DLL=1
  -DBUILDING_DLL=1
)

set(nullwenbox_SRCS
  NullWenbox.cpp
)

include_directories(
  ${NULLWENBOX_INCLUDE_DIRS}
)

add_definitions(
  ${NULLWENBOX_DEFINITIONS}
)

macro_add_plugin(${NULLWENBOX_LIBRARY} ${nullwenbox_SRCS})

target_link_libraries(${NULLWENBOX_LINK_LIBRARIES})

install(
  TARGETS
    ${NULLWENBOX_LIBRARY}
  DESTINATION
    ${PLUGIN_INSTALL_DIR}
)

