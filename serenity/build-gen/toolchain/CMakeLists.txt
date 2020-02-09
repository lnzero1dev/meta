cmake_minimum_required(VERSION 3.10)

set(PROJECT_ROOT_DIR "/home/ema/checkout/serenity")

find_program(PATCH_EXE NAMES patch)

include(ExternalProject)

# TODO: check if patch has changed or new patches have been added... ?
# set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS

# cmake-format: off

set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "sysroot")

ExternalProject_Add(
  binutils
  PREFIX ${CMAKE_BINARY_DIR}/binutils
  URL http://ftp.gnu.org/gnu/binutils/binutils-2.33.1.tar.xz
  URL_HASH MD5=9406231b7d9dd93731c2d06cefe8aaf1
  DOWNLOAD_DIR ${CMAKE_BINARY_DIR}/download
  PATCH_COMMAND ${PATCH_EXE} -p1 --forward < ${PROJECT_ROOT_DIR}/Toolchain/Patches/binutils.patch || true
  CONFIGURE_COMMAND ${CMAKE_BINARY_DIR}/binutils/src/binutils/configure
                            --prefix=${CMAKE_BINARY_DIR}/sysroot
                            --target=i686-pc-serenity
                            --with-sysroot=${CMAKE_BINARY_DIR}/sysroot
                            --enable-shared
                            --disable-nls
  BUILD_COMMAND $(MAKE) ${PARALLEL_BUILD}
  INSTALL_COMMAND $(MAKE) install)
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "binutils")

ExternalProject_Add(
  gcc
  DEPENDS binutils
  PREFIX ${CMAKE_BINARY_DIR}/gcc
  URL https://ftp.gnu.org/gnu/gcc/gcc-9.2.0/gcc-9.2.0.tar.xz
  URL_HASH MD5=3818ad8600447f05349098232c2ddc78
  DOWNLOAD_DIR ${CMAKE_BINARY_DIR}/download
  PATCH_COMMAND ${PATCH_EXE} -p1 --forward < ${PROJECT_ROOT_DIR}/Toolchain/Patches/gcc.patch || true
  CONFIGURE_COMMAND ${CMAKE_BINARY_DIR}/gcc/src/gcc/configure
                            --target=i686-pc-serenity
                            --prefix=${CMAKE_BINARY_DIR}/sysroot
                            --with-sysroot=${CMAKE_BINARY_DIR}/sysroot
                            --disable-nls
                            --with-newlib
                            --enable-shared
                            --enable-languages=c,c++
  BUILD_COMMAND $(MAKE) ${PARALLEL_BUILD} all-gcc all-target-libgcc
  INSTALL_COMMAND $(MAKE) install-gcc install-target-libgcc)
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "gcc")

ExternalProject_Add(
  libc
  DEPENDS gcc
  PREFIX ${CMAKE_BINARY_DIR}/libc
  SOURCE_DIR ${CMAKE_SOURCE_DIR}/../packages/libc
  CMAKE_ARGS
    -DCMAKE_SYSROOT=${CMAKE_BINARY_DIR}/sysroot
    -DCMAKE_TOOLCHAIN_FILE=${CMAKE_CURRENT_LIST_DIR}/serenity.cmake
  BINARY_DIR ${CMAKE_BINARY_DIR}/libc
  INSTALL_COMMAND DESTDIR=${CMAKE_BINARY_DIR}/sysroot $(MAKE) install)
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "libc")

ExternalProject_Add(
  libm
  DEPENDS gcc
  PREFIX ${CMAKE_BINARY_DIR}/libm
  SOURCE_DIR ${CMAKE_SOURCE_DIR}/../packages/libm
  CMAKE_ARGS
    -DCMAKE_SYSROOT=${CMAKE_BINARY_DIR}/sysroot
    -DCMAKE_TOOLCHAIN_FILE=${CMAKE_CURRENT_LIST_DIR}/serenity.cmake
  BINARY_DIR ${CMAKE_BINARY_DIR}/libm
  INSTALL_COMMAND DESTDIR=${CMAKE_BINARY_DIR}/sysroot $(MAKE) install)
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "libm")

ExternalProject_Add(
  libstdc++3
  DEPENDS gcc libc libm
  PREFIX ${CMAKE_BINARY_DIR}/libstdc++3
  URL https://ftp.gnu.org/gnu/gcc/gcc-9.2.0/gcc-9.2.0.tar.xz
  URL_HASH MD5=3818ad8600447f05349098232c2ddc78
  DOWNLOAD_DIR ${CMAKE_BINARY_DIR}/download
  PATCH_COMMAND ${PATCH_EXE} -p1 --forward < ${PROJECT_ROOT_DIR}/Toolchain/Patches/gcc.patch || true
  CONFIGURE_COMMAND ${CMAKE_BINARY_DIR}/libstdc++3/src/libstdc++3/configure
                            --target=i686-pc-serenity
                            --prefix=${CMAKE_BINARY_DIR}/sysroot
                            --with-sysroot=${CMAKE_BINARY_DIR}/sysroot
                            --disable-nls
                            --with-newlib
                            --enable-shared
                            --enable-languages=c,c++
  BUILD_COMMAND $(MAKE) ${PARALLEL_BUILD} all-target-libstdc++-v3
  INSTALL_COMMAND $(MAKE) install-target-libstdc++-v3)
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "libstdc++3")

# cmake-format: on
