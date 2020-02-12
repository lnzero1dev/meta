# - THIS FILE HAS BEEN GENERATED - DO NOT EDIT
# - To regenerate, please use the meta tool.

cmake_minimum_required(VERSION 3.10)

set(PROJECT_ROOT_DIR "/home/ema/checkout/serenity")

set(SOURCES
    "/home/ema/checkout/serenity/Libraries/LibIPC/IEndpoint.cpp"
    "/home/ema/checkout/serenity/Libraries/LibIPC/IMessage.cpp"
)
set(INCLUDE_DIRS
    "${PROJECT_ROOT_DIR}/Libraries/LibIPC"
)
set(STATIC_LINK_LIBRARIES
)
add_library(ipc STATIC ${SOURCES})
target_include_directories(ipc PUBLIC ${INCLUDE_DIRS})
target_link_libraries(ipc PUBLIC ${STATIC_LINK_LIBRARIES})

