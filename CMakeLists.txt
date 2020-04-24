cmake_minimum_required(VERSION 2.6)

project(libr)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(libr_VERSION_MAJOR 0)
set(libr_VERSION_MINOR 1)

set(CMAKE_BUILD_TYPE DEBUG)

############################################################

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

############################################################

# Options.
# Build tests by default. We want to run tests whether we are developing or building a release. I mean... why wouldn't you? :)
option(BUILD_TESTS "Build all tests." ON)
option(BUILD_COVERAGE "Build coverage." ON)
# This flag is used to avoid duplcation when this project is imported as part of a larger project which already has gtest and lcov.
#option(LOCAL_LIBR_BUILD "Used when building libr locally. Set to off for server builds." ON)

if(BUILD_TESTS)
  set(CMAKE_MODULES_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules)
  set(CMAKE_DOWNLOAD_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/download)

  include(CTest)
  enable_testing()

  if (NOT DEFINED googletest_SOURCE_DIR)
      MESSAGE("LIBR downloading google test.")
      if (CMAKE_VERSION VERSION_LESS 3.2)
        set(UPDATE_DISCONNECTED_IF_AVAILABLE "")
      else()
        set(UPDATE_DISCONNECTED_IF_AVAILABLE "UPDATE_DISCONNECTED 1")
      endif()

      include(${CMAKE_MODULES_DIRECTORY}/DownloadProject.cmake)
      download_project(   PROJ                googletest
              GIT_REPOSITORY      https://github.com/google/googletest.git
              GIT_TAG             master
              PREFIX              ${CMAKE_DOWNLOAD_DIRECTORY}/googletest
              ${UPDATE_DISCONNECTED_IF_AVAILABLE}
              )

      # Prevent GoogleTest from overriding our compiler/linker options
      # when building with Visual Studio
      set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
      add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})
  endif()

endif() # BUILD_TESTS

# Can't build for coverage without the test.
if (BUILD_COVERAGE AND BUILD_TESTS)
    if (NOT DEFINED lcov_SOURCE_DIR)
      # lcov doesn't build with CMake so we download, set lcov as an external target, then call make.
      # we then manually add the bin path so that we can call it later.
      download_project(   PROJ              lcov
              URL               https://github.com/linux-test-project/lcov/releases/download/v1.14/lcov-1.14.tar.gz
              PREFIX            ${CMAKE_DOWNLOAD_DIRECTORY}/lcov
              TIMEOUT           180
              ${UPDATE_DISCONNECTED_IF_AVAILABLE}
              )

      set(LCOV_PATH "${lcov_SOURCE_DIR}/bin/lcov")
      set(GENHTML_PATH "${lcov_SOURCE_DIR}/bin/genhtml")
    endif()

    include(${CMAKE_MODULES_DIRECTORY}/CodeCoverage.cmake)

    append_coverage_compiler_flags()
    set( COVERAGE_EXCLUDES
            "/usr/*"
            "*/googletest/*"
            "*/googlemock/*"
            "*test/*")
endif()

######################################

option(WITH_SGC "Compile the SGC library for memory leak detection" OFF)

set(SOURCES
  src/r.c 
  src/mem.c 
  src/thread_pthread.c 
  src/fs_posix.c 
  src/clock_posix.c 
  src/membuf.c
  src/list.c 
  src/log.c 
  src/util.c
  src/json.c
  src/serial_posix.c
  )

include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/include" )

add_library(r SHARED ${SOURCES})

target_include_directories(r
        PUBLIC
          "${CMAKE_CURRENT_SOURCE_DIR}/include"
          "${CMAKE_CURRENT_SOURCE_DIR}/include/r"
        )

if(WITH_SGC)
  target_link_libraries(r pthread m sgc)
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -D MEM_DIAGNOSTICS")
  set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_DEBUG} -D MEM_DIAGNOSTICS ")
else(WITH_SGC)
  target_link_libraries(r pthread m)
endif(WITH_SGC)

#if(BUILD_TESTS AND LOCAL_LIBR_BUILD)
if(BUILD_TESTS)
#  Comment to force a build.
  add_subdirectory(test)
endif()

INSTALL(TARGETS r DESTINATION "lib")
INSTALL (
    DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h")
