include(ExternalProject)
include(Bin2h)

FIND_PACKAGE(libstatgrab)

IF(NOT LIBSTATGRAB_FOUND)
  set(configure "./configure")
  list(APPEND configure "--disable-man")
  list(APPEND configure "--prefix=${CMAKE_BINARY_DIR}/libstatgrab")

  MESSAGE(STATUS "")
  MESSAGE(STATUS "StatGrab not found. Fetching content inplace.")
  MESSAGE(STATUS "")

  ExternalProject_Add(
    statgrab
    GIT_REPOSITORY https://github.com/libstatgrab/libstatgrab.git
    GIT_SHALLOW TRUE
    BUILD_IN_SOURCE 1
    UPDATE_COMMAND ""
    CONFIGURE_COMMAND "./autogen.sh" COMMAND ${CMAKE_COMMAND} -E env ${configure}
  )
  INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR}/libstatgrab/include)
  SET(LIBSTATGRAB_LIBRARY "${CMAKE_BINARY_DIR}/libstatgrab/lib/libstatgrab.a")
ENDIF()

IF(APPLE)
  # statgrab dependencies:
  FIND_LIBRARY(IO_KIT IOKit)
  FIND_LIBRARY(CORE_FOUNDATION CoreFoundation)
ELSEIF(UNIX)
  SET(Extra_LIBRARIES crypt)
ENDIF()

BIN2H(SOURCE_FILE "resources/templates/system-monitor.txt" HEADER_FILE "${CMAKE_BINARY_DIR}/templates/system-monitor.h" VARIABLE_NAME "SYSTEM_MONITOR_TPL")

FILE(GLOB GAMESERVER_REST_SOURCE_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
  *.C
  webui/*.C
  rest/*.C
  db/*.C
  sysmon/*.C
  common/*.C
  config/*.c config/*.C
  resources/*.css
)

ADD_EXECUTABLE(gameserver ${GAMESERVER_REST_SOURCE_FILES})
TARGET_INCLUDE_DIRECTORIES(gameserver PRIVATE lua)
TARGET_LINK_LIBRARIES(gameserver
  wt wthttp wtdbosqlite3 ${WT_THREAD_LIB}
  ${Boost_LIBRARIES}
  ${Extra_LIBRARIES}
  ${LIBSTATGRAB_LIBRARY}
  ${IO_KIT} ${CORE_FOUNDATION}
)
