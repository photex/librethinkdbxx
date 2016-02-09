
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(TESTSUITE_SOURCES)

if (${PYTHON_MAJOR_VERSION} MATCHES "3")
  set(YAML_TO_CXX ${CMAKE_CURRENT_SOURCE_DIR}/yaml_to_cxx.py)
  set(UPSTREAM_TESTS_DIR ${CMAKE_CURRENT_BINARY_DIR}/upstream)
  file(GLOB_RECURSE UPSTREAM_YAML_FILES ${CMAKE_CURRENT_SOURCE_DIR}/upstream *.yaml)
  execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${UPSTREAM_TESTS_DIR})
  foreach(TEST_YAML ${UPSTREAM_YAML_FILES})
    get_filename_component(TEST_YAML_BASENAME ${TEST_YAML} NAME)
    set(UPSTREAM_TEST_SOURCE ${UPSTREAM_TESTS_DIR}/${TEST_YAML_BASENAME}.cc)
    execute_process(
      COMMAND
      ${PYTHON_EXECUTABLE} ${YAML_TO_CXX} ${TEST_YAML}
      COMMENT Generating test source from ${TEST_YAML}
      OUTPUT_FILE ${UPSTREAM_TEST_SOURCE}
      ERROR_VARIABLE _ERRORS
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if (_ERRORS)
      message(FATAL_ERROR "Unable to generate upstream tests: ${_ERRORS}")
    endif()
    list(APPEND TESTSUITE_SOURCES ${UPSTREAM_TEST_SOURCE})
  endforeach()
endif()

execute_process(
  COMMAND
  ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/gen_index_cxx.py
  OUTPUT_FILE ${CMAKE_CURRENT_BINARY_DIR}/upstream.cc
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

add_executable(reql_testsuite
  test.cc
  testlib.cc
  testlib.h
  ${CMAKE_CURRENT_BINARY_DIR}/upstream.cc
  ${TESTSUITE_SOURCES}
  )
target_link_libraries(reql_testsuite rethinkdbxx)
set_target_properties(reql_testsuite PROPERTIES CXX_STANDARD 14 CXX_STANDARD_REQUIRED ON)

add_test(RethinkDBXX_Upstream_Test_Suite
  reql_testsuite
  )

# TODO: Maybe get a broader testsuite together here without requiring code generation.