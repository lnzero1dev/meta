# - THIS FILE HAS BEEN GENERATED - DO NOT EDIT
# - To regenerate, please use the meta tool.

cmake_minimum_required(VERSION 3.10)

set(PROJECT_ROOT_DIR "/home/ema/checkout/serenity")

set(SOURCES
    "/home/ema/checkout/serenity/DevTools/IPCCompiler/main.cpp"
)
set(INCLUDE_DIRS
    "${PROJECT_ROOT_DIR}/DevTools/IPCCompiler"
)
set(STATIC_LINK_LIBRARIES
    "AK"
    "core"
)
add_executable(IPCCompiler STATIC ${SOURCES})
target_include_directories(IPCCompiler PUBLIC ${INCLUDE_DIRS})
target_link_libraries(IPCCompiler PUBLIC ${STATIC_LINK_LIBRARIES})

