# - THIS FILE HAS BEEN GENERATED - DO NOT EDIT
# - To regenerate, please use the meta tool.

cmake_minimum_required(VERSION 3.10)

set(PROJECT_ROOT_DIR "/home/ema/checkout/serenity")

set(SOURCES
    "/home/ema/checkout/serenity/Libraries/LibVT/TerminalWidget.cpp"
    "/home/ema/checkout/serenity/Libraries/LibVT/Terminal.cpp"
)
set(INCLUDE_DIRS
    "${PROJECT_ROOT_DIR}/Libraries/LibVT"
    "${PROJECT_ROOT_DIR}/Libraries"
)
set(STATIC_LINK_LIBRARIES
)
add_library(vt STATIC ${SOURCES})
target_include_directories(vt PUBLIC ${INCLUDE_DIRS})
target_link_libraries(vt PUBLIC ${STATIC_LINK_LIBRARIES})

