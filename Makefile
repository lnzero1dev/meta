USE_HOST_CXX = 1

PROGRAM = meta

DEFINES = -g

OBJS = \
    src/main.o \
    src/StringUtils.o \
    src/Settings.o \
    src/SettingsProvider.o \
    src/SettingsParameter.o \
    src/FileProvider.o \
    src/ToolchainDB.o \
    src/Toolchain.o \
    src/PackageDB.o \
    src/Package.o \
    src/ImageDB.o \
    src/Image.o \
    src/CMakeGenerator.o \
    src/DependencyResolver.o \
    ../../AK/FileSystemPath.o \
    ../../AK/String.o \
    ../../AK/StringImpl.o \
    ../../AK/StringBuilder.o \
    ../../AK/StringUtils.o \
    ../../AK/StringView.o \
    ../../AK/JsonValue.o \
    ../../AK/JsonParser.o \
    ../../AK/LogStream.o \
    ../../Libraries/LibCore/IODevice.o \
    ../../Libraries/LibCore/File.o \
    ../../Libraries/LibCore/Object.o \
    ../../Libraries/LibCore/Event.o \
    ../../Libraries/LibCore/Socket.o \
    ../../Libraries/LibCore/LocalSocket.o \
    ../../Libraries/LibCore/LocalServer.o \
    ../../Libraries/LibCore/Notifier.o \
    ../../Libraries/LibCore/DirIterator.o \
    ../../Libraries/LibCore/EventLoop.o

include ../../Makefile.common
