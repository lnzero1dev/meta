# - THIS FILE HAS BEEN GENERATED - DO NOT EDIT
# - To regenerate, please use the meta tool.

cmake_minimum_required(VERSION 3.10)

set(PROJECT_ROOT_DIR "/home/ema/checkout/serenity")

set(SOURCES
    "/home/ema/checkout/serenity/Libraries/LibPCIDB/Database.cpp"
)
set(INCLUDE_DIRS
    "${PROJECT_ROOT_DIR}/Libraries/LibPCIDB"
)
set(STATIC_LINK_LIBRARIES
)
add_library(pcidb STATIC ${SOURCES})
target_include_directories(pcidb PUBLIC ${INCLUDE_DIRS})
target_link_libraries(pcidb PUBLIC ${STATIC_LINK_LIBRARIES})

