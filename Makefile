USE_HOST_CXX = 1

PROGRAM = meta

DEFINES = -g

OBJS = \
    main.o \
    Settings.o \
    FileProvider.o \
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
