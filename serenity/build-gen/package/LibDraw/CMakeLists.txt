# - THIS FILE HAS BEEN GENERATED - DO NOT EDIT
# - To regenerate, please use the meta tool.

cmake_minimum_required(VERSION 3.10)

set(PROJECT_ROOT_DIR "/home/ema/checkout/serenity")

set(SOURCES
    "/home/ema/checkout/serenity/Libraries/LibDraw/Painter.cpp"
    "/home/ema/checkout/serenity/Libraries/LibDraw/SystemTheme.cpp"
    "/home/ema/checkout/serenity/Libraries/LibDraw/ImageDecoder.cpp"
    "/home/ema/checkout/serenity/Libraries/LibDraw/Rect.cpp"
    "/home/ema/checkout/serenity/Libraries/LibDraw/Font.cpp"
    "/home/ema/checkout/serenity/Libraries/LibDraw/Color.cpp"
    "/home/ema/checkout/serenity/Libraries/LibDraw/GIFLoader.cpp"
    "/home/ema/checkout/serenity/Libraries/LibDraw/DisjointRectSet.cpp"
    "/home/ema/checkout/serenity/Libraries/LibDraw/Emoji.cpp"
    "/home/ema/checkout/serenity/Libraries/LibDraw/GraphicsBitmap.cpp"
    "/home/ema/checkout/serenity/Libraries/LibDraw/PNGLoader.cpp"
    "/home/ema/checkout/serenity/Libraries/LibDraw/CharacterBitmap.cpp"
    "/home/ema/checkout/serenity/Libraries/LibDraw/StylePainter.cpp"
    "/home/ema/checkout/serenity/Libraries/LibDraw/Palette.cpp"
)
set(INCLUDE_DIRS
    "${PROJECT_ROOT_DIR}/Libraries/LibDraw"
)
set(STATIC_LINK_LIBRARIES
    "AK"
    "m"
)
add_library(draw STATIC ${SOURCES})
target_include_directories(draw PUBLIC ${INCLUDE_DIRS})
target_link_libraries(draw PUBLIC ${STATIC_LINK_LIBRARIES})

