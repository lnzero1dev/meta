cmake_minimum_required(VERSION 3.10)

set(PROJECT_ROOT_DIR "/home/ema/checkout/serenity")

set(SOURCES
    "${PROJECT_ROOT_DIR}/Libraries/LibC/assert.cpp"
    "${PROJECT_ROOT_DIR}/Libraries/LibC/crt0.cpp"
    "${PROJECT_ROOT_DIR}/Libraries/LibC/ctype.cpp"
    "${PROJECT_ROOT_DIR}/Libraries/LibC/cxxabi.cpp"
    "${PROJECT_ROOT_DIR}/Libraries/LibC/dirent.cpp"
    "${PROJECT_ROOT_DIR}/Libraries/LibC/dlfcn.cpp"
    "${PROJECT_ROOT_DIR}/Libraries/LibC/fcntl.cpp"
    "${PROJECT_ROOT_DIR}/Libraries/LibC/getopt.cpp"
    "${PROJECT_ROOT_DIR}/Libraries/LibC/grp.cpp"
    "${PROJECT_ROOT_DIR}/Libraries/LibC/ioctl.cpp"
    "${PROJECT_ROOT_DIR}/Libraries/LibC/libcinit.cpp"
    "${PROJECT_ROOT_DIR}/Libraries/LibC/libgen.cpp"
    "${PROJECT_ROOT_DIR}/Libraries/LibC/locale.cpp"
    "${PROJECT_ROOT_DIR}/Libraries/LibC/malloc.cpp"
    "${PROJECT_ROOT_DIR}/Libraries/LibC/mman.cpp"
    "${PROJECT_ROOT_DIR}/Libraries/LibC/mntent.cpp"
    "${PROJECT_ROOT_DIR}/Libraries/LibC/netdb.cpp"
    "${PROJECT_ROOT_DIR}/Libraries/LibC/poll.cpp"
    "${PROJECT_ROOT_DIR}/Libraries/LibC/pwd.cpp"
    "${PROJECT_ROOT_DIR}/Libraries/LibC/qsort.cpp"
    "${PROJECT_ROOT_DIR}/Libraries/LibC/scanf.cpp"
    "${PROJECT_ROOT_DIR}/Libraries/LibC/sched.cpp"
    "${PROJECT_ROOT_DIR}/Libraries/LibC/serenity.cpp"
    "${PROJECT_ROOT_DIR}/Libraries/LibC/signal.cpp"
    "${PROJECT_ROOT_DIR}/Libraries/LibC/stat.cpp"
    "${PROJECT_ROOT_DIR}/Libraries/LibC/stdio.cpp"
    "${PROJECT_ROOT_DIR}/Libraries/LibC/stdlib.cpp"
    "${PROJECT_ROOT_DIR}/Libraries/LibC/string.cpp"
    "${PROJECT_ROOT_DIR}/Libraries/LibC/strings.cpp"
    "${PROJECT_ROOT_DIR}/Libraries/LibC/syslog.cpp"
    "${PROJECT_ROOT_DIR}/Libraries/LibC/termcap.cpp"
    "${PROJECT_ROOT_DIR}/Libraries/LibC/termios.cpp"
    "${PROJECT_ROOT_DIR}/Libraries/LibC/time.cpp"
    "${PROJECT_ROOT_DIR}/Libraries/LibC/times.cpp"
    "${PROJECT_ROOT_DIR}/Libraries/LibC/ulimit.cpp"
    "${PROJECT_ROOT_DIR}/Libraries/LibC/unistd.cpp"
    "${PROJECT_ROOT_DIR}/Libraries/LibC/utime.cpp"
    "${PROJECT_ROOT_DIR}/Libraries/LibC/utsname.cpp"
    "${PROJECT_ROOT_DIR}/Libraries/LibC/wchar.cpp"
    "${PROJECT_ROOT_DIR}/Libraries/LibELF/ELFDynamicObject.cpp"
    "${PROJECT_ROOT_DIR}/Libraries/LibELF/ELFDynamicLoader.cpp"
    "${PROJECT_ROOT_DIR}/Libraries/LibELF/ELFLoader.cpp"
    "${PROJECT_ROOT_DIR}/Libraries/LibELF/ELFImage.cpp"
    "${PROJECT_ROOT_DIR}/AK/StringImpl.cpp"
    "${PROJECT_ROOT_DIR}/AK/String.cpp"
    "${PROJECT_ROOT_DIR}/AK/StringView.cpp"
    "${PROJECT_ROOT_DIR}/AK/StringBuilder.cpp"
    "${PROJECT_ROOT_DIR}/AK/FileSystemPath.cpp"
    "${PROJECT_ROOT_DIR}/AK/URL.cpp"
    "${PROJECT_ROOT_DIR}/AK/JsonValue.cpp"
    "${PROJECT_ROOT_DIR}/AK/JsonParser.cpp"
    "${PROJECT_ROOT_DIR}/AK/LogStream.cpp"
    "${PROJECT_ROOT_DIR}/AK/MappedFile.cpp"
    "${PROJECT_ROOT_DIR}/AK/SharedBuffer.cpp"
    "${PROJECT_ROOT_DIR}/AK/Utf8View.cpp"
    "${PROJECT_ROOT_DIR}/Libraries/LibC/crti.S"
    "${PROJECT_ROOT_DIR}/Libraries/LibC/crtn.S"
    "${PROJECT_ROOT_DIR}/Libraries/LibC/setjmp.S"
    "${PROJECT_ROOT_DIR}/Libraries/LibELF/Arch/i386/plt_trampoline.S"
)

set(INCLUDE_DIRS "${PROJECT_ROOT_DIR}" "${PROJECT_ROOT_DIR}/Libraries" "${PROJECT_ROOT_DIR}/Libraries/LibC")

set(STATIC_LINK_LIBRARIES "")

add_library(c STATIC ${SOURCES})
target_include_directories(c PRIVATE ${INCLUDE_DIRS})
target_link_libraries(c PRIVATE ${STATIC_LINK_LIBRARIES})
target_compile_definitions(c PUBLIC SERENITY_LIBC_BUILD)

install(
  TARGETS c
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)

#file(GLOB_RECURSE HEADERS ${PROJECT_ROOT_DIR}/Libraries/LibC/*.h)
#install(FILES ${HEADERS} DESTINATION include)

install(
  DIRECTORY ${PROJECT_ROOT_DIR}/Libraries/LibC/
  DESTINATION include
  FILES_MATCHING
  PATTERN "*.h"
)
