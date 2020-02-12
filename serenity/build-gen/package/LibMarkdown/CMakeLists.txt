# - THIS FILE HAS BEEN GENERATED - DO NOT EDIT
# - To regenerate, please use the meta tool.

cmake_minimum_required(VERSION 3.10)

set(PROJECT_ROOT_DIR "/home/ema/checkout/serenity")

set(SOURCES
    "/home/ema/checkout/serenity/Libraries/LibMarkdown/MDList.cpp"
    "/home/ema/checkout/serenity/Libraries/LibMarkdown/MDParagraph.cpp"
    "/home/ema/checkout/serenity/Libraries/LibMarkdown/MDDocument.cpp"
    "/home/ema/checkout/serenity/Libraries/LibMarkdown/MDHeading.cpp"
    "/home/ema/checkout/serenity/Libraries/LibMarkdown/MDText.cpp"
    "/home/ema/checkout/serenity/Libraries/LibMarkdown/MDCodeBlock.cpp"
)
set(INCLUDE_DIRS
    "${PROJECT_ROOT_DIR}/Libraries/LibMarkdown"
)
set(STATIC_LINK_LIBRARIES
)
add_library(markdown STATIC ${SOURCES})
target_include_directories(markdown PUBLIC ${INCLUDE_DIRS})
target_link_libraries(markdown PUBLIC ${STATIC_LINK_LIBRARIES})

