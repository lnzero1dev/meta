# - THIS FILE HAS BEEN GENERATED - DO NOT EDIT
# - To regenerate, please use the meta tool.

cmake_minimum_required(VERSION 3.10)

set(PROJECT_ROOT_DIR "/home/ema/checkout/serenity")

set(SOURCES
    "/home/ema/checkout/serenity/Libraries/LibAudio/AWavLoader.cpp"
    "/home/ema/checkout/serenity/Libraries/LibAudio/AClientConnection.cpp"
)
set(INCLUDE_DIRS
    "${PROJECT_ROOT_DIR}/Libraries/LibAudio"
    "${gendata}/LibAudio"
)
set(STATIC_LINK_LIBRARIES
    "draw"
    "protocol"
    "core"
    "gui"
    "ipc"
    "html"
)
add_library(audio STATIC ${SOURCES})
target_include_directories(audio PUBLIC ${INCLUDE_DIRS})
target_link_libraries(audio PUBLIC ${STATIC_LINK_LIBRARIES})

