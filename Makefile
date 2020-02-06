USE_HOST_CXX = 1

PROGRAM = meta

DEFINES = -g

OBJS = \
    src/main.o \
    src/Settings.o \
    src/SettingsProvider.o \
    src/SettingsParameter.o \
    src/FileProvider.o \
    src/ToolchainDB.o \
    src/Toolchain.o \
    src/PackageDB.o \
    src/Package.o \
    ../../AK/FileSystemPath.o \
    ../../AK/String.o \
    ../../AK/StringImpl.o \
    ../../AK/StringBuilder.o \
    ../../AK/StringView.o \
    ../../AK/JsonValue.o \
    ../../AK/JsonParser.o \
    ../../AK/LogStream.o \
    ../../Libraries/LibCore/CIODevice.o \
    ../../Libraries/LibCore/CFile.o \
    ../../Libraries/LibCore/CObject.o \
    ../../Libraries/LibCore/CEvent.o \
    ../../Libraries/LibCore/CSocket.o \
    ../../Libraries/LibCore/CLocalSocket.o \
    ../../Libraries/LibCore/CLocalServer.o \
    ../../Libraries/LibCore/CNotifier.o \
    ../../Libraries/LibCore/CDirIterator.o \
    ../../Libraries/LibCore/CEventLoop.o

include ../../Makefile.common
