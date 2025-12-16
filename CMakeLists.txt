cmake_minimum_required(VERSION 3.24)

project(macassistant_native
  VERSION 0.1.0
  LANGUAGES C CXX
)

option(MM_BUILD_UNIVERSAL "Build universal binary (arm64+x86_64) on macOS" OFF)
option(MM_BUILD_SHARED    "Build shared libs instead of static" OFF)

if(APPLE)
  set(CMAKE_OSX_DEPLOYMENT_TARGET "12.0" CACHE STRING "Minimum macOS version")
  if(MM_BUILD_UNIVERSAL)
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "" FORCE)
  endif()
endif()

if(MM_BUILD_SHARED)
  set(_MM_LIB_TYPE SHARED)
else()
  set(_MM_LIB_TYPE STATIC)
endif()

# Predictable output folders
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIG>/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIG>/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIG>/bin)

add_subdirectory(macmanager/core)
add_subdirectory(macmanager/bridge)
add_subdirectory(tools/cli)
