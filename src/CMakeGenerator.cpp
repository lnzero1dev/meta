#include "CMakeGenerator.h"
#include "SettingsProvider.h"

CMakeGenerator::CMakeGenerator()
{
}

CMakeGenerator::~CMakeGenerator()
{
}

CMakeGenerator& CMakeGenerator::the()
{
    static CMakeGenerator* s_the;
    if (!s_the)
        s_the = &CMakeGenerator::construct().leak_ref();
    return *s_the;
}

const String CMakeGenerator::gen_header() const
{
    return "# - THIS FILE HAS BEEN GENERATED - DO NOT EDIT\n# - To regenerate, please use the meta tool.\n\n";
}

void CMakeGenerator::gen_image(const Image&) //image)
{
}

void CMakeGenerator::gen_package(const Package&) //package)
{
}

void CMakeGenerator::gen_toolchain(const Toolchain& toolchain, const Vector<Package>& native_packages)
{
    /**
     * This generates the toolchain file: serenity.cmake
     * This generates the toolchain file: CMakeLists.txt
     */

    /**
     * Example serenity.cmake:
     *
     * set(CMAKE_C_COMPILER i686-pc-serenity-gcc)
     * set(CMAKE_CXX_COMPILER i686-pc-serenity-g++)
     * set(CMAKE_CXX_FLAGS "-Os -MMD -MP -std=c++17 -Werror -Wextra -Wall -Wno-nonnull-compare -Wno-deprecated-copy -Wno-address-of-packed-member -Wundef -Wcast-qual -Wwrite-strings -Wimplicit-fallthrough -Wno-expansion-to-defined -fno-exceptions -fno-rtti -fstack-protector" CACHE STRING "" FORCE)
     * set(CMAKE_C_FLAGS "-Os -MMD -MP -Werror -Wextra -Wall -Wno-nonnull-compare -Wno-address-of-packed-member -Wundef -Wcast-qual -Wwrite-strings -Wimplicit-fallthrough -Wno-expansion-to-defined -fstack-protector" CACHE STRING "" FORCE)
     * set(CMAKE_EXE_LINKER_FLAGS "" CACHE INTERNAL "")
     * set(CMAKE_SHARED_LINKER_FLAGS "" CACHE INTERNAL "")
     * set(CMAKE_MODULE_LINKER_FLAGS "" CACHE INTERNAL "")
     * set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
     * set(CMAKE_INSTALL_PREFIX "/usr")
     * add_definitions(-DSANITIZE_PTRS)
     * add_definitions(-DDEBUG)
     */

    auto gen_path = SettingsProvider::the().get_string("gendata_directory");
    fprintf(stdout, "Gendata directory: %s\n", gen_path.value().characters());
    StringBuilder serenity_cmake;

    serenity_cmake.append(gen_header());

    serenity_cmake.append("if(LOADED)\n     return()\nendif()\nset(LOADED true)\n\n");
    serenity_cmake.append("if(NOT CMAKE_BUILD_TYPE)\n     set(CMAKE_BUILD_TYPE Debug)\nendif()\n\n");

    for (auto tool : toolchain.target_tools()) {
        if (tool.key == "cxx") {
            serenity_cmake.append("set(CMAKE_CXX_COMPILER ");
            serenity_cmake.append(tool.value.executable);
            serenity_cmake.append(")");
            serenity_cmake.append("\n");
            serenity_cmake.append("set(CMAKE_CXX_FLAGS \"");
            serenity_cmake.append(tool.value.flags);
            serenity_cmake.append("\" CACHE STRING \"\" FORCE)");
            serenity_cmake.append("\n");

        } else if (tool.key == "cc") {
            serenity_cmake.append("set(CMAKE_C_COMPILER ");
            serenity_cmake.append(tool.value.executable);
            serenity_cmake.append(")");
            serenity_cmake.append("\n");
            serenity_cmake.append("set(CMAKE_C_FLAGS \"");
            serenity_cmake.append(tool.value.flags);
            serenity_cmake.append("\" CACHE STRING \"\" FORCE)");
            serenity_cmake.append("\n");
        } else if (tool.key == "link") {
            serenity_cmake.append("set(CMAKE_EXE_LINKER_FLAGS \"");
            serenity_cmake.append(tool.value.flags);
            serenity_cmake.append("\" CACHE INTERNAL \"\" FORCE)");
            serenity_cmake.append("\n");
            serenity_cmake.append("set(CMAKE_SHARED_LINKER_FLAGS \"");
            serenity_cmake.append(tool.value.flags);
            serenity_cmake.append("\" CACHE INTERNAL \"\" FORCE)");
            serenity_cmake.append("\n");
            serenity_cmake.append("set(CMAKE_MODULE_LINKER_FLAGS \"");
            serenity_cmake.append(tool.value.flags);
            serenity_cmake.append("\" CACHE INTERNAL \"\" FORCE)");
            serenity_cmake.append("\n");
        }
        serenity_cmake.append("\n");
    }

    serenity_cmake.append("set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)");
    serenity_cmake.append("\n");
    serenity_cmake.append("set(CMAKE_INSTALL_PREFIX \"/usr\")");
    serenity_cmake.append("\n");
    serenity_cmake.append("\n");

    for (auto& configuration : toolchain.configuration()) {
        serenity_cmake.append("if(CMAKE_BUILD_TYPE STREQUAL \"");
        serenity_cmake.append(configuration.key);
        serenity_cmake.append("\")");
        serenity_cmake.append("\n");
        for (auto& tool : configuration.value) {

            if (tool.key == "cxx") {
                serenity_cmake.append("    set(CMAKE_CXX_FLAGS \"${CMAKE_CXX_FLAGS} ");
                serenity_cmake.append(tool.value.flags);
                serenity_cmake.append("\" CACHE INTERNAL \"\" FORCE)");
                serenity_cmake.append("\n");
            } else if (tool.key == "cc") {
                serenity_cmake.append("    set(CMAKE_C_FLAGS \"${CMAKE_C_FLAGS} ");
                serenity_cmake.append(tool.value.flags);
                serenity_cmake.append("\" CACHE INTERNAL \"\" FORCE)");
                serenity_cmake.append("\n");
            }
        }
        serenity_cmake.append("endif()");
        serenity_cmake.append("\n");
    }

    // write out
    StringBuilder filename;
    filename.append(gen_path.value());
    filename.append("/toolchain/serenity.cmake");
    auto filename_out = filename.build();

    fprintf(stdout, "Writing serenity.cmake: %s\n", filename_out.characters());

    FILE* fd = fopen(filename_out.characters(), "w+");

    if (!fd)
        perror("fopen");

    auto serenity_cmake_out = serenity_cmake.build();
    auto bytes = fwrite(serenity_cmake_out.characters(), 1, serenity_cmake_out.length(), fd);
    if (bytes != serenity_cmake_out.length())
        perror("fwrite");

    if (fclose(fd) < 0)
        perror("fclose");

    for (auto& package : native_packages) {
        package.name();
    }
}
