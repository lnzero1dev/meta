# - THIS FILE HAS BEEN GENERATED - DO NOT EDIT
# - To regenerate, please use the meta tool.

cmake_minimum_required(VERSION 3.10)

set(PROJECT_ROOT_DIR "/home/ema/checkout/serenity")

find_program(PATCH_EXE patch)
if(PATCH_EXE-NOTFOUND)
    warning_message("Did not find program patch")
endif()

find_program(BUILD-GENERATOR_EXE cmake)
if(BUILD-GENERATOR_EXE-NOTFOUND)
    warning_message("Did not find program cmake")
endif()

find_program(CXX_EXE g++)
if(CXX_EXE-NOTFOUND)
    warning_message("Did not find program g++")
endif()

find_program(CC_EXE gcc)
if(CC_EXE-NOTFOUND)
    warning_message("Did not find program gcc")
endif()

find_program(BUILD_EXE make)
if(BUILD_EXE-NOTFOUND)
    warning_message("Did not find program make")
endif()

find_program(AS_EXE as)
if(AS_EXE-NOTFOUND)
    warning_message("Did not find program as")
endif()

find_program(RANLIB_EXE ranlib)
if(RANLIB_EXE-NOTFOUND)
    warning_message("Did not find program ranlib")
endif()

find_program(INSTALL_EXE make)
if(INSTALL_EXE-NOTFOUND)
    warning_message("Did not find program make")
endif()

find_program(AR_EXE ar)
if(AR_EXE-NOTFOUND)
    warning_message("Did not find program ar")
endif()

find_program(DOWNLOAD_EXE wget)
if(DOWNLOAD_EXE-NOTFOUND)
    warning_message("Did not find program wget")
endif()

find_program(LINK_EXE ld)
if(LINK_EXE-NOTFOUND)
    warning_message("Did not find program ld")
endif()


include(ExternalProject)
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES sysroot)
# Host package: binutils
ExternalProject_Add(binutils
    PREFIX ${CMAKE_BINARY_DIR}/binutils
    URL http://ftp.gnu.org/gnu/binutils/binutils-2.33.1.tar.xz
    URL_HASH MD5=9406231b7d9dd93731c2d06cefe8aaf1
    DOWNLOAD_DIR ${CMAKE_BINARY_DIR}/download
    PATCH_COMMAND ${PATCH_EXE} -p1 --forward < ${PROJECT_ROOT_DIR}/Toolchain/Patches/binutils.patch || true
    CONFIGURE_COMMAND ${CMAKE_BINARY_DIR}/binutils/src/binutils/configure
      --prefix=${CMAKE_BINARY_DIR}/sysroot
      --with-sysroot=${CMAKE_BINARY_DIR}/sysroot
      --target=i686-pc-serenity
      --enable-shared
      --disable-nls
    BUILD_COMMAND $(MAKE)
    INSTALL_COMMAND $(MAKE) install
)

set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES binutils)

# Target package: LibC
ExternalProject_Add(LibC
    PREFIX ${CMAKE_BINARY_DIR}/LibC
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/../package/LibC
    CMAKE_ARGS
      -DCMAKE_SYSROOT=${CMAKE_BINARY_DIR}/sysroot
      -DCMAKE_TOOLCHAIN_FILE=${CMAKE_CURRENT_LIST_DIR}/serenity.cmake
    BINARY_DIR ${CMAKE_BINARY_DIR}/LibC
    INSTALL_COMMAND DESTDIR=${CMAKE_BINARY_DIR}/sysroot $(MAKE) install)
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES LibC)

# Target package: LibM
ExternalProject_Add(LibM
    DEPENDS LibC 
    PREFIX ${CMAKE_BINARY_DIR}/LibM
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/../package/LibM
    CMAKE_ARGS
      -DCMAKE_SYSROOT=${CMAKE_BINARY_DIR}/sysroot
      -DCMAKE_TOOLCHAIN_FILE=${CMAKE_CURRENT_LIST_DIR}/serenity.cmake
    BINARY_DIR ${CMAKE_BINARY_DIR}/LibM
    INSTALL_COMMAND DESTDIR=${CMAKE_BINARY_DIR}/sysroot $(MAKE) install)
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES LibM)

# Host package: gcc
ExternalProject_Add(gcc
    DEPENDS binutils 
    PREFIX ${CMAKE_BINARY_DIR}/gcc
    URL http://ftp.gnu.org/gnu/gcc/gcc-9.2.0/gcc-9.2.0.tar.xz
    URL_HASH MD5=3818ad8600447f05349098232c2ddc78
    DOWNLOAD_DIR ${CMAKE_BINARY_DIR}/download
    PATCH_COMMAND ${PATCH_EXE} -p1 --forward < ${PROJECT_ROOT_DIR}/Toolchain/Patches/gcc.patch || true
    CONFIGURE_COMMAND ${CMAKE_BINARY_DIR}/gcc/src/gcc/configure
      --prefix=${CMAKE_BINARY_DIR}/sysroot
      --with-sysroot=${CMAKE_BINARY_DIR}/sysroot
      --target=i686-pc-serenity
      --disable-nls
      --with-newlib
      --enable-shared
      --enable-languages=c,c++
    BUILD_COMMAND $(MAKE) all-gcc all-target-libgcc
    INSTALL_COMMAND $(MAKE) install-gcc install-target-libgcc
)

set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES gcc)

# Host package: libstdc++-v3
ExternalProject_Add(libstdc++-v3
    DEPENDS binutils LibM gcc LibC 
    PREFIX ${CMAKE_BINARY_DIR}/libstdc++-v3
    URL http://ftp.gnu.org/gnu/gcc/gcc-9.2.0.tar.xz
    URL_HASH MD5=3818ad8600447f05349098232c2ddc78
    DOWNLOAD_DIR ${CMAKE_BINARY_DIR}/download
    PATCH_COMMAND ${PATCH_EXE} -p1 --forward < ${PROJECT_ROOT_DIR}/Toolchain/Patches/gcc.patch || true
    CONFIGURE_COMMAND ${CMAKE_BINARY_DIR}/libstdc++-v3/src/libstdc++-v3/configure
      --prefix=${CMAKE_BINARY_DIR}/sysroot
      --with-sysroot=${CMAKE_BINARY_DIR}/sysroot
      --target=i686-pc-serenity
      --disable-nls
      --with-newlib
      --enable-shared
      --enable-languages=c,c++
    BUILD_COMMAND $(MAKE) all-target-libstdc++-v3
    INSTALL_COMMAND $(MAKE) install-target-libstdc++-v3
)

set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES libstdc++-v3)

# Target package: AK
ExternalProject_Add(AK
    DEPENDS LibC 
    PREFIX ${CMAKE_BINARY_DIR}/AK
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/../package/AK
    CMAKE_ARGS
      -DCMAKE_SYSROOT=${CMAKE_BINARY_DIR}/sysroot
      -DCMAKE_TOOLCHAIN_FILE=${CMAKE_CURRENT_LIST_DIR}/serenity.cmake
    BINARY_DIR ${CMAKE_BINARY_DIR}/AK
    INSTALL_COMMAND DESTDIR=${CMAKE_BINARY_DIR}/sysroot $(MAKE) install)
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES AK)

# Target package: LibCore
ExternalProject_Add(LibCore
    PREFIX ${CMAKE_BINARY_DIR}/LibCore
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/../package/LibCore
    CMAKE_ARGS
      -DCMAKE_SYSROOT=${CMAKE_BINARY_DIR}/sysroot
      -DCMAKE_TOOLCHAIN_FILE=${CMAKE_CURRENT_LIST_DIR}/serenity.cmake
    BINARY_DIR ${CMAKE_BINARY_DIR}/LibCore
    INSTALL_COMMAND DESTDIR=${CMAKE_BINARY_DIR}/sysroot $(MAKE) install)
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES LibCore)

# Host package: IPCCompiler
ExternalProject_Add(IPCCompiler
    DEPENDS AK LibCore 
    PREFIX ${CMAKE_BINARY_DIR}/IPCCompiler
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/../package/IPCCompiler
    CMAKE_ARGS
      -DCMAKE_SYSROOT=${CMAKE_BINARY_DIR}/sysroot
      -DCMAKE_TOOLCHAIN_FILE=${CMAKE_CURRENT_LIST_DIR}/serenity.cmake
    BINARY_DIR ${CMAKE_BINARY_DIR}/IPCCompiler
    INSTALL_COMMAND DESTDIR=${CMAKE_BINARY_DIR}/sysroot $(MAKE) install)
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES IPCCompiler)

