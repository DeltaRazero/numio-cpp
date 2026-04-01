cmake_minimum_required(VERSION 3.14)

# ******************************************************************************

# CPM dependency manager.
file(
  DOWNLOAD
  https://github.com/cpm-cmake/CPM.cmake/releases/latest/download/CPM.cmake
  # ${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake
  ${CMAKE_CURRENT_SOURCE_DIR}/.cpm/CPM.cmake
)
include(${CMAKE_CURRENT_SOURCE_DIR}/.cpm/CPM.cmake)

# ******************************************************************************

CPMAddPackage(NAME googletest
  GITHUB_REPOSITORY
    google/googletest
  GIT_TAG
    v1.17.x
  OPTIONS
    "INSTALL_GTEST OFF"
    # For Windows: prevent GTest from overriding the compiler/linker settings.
    "gtest_force_shared_crt"
)

# ******************************************************************************

file(GLOB SOURCE_FILES
  ./src/*.cpp
)

# Define the test executable.
add_executable(unit_tests
  ${SOURCE_FILES}
)

# Link to the library and GTest.
# GTest provides 'gtest_main' which includes a default main() function.
target_link_libraries(unit_tests
  PRIVATE
    numio::numio
    gtest_main
)

# Allow CTest to discover the tests automatically.
include(GoogleTest)
gtest_discover_tests(unit_tests)
