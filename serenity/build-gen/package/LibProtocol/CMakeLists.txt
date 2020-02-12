# - THIS FILE HAS BEEN GENERATED - DO NOT EDIT
# - To regenerate, please use the meta tool.

cmake_minimum_required(VERSION 3.10)

set(PROJECT_ROOT_DIR "/home/ema/checkout/serenity")

set(SOURCES
    "/home/ema/checkout/serenity/Libraries/LibProtocol/Client.cpp"
    "/home/ema/checkout/serenity/Libraries/LibProtocol/Download.cpp"
)
set(INCLUDE_DIRS
    "${PROJECT_ROOT_DIR}/Libraries/LibProtocol"
    "${gendata}/LibProtocol"
)
set(STATIC_LINK_LIBRARIES
    "AK"
)
add_library(protocol STATIC ${SOURCES})
target_include_directories(protocol PUBLIC ${INCLUDE_DIRS})
target_link_libraries(protocol PUBLIC ${STATIC_LINK_LIBRARIES})

