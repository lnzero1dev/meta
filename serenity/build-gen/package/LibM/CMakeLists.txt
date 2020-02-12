# - THIS FILE HAS BEEN GENERATED - DO NOT EDIT
# - To regenerate, please use the meta tool.

cmake_minimum_required(VERSION 3.10)

set(PROJECT_ROOT_DIR "/home/ema/checkout/serenity")

set(SOURCES
    "${PROJECT_ROOT_DIR}/Libraries/LibM/math.cpp"
)
set(INCLUDE_DIRS
    "${PROJECT_ROOT_DIR}/Libraries/LibM"
)
set(STATIC_LINK_LIBRARIES
    "c"
)
add_library(m STATIC ${SOURCES})
target_include_directories(m PUBLIC ${INCLUDE_DIRS})
target_link_libraries(m PUBLIC ${STATIC_LINK_LIBRARIES})

