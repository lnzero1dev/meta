cmake_minimum_required(VERSION 3.10)

set(PROJECT_ROOT_DIR "/home/ema/checkout/serenity")

if(BUILD_NATIVE_TOOLCHAIN)
  message(STATUS "BUILD_NATIVE_TOOLCHAIN=${BUILD_NATIVE_TOOLCHAIN}")
  add_subdirectory(toolchain)
endif()

add_subdirectory(images/default-image)
