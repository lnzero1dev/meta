# - THIS FILE HAS BEEN GENERATED - DO NOT EDIT
# - To regenerate, please use the meta tool.

if(LOADED)
     return()
endif()
set(LOADED true)

if(NOT CMAKE_BUILD_TYPE)
     set(CMAKE_BUILD_TYPE Debug)
endif()

set(CMAKE_EXE_LINKER_FLAGS "" CACHE INTERNAL "" FORCE)
set(CMAKE_SHARED_LINKER_FLAGS "" CACHE INTERNAL "" FORCE)
set(CMAKE_MODULE_LINKER_FLAGS "" CACHE INTERNAL "" FORCE)

set(CMAKE_CXX_COMPILER i686-pc-serenity-g++)
set(CMAKE_CXX_FLAGS " -MMD -MP -std=c++17 -Wno-sized-deallocation -fno-sized-deallocation -Werror -Wextra -Wall -Wno-nonnull-compare -Wno-deprecated-copy -Wno-address-of-packed-member -Wundef -Wcast-qual -Wwrite-strings -Wimplicit-fallthrough -Wno-expansion-to-defined -fno-exceptions -fno-rtti -fstack-protector" CACHE STRING "" FORCE)

set(CMAKE_C_COMPILER i686-pc-serenity-gcc)
set(CMAKE_C_FLAGS " -MMD -MP -Werror -Wextra -Wall -Wno-nonnull-compare -Wno-address-of-packed-member -Wundef -Wcast-qual -Wwrite-strings -Wimplicit-fallthrough -Wno-expansion-to-defined -fstack-protector" CACHE STRING "" FORCE)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
set(CMAKE_INSTALL_PREFIX "/usr")

if(CMAKE_BUILD_TYPE STREQUAL "DebWithRelInfo")
endif()
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Os -DRELEASE" CACHE INTERNAL "" FORCE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Os -DRELEASE" CACHE INTERNAL "" FORCE)
endif()
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Og -DDEBUG -DSANITIZE_PTRS" CACHE INTERNAL "" FORCE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Og -DDEBUG -DSANITIZE_PTRS" CACHE INTERNAL "" FORCE)
endif()
if(CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
endif()
