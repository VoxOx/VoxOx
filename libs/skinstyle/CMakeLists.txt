project(owskinstyle)

include(${QT_USE_FILE})

# needed include directories to build owskinstyle
# saves the variable in internal cache for later use
set(OWSKINSTYLE_INCLUDE_DIRS
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${OWCUTIL_INCLUDE_DIRS}
  ${OWQTUTIL_INCLUDE_DIRS}
  ${QT_INCLUDE_DIR}
  CACHE INTERNAL "owskinstyle include directories"
)

set(OWSKINSTYLE_LIBRARY
  owskinstyle
  CACHE INTERNAL "owskinstyle library"
)

# owskinstyle lib and dependencies
set(OWSKINSTYLE_LINK_LIBRARIES
  ${OWSKINSTYLE_LIBRARY}
  ${OWCUTIL_LIBRARY}
  ${OWQTUTIL_LIBRARY}
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
)

qt4_wrap_cpp(owskinstyle_SRCS
  src/SkinStyle.h
  src/SkinInfoReader.h
  src/SkinWidget.h
  src/SkinWindow.h
  src/SkinButton.h
)

set(owskinstyle_SRCS
  ${owskinstyle_SRCS}
  src/SkinStyle.cpp
  src/SkinInfoReader.cpp
  src/SkinWidget.cpp
  src/SkinWindow.cpp
  src/SkinButton.cpp
)

include_directories(
  ${OWSKINSTYLE_INCLUDE_DIRS}
)
add_library(${OWSKINSTYLE_LIBRARY} STATIC ${owskinstyle_SRCS})

target_link_libraries(${OWSKINSTYLE_LINK_LIBRARIES})

