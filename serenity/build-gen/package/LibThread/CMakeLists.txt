# - THIS FILE HAS BEEN GENERATED - DO NOT EDIT
# - To regenerate, please use the meta tool.

cmake_minimum_required(VERSION 3.10)

set(PROJECT_ROOT_DIR "/home/ema/checkout/serenity")

set(SOURCES
    "/home/ema/checkout/serenity/Libraries/LibThread/Thread.cpp"
    "/home/ema/checkout/serenity/Libraries/LibThread/BackgroundAction.cpp"
)
set(INCLUDE_DIRS
    "${PROJECT_ROOT_DIR}/Libraries/LibThread"
)
set(STATIC_LINK_LIBRARIES
)
add_library(thread STATIC ${SOURCES})
target_include_directories(thread PUBLIC ${INCLUDE_DIRS})
target_link_libraries(thread PUBLIC ${STATIC_LINK_LIBRARIES})

