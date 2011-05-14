project(tinyxml)

# needed include directories to build tinyxml
# saves the variable in internal cache for later use
set(TINYXML_INCLUDE_DIRS
  ${CMAKE_CURRENT_SOURCE_DIR}
  CACHE INTERNAL "tinyxml include directories"
)

# tinyxml lib and dependencies
# saves the variable in internal cache for later use
set(TINYXML_LIBRARY
  tinyxml
  CACHE INTERNAL "tinyxml library"
)

set(TINYXML_LINK_LIBRARIES
  ${TINYXML_LIBRARY}
)

set(tinyxml_SRCS
  tinyxml.cpp
  tinyxmlerror.cpp
  tinyxmlparser.cpp
)


include_directories(
  ${TINYXML_INCLUDE_DIRS}
)

include_directories(${TINYXML_INCLUDE_DIRS})

add_library(${TINYXML_LIBRARY} STATIC ${tinyxml_SRCS})

