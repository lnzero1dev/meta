# - THIS FILE HAS BEEN GENERATED - DO NOT EDIT
# - To regenerate, please use the meta tool.

cmake_minimum_required(VERSION 3.10)

set(PROJECT_ROOT_DIR "/home/ema/checkout/serenity")

set(SOURCES
    "/home/ema/checkout/serenity/Libraries/LibC/mntent.cpp"
    "/home/ema/checkout/serenity/Libraries/LibC/dlfcn.cpp"
    "/home/ema/checkout/serenity/Libraries/LibC/pwd.cpp"
    "/home/ema/checkout/serenity/Libraries/LibC/grp.cpp"
    "/home/ema/checkout/serenity/Libraries/LibC/poll.cpp"
    "/home/ema/checkout/serenity/Libraries/LibC/sched.cpp"
    "/home/ema/checkout/serenity/Libraries/LibC/termcap.cpp"
    "/home/ema/checkout/serenity/Libraries/LibC/malloc.cpp"
    "/home/ema/checkout/serenity/Libraries/LibC/locale.cpp"
    "/home/ema/checkout/serenity/Libraries/LibC/dirent.cpp"
    "/home/ema/checkout/serenity/Libraries/LibC/ioctl.cpp"
    "/home/ema/checkout/serenity/Libraries/LibC/strings.cpp"
    "/home/ema/checkout/serenity/Libraries/LibC/unistd.cpp"
    "/home/ema/checkout/serenity/Libraries/LibC/crt0.cpp"
    "/home/ema/checkout/serenity/Libraries/LibC/serenity.cpp"
    "/home/ema/checkout/serenity/Libraries/LibC/time.cpp"
    "/home/ema/checkout/serenity/Libraries/LibC/netdb.cpp"
    "/home/ema/checkout/serenity/Libraries/LibC/ctype.cpp"
    "/home/ema/checkout/serenity/Libraries/LibC/wchar.cpp"
    "/home/ema/checkout/serenity/Libraries/LibC/sys/socket.cpp"
    "/home/ema/checkout/serenity/Libraries/LibC/sys/wait.cpp"
    "/home/ema/checkout/serenity/Libraries/LibC/sys/select.cpp"
    "/home/ema/checkout/serenity/Libraries/LibC/sys/uio.cpp"
    "/home/ema/checkout/serenity/Libraries/LibC/signal.cpp"
    "/home/ema/checkout/serenity/Libraries/LibC/libgen.cpp"
    "/home/ema/checkout/serenity/Libraries/LibC/utsname.cpp"
    "/home/ema/checkout/serenity/Libraries/LibC/fcntl.cpp"
    "/home/ema/checkout/serenity/Libraries/LibC/stat.cpp"
    "/home/ema/checkout/serenity/Libraries/LibC/arpa/inet.cpp"
    "/home/ema/checkout/serenity/Libraries/LibC/mman.cpp"
    "/home/ema/checkout/serenity/Libraries/LibC/stdlib.cpp"
    "/home/ema/checkout/serenity/Libraries/LibC/utime.cpp"
    "/home/ema/checkout/serenity/Libraries/LibC/times.cpp"
    "/home/ema/checkout/serenity/Libraries/LibC/termios.cpp"
    "/home/ema/checkout/serenity/Libraries/LibC/assert.cpp"
    "/home/ema/checkout/serenity/Libraries/LibC/getopt.cpp"
    "/home/ema/checkout/serenity/Libraries/LibC/scanf.cpp"
    "/home/ema/checkout/serenity/Libraries/LibC/libcinit.cpp"
    "/home/ema/checkout/serenity/Libraries/LibC/stdio.cpp"
    "/home/ema/checkout/serenity/Libraries/LibC/ulimit.cpp"
    "/home/ema/checkout/serenity/Libraries/LibC/cxxabi.cpp"
    "/home/ema/checkout/serenity/Libraries/LibC/string.cpp"
    "/home/ema/checkout/serenity/Libraries/LibC/qsort.cpp"
    "/home/ema/checkout/serenity/Libraries/LibC/syslog.cpp"
    "/home/ema/checkout/serenity/Libraries/LibC/crtn.S"
    "/home/ema/checkout/serenity/Libraries/LibC/crti.S"
    "/home/ema/checkout/serenity/Libraries/LibC/setjmp.S"
    "/home/ema/checkout/serenity/Libraries/LibELF/ELFDynamicObject.cpp"
    "/home/ema/checkout/serenity/Libraries/LibELF/ELFImage.cpp"
    "/home/ema/checkout/serenity/Libraries/LibELF/ELFLoader.cpp"
    "/home/ema/checkout/serenity/Libraries/LibELF/ELFDynamicLoader.cpp"
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
    "${PROJECT_ROOT_DIR}/Libraries/LibC"
    "${PROJECT_ROOT_DIR}/Libraries"
    "${PROJECT_ROOT_DIR}"
)
set(STATIC_LINK_LIBRARIES
)
add_library(c STATIC ${SOURCES})
target_include_directories(c PUBLIC ${INCLUDE_DIRS})
target_link_libraries(c PUBLIC ${STATIC_LINK_LIBRARIES})

