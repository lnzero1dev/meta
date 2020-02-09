cmake_minimum_required(VERSION 3.10)

set(PROJECT_ROOT_DIR "/home/ema/checkout/serenity")

# cmake-format: off
add_defines(-DGIT_COMMIT=`git rev-parse --short HEAD` -DGIT_BRANCH=`git rev-parse --abbrev-ref HEAD` -DGIT_CHANGES=`git diff-index --quiet HEAD -- && echo "tracked"|| echo "untracked"`)
# cmake-format: on

if(BUILD_NATIVE_TOOLCHAIN)
  message(STATUS "BUILD_NATIVE_TOOLCHAIN=${BUILD_NATIVE_TOOLCHAIN}")
  add_subdirectory(toolchain)
endif()

add_subdirectory(images/default-image)
