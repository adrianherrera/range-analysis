cmake_minimum_required(VERSION 3.10.2)

project(RangeAnalysis)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(
  -Werror
  -Wall
  -Wextra
  -Wreturn-type
  -Wimplicit-fallthrough
)

find_package(LLVM 14 REQUIRED)
message(STATUS "Found LLVM ${LLVM_CMAKE_DIR}")

list(APPEND CMAKE_MODULE_PATH ${LLVM_CMAKE_DIR})
include(AddLLVM)

if(NOT LLVM_ENABLE_RTTI)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
endif(NOT LLVM_ENABLE_RTTI)

include_directories(SYSTEM include ${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})

add_subdirectory(lib)
add_subdirectory(tools)
