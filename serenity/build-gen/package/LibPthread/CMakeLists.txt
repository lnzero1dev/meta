# - THIS FILE HAS BEEN GENERATED - DO NOT EDIT
# - To regenerate, please use the meta tool.

cmake_minimum_required(VERSION 3.10)

set(PROJECT_ROOT_DIR "/home/ema/checkout/serenity")

set(SOURCES
    "/home/ema/checkout/serenity/Libraries/LibPthread/pthread.cpp"
)
set(INCLUDE_DIRS
    "${PROJECT_ROOT_DIR}/Libraries/LibPthread"
)
set(STATIC_LINK_LIBRARIES
    "c"
)
add_library(pthread STATIC ${SOURCES})
target_include_directories(pthread PUBLIC ${INCLUDE_DIRS})
target_link_libraries(pthread PUBLIC ${STATIC_LINK_LIBRARIES})

