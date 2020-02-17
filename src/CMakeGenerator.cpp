#include "CMakeGenerator.h"
#include "DependencyResolver.h"
#include "PackageDB.h"
#include "SettingsProvider.h"
#include "StringUtils.h"
#include <string>
#include <sys/stat.h>

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

const String CMakeGenerator::cmake_minimum_version() const
{
    return "cmake_minimum_required(VERSION 3.10)\n\n";
}

const String CMakeGenerator::project_root_dir() const
{
    static String s_project_root_dir;
    if (s_project_root_dir.is_empty()) {
        StringBuilder builder;
        builder.append("set(PROJECT_ROOT_DIR \"");
        builder.append(SettingsProvider::the().get_string("root").value_or(""));
        builder.append("\")\n\n");
        s_project_root_dir = builder.build();
    }
    return s_project_root_dir;
}

bool create_dir(const String& path, const String& sub_dir = "")
{
    String path2 = path;
    if (!sub_dir.is_empty()) {
        StringBuilder builder;
        builder.append(path);
        builder.append("/");
        builder.append(sub_dir);
        path2 = builder.build();
    }

    struct stat st;

    if (stat(path2.characters(), &st) == 0) {
        if (S_ISDIR(st.st_mode)) {
            return true;
        }
    }

    int rc = mkdir(path2.characters(), 0755);
    if (rc < 0) {
        fprintf(stderr, "Could not create directory %s\n", path2.characters());
        return false;
    }
    return true;
}

String get_target_name(const String& name)
{
    String ret = name;
    if (ret.to_lowercase().contains("lib")) {
        ret = ret.to_lowercase();
        ret = replace(ret, "lib", "");
    }
    return ret;
}

void CMakeGenerator::gen_image(const Image& image, const Vector<const Package*> packages)
{
    auto gen_path = SettingsProvider::the().get_string("gendata_directory").value_or("");

    if (gen_path.is_empty()) {
        return;
    }

    if (!create_dir(gen_path, "image"))
        return;

    // write out
    StringBuilder pathBuilder;
    pathBuilder.append(gen_path);
    pathBuilder.append("/image/");
    pathBuilder.append(image.name());
    String path = pathBuilder.build();

    if (!create_dir(path))
        return;

    StringBuilder cmakelists_txt;

    for (auto& package : packages) {
        ASSERT(package);
        cmakelists_txt.append("add_subdirectory(../../package/");
        cmakelists_txt.append(package->name());
        cmakelists_txt.append(" ");
        cmakelists_txt.append(package->name());
        cmakelists_txt.append(")\n");
    }

    StringBuilder cmakelists_txt_filename;
    cmakelists_txt_filename.append(path);
    cmakelists_txt_filename.append("/CMakeLists.txt");
    FILE* fd = fopen(cmakelists_txt_filename.build().characters(), "w+");
    size_t bytes;

    if (!fd)
        perror("fopen");

    auto cmakelists_txt_out = cmakelists_txt.build();
    bytes = fwrite(cmakelists_txt_out.characters(), 1, cmakelists_txt_out.length(), fd);
    if (bytes != cmakelists_txt_out.length())
        perror("fwrite");

    if (fclose(fd) < 0)
        perror("fclose");
}

bool CMakeGenerator::gen_package(const Package& package)
{
    /**
     * This generates CMakeLists.txt for a package
     * 
     * cmake_minimum_required(VERSION 3.10)
     * set(PROJECT_ROOT_DIR "/home/ema/checkout/serenity")
     * set(SOURCES
     *    "${PROJECT_ROOT_DIR}/Libraries/LibELF/ELFDynamicLoader.cpp"
     *    "${PROJECT_ROOT_DIR}/Libraries/LibELF/ELFDynamicObject.cpp"
     *    "${PROJECT_ROOT_DIR}/Libraries/LibELF/ELFImage.cpp"
     *    "${PROJECT_ROOT_DIR}/Libraries/LibELF/ELFLoader.cpp")
     * set(INCLUDE_DIRS "${PROJECT_ROOT_DIR}/Libraries/LibELF" "${PROJECT_ROOT_DIR}/Libraries" "${PROJECT_ROOT_DIR}/Libraries/LibC" "${PROJECT_ROOT_DIR}")
     * set(STATIC_LINK_LIBRARIES "")
     * 
     * add_library(elf STATIC ${SOURCES})
     * target_include_directories(elf PRIVATE ${INCLUDE_DIRS})
     * target_link_libraries(elf PRIVATE ${STATIC_LINK_LIBRARIES})
     * 
     * install(
     *    TARGETS elf
     *    RUNTIME DESTINATION bin
     *    LIBRARY DESTINATION lib
     *    ARCHIVE DESTINATION lib)
     *
     * file(GLOB_RECURSE HEADERS ${PROJECT_ROOT_DIR}/Libraries/LibELF/\*.h)
     * install(FILES ${HEADERS} DESTINATION include/LibELF)
     * 
     */

    auto gen_path = SettingsProvider::the().get_string("gendata_directory").value_or("");

    if (gen_path.is_empty()) {
        fprintf(stderr, "Empty gen path, check configuration!\n");
        return false;
    }

    if (!create_dir(gen_path, "package"))
        return false;

    if (package.type() != PackageType::Library && package.type() != PackageType::Executable) {
        fprintf(stderr, "Package %s not of type Library or Executable.\n", package.name().characters());
        return false;
    }

    if (package.machine() != "target" && package.machine() != "host") {
        fprintf(stderr, "Package %s machine is not target or host. It is %s!\n", package.name().characters(), package.machine().characters());
        return false;
    }

    StringBuilder cmakelists_txt;

    cmakelists_txt.append(gen_header());
    cmakelists_txt.append(cmake_minimum_version());
    cmakelists_txt.append(project_root_dir());

    // sources
    cmakelists_txt.append("set(SOURCES\n");
    for (auto& source : package.sources()) {
        cmakelists_txt.append("    \"");
        cmakelists_txt.append(replace_variables(source, "root", "${PROJECT_ROOT_DIR}"));
        cmakelists_txt.append("\"");
        cmakelists_txt.append("\n");
    }
    cmakelists_txt.append(")\n");

    // includes
    cmakelists_txt.append("set(INCLUDE_DIRS\n");
    for (auto& include : package.includes()) {
        cmakelists_txt.append("    \"");
        cmakelists_txt.append(replace_variables(include, "root", "${PROJECT_ROOT_DIR}"));
        cmakelists_txt.append("\"");
        cmakelists_txt.append("\n");
    }
    cmakelists_txt.append(")\n");

    // dependencies
    cmakelists_txt.append("set(STATIC_LINK_LIBRARIES\n");
    for (auto& dependency : package.dependencies()) {
        cmakelists_txt.append("    \"");
        cmakelists_txt.append(get_target_name(dependency.key));
        cmakelists_txt.append("\"");
        cmakelists_txt.append("\n");
    }
    cmakelists_txt.append(")\n");

    // target
    auto targetName = get_target_name(package.name());

    cmakelists_txt.append("add_");
    cmakelists_txt.append(package.type() == PackageType::Library ? "library(" : (package.type() == PackageType::Executable ? "executable(" : "undefined("));
    cmakelists_txt.append(targetName);
    cmakelists_txt.append(" STATIC ${SOURCES})\n");

    cmakelists_txt.append("target_include_directories(");
    cmakelists_txt.append(targetName);
    cmakelists_txt.append(" PUBLIC ${INCLUDE_DIRS})\n");

    cmakelists_txt.append("target_link_libraries(");
    cmakelists_txt.append(targetName);
    cmakelists_txt.append(" PUBLIC ${STATIC_LINK_LIBRARIES})\n");
    cmakelists_txt.append("\n");

    // write out
    StringBuilder pathBuilder;
    pathBuilder.append(gen_path);
    pathBuilder.append("/package/");
    pathBuilder.append(package.name());
    String path = pathBuilder.build();

    if (!create_dir(path))
        return false;

    StringBuilder cmakelists_txt_filename;
    cmakelists_txt_filename.append(path);
    cmakelists_txt_filename.append("/CMakeLists.txt");
    FILE* fd = fopen(cmakelists_txt_filename.build().characters(), "w+");
    size_t bytes;

    if (!fd)
        perror("fopen");

    auto cmakelists_txt_out = cmakelists_txt.build();
    bytes = fwrite(cmakelists_txt_out.characters(), 1, cmakelists_txt_out.length(), fd);
    if (bytes != cmakelists_txt_out.length())
        perror("fwrite");

    if (fclose(fd) < 0)
        perror("fclose");

    return true;
}

void CMakeGenerator::gen_toolchain(const Toolchain& toolchain, const Vector<Package>& packages_to_build)
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

    auto gen_path = SettingsProvider::the().get_string("gendata_directory").value_or("");
    if (gen_path.is_empty()) {
        return;
    }

    if (!create_dir(gen_path, "toolchain"))
        return;

    //fprintf(stdout, "Gendata directory: %s\n", gen_path.value().characters());
    StringBuilder serenity_cmake;

    serenity_cmake.append(gen_header());

    serenity_cmake.append("if(LOADED)\n     return()\nendif()\nset(LOADED true)\n\n");
    serenity_cmake.append("if(NOT CMAKE_BUILD_TYPE)\n     set(CMAKE_BUILD_TYPE Debug)\nendif()\n\n");

    for (auto tool : toolchain.host_tools()) {
        if (tool.key == "cxx") {
            serenity_cmake.append("set(CMAKE_CXX_COMPILER ");
            serenity_cmake.append(tool.value.executable);
            serenity_cmake.append(")");
            serenity_cmake.append("\n");
            serenity_cmake.append("set(CMAKE_CXX_FLAGS \"");
            serenity_cmake.append(tool.value.flags);
            serenity_cmake.append("\" CACHE STRING \"\" FORCE)");
            serenity_cmake.append("\n\n");

        } else if (tool.key == "cc") {
            serenity_cmake.append("set(CMAKE_C_COMPILER ");
            serenity_cmake.append(tool.value.executable);
            serenity_cmake.append(")");
            serenity_cmake.append("\n");
            serenity_cmake.append("set(CMAKE_C_FLAGS \"");
            serenity_cmake.append(tool.value.flags);
            serenity_cmake.append("\" CACHE STRING \"\" FORCE)");
            serenity_cmake.append("\n\n");
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
            serenity_cmake.append("\n\n");
        }
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

    FILE* fd;

    // write out
    StringBuilder serenity_cmake_filename;
    serenity_cmake_filename.append(gen_path);
    serenity_cmake_filename.append("/toolchain/serenity.cmake");
    fd = fopen(serenity_cmake_filename.build().characters(), "w+");

    if (!fd)
        perror("fopen");

    auto serenity_cmake_out = serenity_cmake.build();
    auto bytes = fwrite(serenity_cmake_out.characters(), 1, serenity_cmake_out.length(), fd);
    if (bytes != serenity_cmake_out.length())
        perror("fwrite");

    if (fclose(fd) < 0)
        perror("fclose");

    StringBuilder cmakelists_txt;

    cmakelists_txt.append(gen_header());
    cmakelists_txt.append(cmake_minimum_version());
    cmakelists_txt.append(project_root_dir());

    serenity_cmake.append("string(ASCII 27 ESCAPE_CHAR)\n");
    serenity_cmake.append("macro(warning_message msg)\n");
    serenity_cmake.append("    message(STATUS \"${ESCAPE_CHAR}[1;${92}m${msg}${ESCAPE_CHAR}[0m\")\n");
    serenity_cmake.append("endmacro()\n\n");

    for (auto tool : toolchain.build_tools()) {
        if (!PackageDB::the().find_package_that_provides(tool.key)) {
            cmakelists_txt.append("find_program(");
            cmakelists_txt.append(tool.key.to_uppercase());
            cmakelists_txt.append("_EXE ");
            cmakelists_txt.append(tool.value.executable);
            cmakelists_txt.append(")");
            cmakelists_txt.append("\n");

            cmakelists_txt.append("if(");
            cmakelists_txt.append(tool.key.to_uppercase());
            cmakelists_txt.append("_EXE-NOTFOUND)\n");
            cmakelists_txt.append("    warning_message(\"Did not find program ");
            cmakelists_txt.append(tool.value.executable);
            cmakelists_txt.append("\")");
            cmakelists_txt.append("\n");
            cmakelists_txt.append("endif()");
            cmakelists_txt.append("\n\n");
        }
    }
    cmakelists_txt.append("\n");

    cmakelists_txt.append("include(ExternalProject)");
    cmakelists_txt.append("\n");
    cmakelists_txt.append("set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES sysroot)");
    cmakelists_txt.append("\n");

    auto root = SettingsProvider::the().get_string("root").value_or("");

    auto process_package = [&](auto& package) {
        fprintf(stdout, "Package to build: %s\n", package.name().characters());

        if (package.machine() == "host") {
            cmakelists_txt.append("# Host package: ");
            cmakelists_txt.append(package.name());
            cmakelists_txt.append("\n");

        } else if (package.machine() == "target") {
            cmakelists_txt.append("# Target package: ");
            cmakelists_txt.append(package.name());
            cmakelists_txt.append("\n");
        }

        cmakelists_txt.append("ExternalProject_Add(");
        cmakelists_txt.append(package.name());
        cmakelists_txt.append("\n");

        if (package.dependencies().size()) {
            cmakelists_txt.append("    DEPENDS ");
            for (auto& dependency : package.dependencies()) {
                cmakelists_txt.append(dependency.key);
                cmakelists_txt.append(" ");
            }
            cmakelists_txt.append("\n");
        }

        cmakelists_txt.append("    PREFIX ${CMAKE_BINARY_DIR}/");
        cmakelists_txt.append(package.name());
        cmakelists_txt.append("\n");

            // get package toolchain!
        auto& pkg_toolchain_steps = package.toolchain_steps();
        auto& pkg_toolchain_options = package.toolchain_options();

        if (pkg_toolchain_steps.size()) {
            /** 
             * Example ExternalProject for build machine package
             * 
             * ExternalProject_Add( binutils
             *     DEPENDS gcc
             *     PREFIX ${CMAKE_BINARY_DIR}/binutils
             *     URL http://ftp.gnu.org/gnu/binutils/binutils-2.33.1.tar.xz
             *     URL_HASH MD5=9406231b7d9dd93731c2d06cefe8aaf1
             *     DOWNLOAD_DIR ${CMAKE_BINARY_DIR}/download
             *     PATCH_COMMAND ${PATCH_EXE} -p1 --forward < ${PROJECT_ROOT_DIR}/Toolchain/Patches/binutils.patch || true
             *     CONFIGURE_COMMAND ${CMAKE_BINARY_DIR}/binutils/src/binutils/configure
             *            --prefix=${CMAKE_BINARY_DIR}/sysroot
             *            --target=i686-pc-serenity
             *            --with-sysroot=${CMAKE_BINARY_DIR}/sysroot
             *            --enable-shared
             *            --disable-nls
             *     BUILD_COMMAND $(MAKE) ${PARALLEL_BUILD}
             *     INSTALL_COMMAND $(MAKE) install)
             * 
             * set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES binutils)
             */

            for (auto& step : pkg_toolchain_steps) {
                if (pkg_toolchain_options.contains(step)) {
                    // found options
                    auto options = pkg_toolchain_options.get(step).value();
                    if (step == "download") {
                        if (options.has("url")) {
                            auto url = options.get("url").as_string();
                            url = replace_variables(url, "version", package.version());

                            cmakelists_txt.append("    URL ");
                            cmakelists_txt.append(url);
                            cmakelists_txt.append("\n");
                            if (options.has("url_hash_type") && options.has("url_hash")) {
                                cmakelists_txt.append("    URL_HASH ");
                                cmakelists_txt.append(options.get("url_hash_type").as_string().to_uppercase());
                                cmakelists_txt.append("=");
                                cmakelists_txt.append(options.get("url_hash").as_string());
                                cmakelists_txt.append("\n");
                            }
                            cmakelists_txt.append("    DOWNLOAD_DIR ${CMAKE_BINARY_DIR}/download");
                            cmakelists_txt.append("\n");
                        }
                    } else if (step == "patch") {
                        if (options.has("file")) {
                            Vector<String> patch_filenames;
                            if (options.get("file").is_string()) {
                                patch_filenames.append(options.get("file").as_string());
                            } else if (options.get("file").is_array()) {
                                auto values = options.get("file").as_array().values();
                                for (auto& value : values) {
                                    patch_filenames.append(value.as_string());
                                }
                            }
                            if (patch_filenames.size()) {
                                cmakelists_txt.append("    PATCH_COMMAND ${PATCH_EXE} -p1 --forward < ");
                            }
                            for (auto& patch_filename : patch_filenames) {
                                patch_filename = replace_variables(patch_filename, "root", "${PROJECT_ROOT_DIR}");
                                cmakelists_txt.append(patch_filename);
                                cmakelists_txt.append(" ");
                            }
                            if (patch_filenames.size()) {
                                cmakelists_txt.append("|| true");
                            }
                            cmakelists_txt.append("\n");
                        }
                    } else if (step == "configure") {
                        cmakelists_txt.append("    CONFIGURE_COMMAND ${CMAKE_BINARY_DIR}/");
                        cmakelists_txt.append(package.name());
                        cmakelists_txt.append("/src/");
                        cmakelists_txt.append(package.name());
                        cmakelists_txt.append("/configure");
                        cmakelists_txt.append("\n");

                        if (options.has("flags")) {
                            auto flags_object = options.get("flags");
                            Vector<String> flags;
                            if (flags_object.is_string()) {
                                fprintf(stderr, "Flags: %s\n", options.get("flags").as_string().characters());
                                //flags.append(options.get("flags").as_string());
                            } else if (flags_object.is_array()) {
                                auto values = options.get("flags").as_array().values();
                                for (auto value : values) {
                                    flags.append(value.as_string());
                                }
                            }
                            for (auto& flag : flags) {
                                flag = replace_variables(flag, "sysroot", "${CMAKE_BINARY_DIR}/sysroot");
                                cmakelists_txt.append("      ");
                                cmakelists_txt.append(flag);
                                cmakelists_txt.append("\n");
                            }
                        }
                    } else if (step == "build") {
                        cmakelists_txt.append("    BUILD_COMMAND $(MAKE) ");
                        if (options.has("targets"))
                            cmakelists_txt.append(options.get("targets").as_string());
                        cmakelists_txt.append("\n");
                    } else if (step == "install") {
                        cmakelists_txt.append("    INSTALL_COMMAND $(MAKE) ");
                        if (options.has("targets"))
                            cmakelists_txt.append(options.get("targets").as_string());
                        cmakelists_txt.append("\n");
                    }
                } else {
                    if (step == "build") {
                        // build step without options.... ok
                        cmakelists_txt.append("    BUILD_COMMAND $(MAKE)");
                        cmakelists_txt.append("\n");
                    } else if (step == "install") {
                        cmakelists_txt.append("    INSTALL_COMMAND $(MAKE) install");
                        cmakelists_txt.append("\n");
                    } else
                        fprintf(stderr, "[%s] No options for step: %s\n", package.name().characters(), step.characters());
                }
            }
            cmakelists_txt.append(")\n\n");
        } else {

            /** 
             * Example ExternalProject for target package
             * 
             * ExternalProject_Add(LibC
             *     DEPENDS gcc
             *     PREFIX ${CMAKE_BINARY_DIR}/libc
             *     SOURCE_DIR ${CMAKE_SOURCE_DIR}/../package/libc
             *     CMAKE_ARGS
             *       -DCMAKE_SYSROOT=${CMAKE_BINARY_DIR}/sysroot
             *       -DCMAKE_TOOLCHAIN_FILE=${CMAKE_CURRENT_LIST_DIR}/serenity.cmake
             *     BINARY_DIR ${CMAKE_BINARY_DIR}/libc
             *     INSTALL_COMMAND DESTDIR=${CMAKE_BINARY_DIR}/sysroot $(MAKE) install)
             * 
             *     set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "libc")
             */
            cmakelists_txt.append("    SOURCE_DIR ${CMAKE_SOURCE_DIR}/../package/");
            cmakelists_txt.append(package.name());
            cmakelists_txt.append("\n");
            cmakelists_txt.append("    CMAKE_ARGS");
            cmakelists_txt.append("\n");
            cmakelists_txt.append("      -DCMAKE_SYSROOT=${CMAKE_BINARY_DIR}/sysroot");
            cmakelists_txt.append("\n");
            cmakelists_txt.append("      -DCMAKE_TOOLCHAIN_FILE=${CMAKE_CURRENT_LIST_DIR}/serenity.cmake");
            cmakelists_txt.append("\n");
            cmakelists_txt.append("    BINARY_DIR ${CMAKE_BINARY_DIR}/");
            cmakelists_txt.append(package.name());
            cmakelists_txt.append("\n");
            cmakelists_txt.append("    INSTALL_COMMAND DESTDIR=${CMAKE_BINARY_DIR}/sysroot $(MAKE) install");
            cmakelists_txt.append(")\n");

            // ensure that the package CMakeLists.txt file is also generated
            if (!gen_package(package))
                fprintf(stderr, "Could not generate package: %s\n", package.name().characters());
        }

        cmakelists_txt.append("set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ");
        cmakelists_txt.append(package.name());
        cmakelists_txt.append(")\n\n");
    };

    Vector<String> processed_packages;
    for (auto& package : packages_to_build) {
        auto node = DependencyResolver::the().get_dependency_tree(package);
        if (node) {
            // got to leaves
            DependencyNode::start_by_leave(node, [&](auto& package) {
                if (!processed_packages.contains_slow(package.name())) {
                    process_package(package);
                    processed_packages.append(package.name());
                }
            });
        }
    }

    // write out
    StringBuilder cmakelists_txt_filename;
    cmakelists_txt_filename.append(gen_path);
    cmakelists_txt_filename.append("/toolchain/CMakeLists.txt");
    fd = fopen(cmakelists_txt_filename.build().characters(), "w+");

    if (!fd)
        perror("fopen");

    auto cmakelists_txt_out = cmakelists_txt.build();
    bytes = fwrite(cmakelists_txt_out.characters(), 1, cmakelists_txt_out.length(), fd);
    if (bytes != cmakelists_txt_out.length())
        perror("fwrite");

    if (fclose(fd) < 0)
        perror("fclose");
}
