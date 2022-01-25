include(ExternalProject)

set(configure "../statgrab/configure")
list(APPEND configure "--disable-man")
list(APPEND configure "--prefix=${CMAKE_BINARY_DIR}/libstatgrab")

ExternalProject_Add(
  statgrab
  GIT_REPOSITORY https://github.com/libstatgrab/libstatgrab.git
  GIT_SHALLOW TRUE
  UPDATE_COMMAND "./autogen.sh"
  CONFIGURE_COMMAND ${CMAKE_COMMAND} -E env ${configure}
)

FILE(GLOB GAMESERVER_REST_SOURCE_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
  *.C
  webui/*.C
  rest/*.C
  db/*.C
  common/*.C
  resources/*.css
)

ADD_EXECUTABLE(gameserver ${GAMESERVER_REST_SOURCE_FILES})
TARGET_LINK_LIBRARIES(gameserver
  ${Boost_LIBRARIES}
  wt wthttp wtdbosqlite3 ${WT_THREAD_LIB}
)
ADD_DEPENDENCIES(gameserver statgrab)
