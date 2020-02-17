# - THIS FILE HAS BEEN GENERATED - DO NOT EDIT
# - To regenerate, please use the meta tool.

cmake_minimum_required(VERSION 3.10)

set(PROJECT_ROOT_DIR "/home/ema/checkout/serenity")

set(SOURCES
    "/home/ema/checkout/serenity/Shell/Parser.cpp"
    "/home/ema/checkout/serenity/Shell/LineEditor.cpp"
    "/home/ema/checkout/serenity/Shell/main.cpp"
)
set(INCLUDE_DIRS
    "${PROJECT_ROOT_DIR}/Shell"
)
set(STATIC_LINK_LIBRARIES
    "core"
)
add_executable(Shell STATIC ${SOURCES})
target_include_directories(Shell PUBLIC ${INCLUDE_DIRS})
target_link_libraries(Shell PUBLIC ${STATIC_LINK_LIBRARIES})

