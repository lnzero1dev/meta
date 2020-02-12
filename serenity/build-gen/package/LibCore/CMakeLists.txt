# - THIS FILE HAS BEEN GENERATED - DO NOT EDIT
# - To regenerate, please use the meta tool.

cmake_minimum_required(VERSION 3.10)

set(PROJECT_ROOT_DIR "/home/ema/checkout/serenity")

set(SOURCES
    "/home/ema/checkout/serenity/Libraries/LibCore/CProcessStatisticsReader.cpp"
    "/home/ema/checkout/serenity/Libraries/LibCore/CEventLoop.cpp"
    "/home/ema/checkout/serenity/Libraries/LibCore/CUserInfo.cpp"
    "/home/ema/checkout/serenity/Libraries/LibCore/CArgsParser.cpp"
    "/home/ema/checkout/serenity/Libraries/LibCore/CTCPServer.cpp"
    "/home/ema/checkout/serenity/Libraries/LibCore/CSocket.cpp"
    "/home/ema/checkout/serenity/Libraries/LibCore/CObject.cpp"
    "/home/ema/checkout/serenity/Libraries/LibCore/CTCPSocket.cpp"
    "/home/ema/checkout/serenity/Libraries/LibCore/CIODevice.cpp"
    "/home/ema/checkout/serenity/Libraries/LibCore/CFile.cpp"
    "/home/ema/checkout/serenity/Libraries/LibCore/CElapsedTimer.cpp"
    "/home/ema/checkout/serenity/Libraries/LibCore/CTimer.cpp"
    "/home/ema/checkout/serenity/Libraries/LibCore/CNetworkJob.cpp"
    "/home/ema/checkout/serenity/Libraries/LibCore/CHttpRequest.cpp"
    "/home/ema/checkout/serenity/Libraries/LibCore/CNetworkResponse.cpp"
    "/home/ema/checkout/serenity/Libraries/LibCore/CHttpJob.cpp"
    "/home/ema/checkout/serenity/Libraries/LibCore/CGzip.cpp"
    "/home/ema/checkout/serenity/Libraries/LibCore/CConfigFile.cpp"
    "/home/ema/checkout/serenity/Libraries/LibCore/CLocalSocket.cpp"
    "/home/ema/checkout/serenity/Libraries/LibCore/CLocalServer.cpp"
    "/home/ema/checkout/serenity/Libraries/LibCore/CEvent.cpp"
    "/home/ema/checkout/serenity/Libraries/LibCore/CNotifier.cpp"
    "/home/ema/checkout/serenity/Libraries/LibCore/CHttpResponse.cpp"
    "/home/ema/checkout/serenity/Libraries/LibCore/CUdpServer.cpp"
    "/home/ema/checkout/serenity/Libraries/LibCore/CDirIterator.cpp"
    "/home/ema/checkout/serenity/Libraries/LibCore/CUdpSocket.cpp"
)
set(INCLUDE_DIRS
    "${PROJECT_ROOT_DIR}/Libraries/LibCore"
)
set(STATIC_LINK_LIBRARIES
)
add_library(core STATIC ${SOURCES})
target_include_directories(core PUBLIC ${INCLUDE_DIRS})
target_link_libraries(core PUBLIC ${STATIC_LINK_LIBRARIES})

