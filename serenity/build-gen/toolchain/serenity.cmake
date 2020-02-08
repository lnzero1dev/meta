# set(CMAKE_SYSTEM_NAME serenity)

set(CMAKE_C_COMPILER i686-pc-serenity-gcc)
set(CMAKE_CXX_COMPILER i686-pc-serenity-g++)

set(CMAKE_CXX_FLAGS
    "-Os -MMD -MP -std=c++17 -Werror -Wextra -Wall -Wno-nonnull-compare -Wno-deprecated-copy -Wno-address-of-packed-member -Wundef -Wcast-qual -Wwrite-strings -Wimplicit-fallthrough -Wno-expansion-to-defined -fno-exceptions -fno-rtti -fstack-protector"
    CACHE STRING "" FORCE
)
set(CMAKE_C_FLAGS
    "-Os -MMD -MP -Werror -Wextra -Wall -Wno-nonnull-compare -Wno-address-of-packed-member -Wundef -Wcast-qual -Wwrite-strings -Wimplicit-fallthrough -Wno-expansion-to-defined -fstack-protector"
    CACHE STRING "" FORCE
)

set(CMAKE_EXE_LINKER_FLAGS
    ""
    CACHE INTERNAL ""
)
set(CMAKE_SHARED_LINKER_FLAGS
    ""
    CACHE INTERNAL ""
)
set(CMAKE_MODULE_LINKER_FLAGS
    ""
    CACHE INTERNAL ""
)

# important, otherwise we get errors during compiler check
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CMAKE_INSTALL_PREFIX "/usr")

add_definitions(-DSANITIZE_PTRS)
add_definitions(-DDEBUG)
