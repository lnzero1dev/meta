# - THIS FILE HAS BEEN GENERATED - DO NOT EDIT
# - To regenerate, please use the meta tool.

cmake_minimum_required(VERSION 3.10)

set(PROJECT_ROOT_DIR "/home/ema/checkout/serenity")

set(SOURCES
    "/home/ema/checkout/serenity/Libraries/LibELF/ELFDynamicObject.cpp"
    "/home/ema/checkout/serenity/Libraries/LibELF/ELFImage.cpp"
    "/home/ema/checkout/serenity/Libraries/LibELF/ELFLoader.cpp"
    "/home/ema/checkout/serenity/Libraries/LibELF/ELFDynamicLoader.cpp"
)
set(INCLUDE_DIRS
    "${PROJECT_ROOT_DIR}/Libraries/LibELF"
    "${PROJECT_ROOT_DIR}/Libraries"
    "${PROJECT_ROOT_DIR}"
)
set(STATIC_LINK_LIBRARIES
)
add_library(elf STATIC ${SOURCES})
target_include_directories(elf PUBLIC ${INCLUDE_DIRS})
target_link_libraries(elf PUBLIC ${STATIC_LINK_LIBRARIES})

