cmake_minimum_required(VERSION 3.2)

project(rethinkdbxx CXX)

option(ENABLE_TESTS OFF)

set(CMAKE_CXX_FLAGS "-Wall -fPIC")
set(CMAKE_CXX_FLAGS_MINSIZEREL "-flto")

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set(USE_XLOCALE_H ON)
else()
  set(USE_XLOCALE_H OFF)
endif()

if (NOT PYTHON_EXECUTABLE)
  set(PYTHON_EXECUTABLE /usr/bin/python)
endif()

execute_process(
  COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/reql/gen.py ${CMAKE_CURRENT_SOURCE_DIR}/reql/ql2.proto
  OUTPUT_FILE ${CMAKE_CURRENT_BINARY_DIR}/protocol_defs.h
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

configure_file(src/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

set(GENERATED_HEADERS
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
  ${CMAKE_CURRENT_BINARY_DIR}/protocol_defs.h
  )

include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(LIB_SOURCES
  src/utils.cc
  src/types.cc
  src/query.cc
  src/net.cc
  src/json.cc
  src/datum.cc
  src/cursor.cc
  )

set(LIB_HEADERS
  src/utils.h
  src/types.h
  src/query.h
  src/net.h
  src/json.h
  src/datum.h
  src/cursor.h
  src/stream.h
  src/error.h
  )

add_library(rethinkdbxx STATIC
  ${LIB_HEADERS}
  ${LIB_SOURCES}
  )

set_target_properties(rethinkdbxx
  PROPERTIES
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED ON
  )

install(TARGETS rethinkdbxx
  INCLUDES DESTINATION include
  ARCHIVE DESTINATION lib
  )
install(FILES
  src/rethinkdb.hpp
  ${LIB_HEADERS}
  ${GENERATED_HEADERS}
  DESTINATION include
  )

if (ENABLE_TESTS)
  enable_testing()

  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
  include_directories(${CMAKE_CURRENT_BINARY_DIR})

  add_executable(test_connection "test/connection.cpp")
  target_link_libraries(test_connection rethinkdbxx)
  set_target_properties(test_connection
    PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON
    )

  add_test(RethinkDBXX_Tests
    test_connection
    )
endif()
