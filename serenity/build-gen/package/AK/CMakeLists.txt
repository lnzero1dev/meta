# - THIS FILE HAS BEEN GENERATED - DO NOT EDIT
# - To regenerate, please use the meta tool.

cmake_minimum_required(VERSION 3.10)

set(PROJECT_ROOT_DIR "/home/ema/checkout/serenity")

set(SOURCES
    "/home/ema/checkout/serenity/AK/URL.cpp"
    "/home/ema/checkout/serenity/AK/SharedBuffer.cpp"
    "/home/ema/checkout/serenity/AK/FileSystemPath.cpp"
    "/home/ema/checkout/serenity/AK/LogStream.cpp"
    "/home/ema/checkout/serenity/AK/JsonValue.cpp"
    "/home/ema/checkout/serenity/AK/StringView.cpp"
    "/home/ema/checkout/serenity/AK/MappedFile.cpp"
    "/home/ema/checkout/serenity/AK/Utf8View.cpp"
    "/home/ema/checkout/serenity/AK/StringImpl.cpp"
    "/home/ema/checkout/serenity/AK/JsonParser.cpp"
    "/home/ema/checkout/serenity/AK/String.cpp"
    "/home/ema/checkout/serenity/AK/StringBuilder.cpp"
)
set(INCLUDE_DIRS
    "${PROJECT_ROOT_DIR}/AK"
    "${PROJECT_ROOT_DIR}"
)
set(STATIC_LINK_LIBRARIES
    "c"
)
add_library(AK STATIC ${SOURCES})
target_include_directories(AK PUBLIC ${INCLUDE_DIRS})
target_link_libraries(AK PUBLIC ${STATIC_LINK_LIBRARIES})

