file(GLOB TEST_SRC "*.cpp" "*.hpp" "*.h")
file(GLOB TEST_HEADERS "*.hpp" "*.h")

set(TEST_NAME "experimental")

message(STATUS "files....." ${TEST_SRC})

pybind11_add_module(${TEST_NAME} SHARED ${TEST_SRC})

