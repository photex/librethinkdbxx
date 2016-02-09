cmake_minimum_required(VERSION 3.2)

project(rethinkdbxx CXX)

option(ENABLE_TESTS OFF)
option(ENABLE_EXAMPLES OFF)

set(CMAKE_CXX_FLAGS "-Wall -fPIC")
set(CMAKE_CXX_FLAGS_MINSIZEREL "-flto")

if (NOT PYTHON_EXECUTABLE)
  find_program(PYTHON_EXECUTABLE NAMES python3 python3.4 python3.5 python PATHS /usr/bin /usr/local/bin)
  if (NOT PYTHON_EXECUTABLE)
    message(FATAL_ERROR "Unable to find any python interpreter.")
  endif()
endif()
execute_process(
  COMMAND ${PYTHON_EXECUTABLE} -c "import sys; print(sys.version_info[0])"
  OUTPUT_VARIABLE PYTHON_MAJOR_VERSION
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
if (${PYTHON_MAJOR_VERSION} MATCHES "2")
  if (ENABLE_TESTS)
    message(WARNING "This library requires python 3 to generate the upstream testsuite. Tests will not be complete.")
  else()
    # Not using a proper CMake warning here because it's pretty intrusive and
    # a developer in this situation very likely knows what's up.
    message(STATUS "WARNING: If you plan to run the testsuite you'll need to install python3")
  endif()
endif()
message(STATUS "Using Python Executable: ${PYTHON_EXECUTABLE}")

execute_process(
  COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/reql/gen.py ${CMAKE_CURRENT_SOURCE_DIR}/reql/ql2.proto
  OUTPUT_FILE ${CMAKE_CURRENT_BINARY_DIR}/protocol_defs.h
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set(USE_XLOCALE_H ON)
else()
  set(USE_XLOCALE_H OFF)
endif()
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
  src/rethinkdb.h
  )

add_library(rethinkdbxx STATIC
  ${LIB_HEADERS}
  ${LIB_SOURCES}
  )
target_include_directories(rethinkdbxx
  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_BINARY_DIR}
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
  src/rethinkdb.h
  ${LIB_HEADERS}
  ${GENERATED_HEADERS}
  DESTINATION include
  )

if (ENABLE_EXAMPLES)
  add_subdirectory(examples)
endif()

if (ENABLE_TESTS)
  enable_testing()
  add_subdirectory(test)
endif()
