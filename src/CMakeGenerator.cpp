#include "CMakeGenerator.h"
#include "DependencyResolver.h"
#include "FileProvider.h"
#include "PackageDB.h"
#include "SettingsProvider.h"
#include "StringUtils.h"
#include <AK/FileSystemPath.h>
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
    if (!s_the) {
        s_the = &CMakeGenerator::construct().leak_ref();

        auto gen_path = SettingsProvider::the().get_string("gendata_directory").value_or("");
        if (!gen_path.is_empty())
            create_dir(gen_path);
    }
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

const String CMakeGenerator::colorful_message() const
{
    static String s_colorful_messages;
    if (s_colorful_messages.is_empty()) {
        StringBuilder builder;
        builder.append("string(ASCII 27 ESCAPE_CHAR)\n");
        builder.append("macro(warning_message msg)\n");
        builder.append("    message(STATUS \"${ESCAPE_CHAR}[1;92m${msg}${ESCAPE_CHAR}[0m\")\n");
        builder.append("endmacro()\n\n");
        builder.append("macro(info_message msg)\n");
        builder.append("    message(STATUS \"${ESCAPE_CHAR}[1;32m${msg}${ESCAPE_CHAR}[0m\")\n");
        builder.append("endmacro()\n\n");
        s_colorful_messages = builder.build();
    }
    return s_colorful_messages;
}

const String CMakeGenerator::make_command_workaround() const
{
    static String s_make_command_workaround;
    if (s_make_command_workaround.is_empty()) {
        StringBuilder builder;
        builder.append("# Workaround: Ninja doesn't support $ signs... \n");
        builder.append("string(FIND ${CMAKE_MAKE_PROGRAM} \"ninja\" NINJA_USED)\n");
        builder.append("if(NOT \"${NINJA_USED}\" STREQUAL \"-1\")\n");
        builder.append("    set(BUILD_CMD \"make\")\n");
        builder.append("else()\n");
        builder.append("    set(BUILD_CMD \"$(MAKE)\")\n");
        builder.append("endif()\n\n");
        s_make_command_workaround = builder.build();
    }
    return s_make_command_workaround;
}

const String CMakeGenerator::includes() const
{
    static String s_includes;
    if (s_includes.is_empty()) {
        StringBuilder builder;
        builder.append("include(GNUInstallDirs)\n");
        builder.append("include(ExternalProject)\n");
        builder.append("\n\n");
        s_includes = builder.build();
    }
    return s_includes;
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

bool CMakeGenerator::gen_image(const Image& image, const Vector<const Package*> packages)
{
    auto gen_path = SettingsProvider::the().get_string("gendata_directory").value_or("");

    if (gen_path.is_empty()) {
        return false;
    }

    if (!create_dir(gen_path, "Image"))
        return false;

    // write out
    StringBuilder pathBuilder;
    pathBuilder.append(gen_path);
    pathBuilder.append("/Image/");
    pathBuilder.append(image.name());
    String path = pathBuilder.build();

    if (!create_dir(path))
        return false;

    StringBuilder cmakelists_txt;

    cmakelists_txt.append(gen_header());
    cmakelists_txt.append(cmake_minimum_version());
    cmakelists_txt.append(colorful_message());

    cmakelists_txt.append("project(");
    cmakelists_txt.append(image.name());
    cmakelists_txt.append(" C CXX ASM)\n\n");
    cmakelists_txt.appendf("set(META_BUILD_IMAGE %s)\n\n", image.name().characters());
    cmakelists_txt.append("include(${CMAKE_CURRENT_LIST_DIR}/../../Toolchain/Host/tools.cmake)\n\n");

    cmakelists_txt.append(make_command_workaround());
    cmakelists_txt.append(includes());

    cmakelists_txt.append("set(DOWNLOAD_DIRECTORY ${CMAKE_BINARY_DIR}/Download CACHE INTERNAL \"\")\n");

    cmakelists_txt.append("set(CMAKE_INSTALL_PREFIX \"");
    cmakelists_txt.append(image.install_prefix());
    cmakelists_txt.append("\" CACHE INTERNAL \"\" FORCE)");
    cmakelists_txt.append("\n");

    for (auto& installDir : image.install_dirs()) {
        cmakelists_txt.append("set(CMAKE_INSTALL_");
        cmakelists_txt.append(image.install_dir_to_string(installDir.key).to_uppercase());
        cmakelists_txt.append(" ");
        cmakelists_txt.append(installDir.value);
        cmakelists_txt.append(")\n");
    }
    cmakelists_txt.append("\n");

    cmakelists_txt.append("include_directories(");
    cmakelists_txt.append(SettingsProvider::the().get_string("gendata_directory").value_or(""));
    cmakelists_txt.append("/Package/Target)\n\n");

    for (auto& package : packages) {
        ASSERT(package);
        if (!gen_package(*package))
            fprintf(stderr, "Could not generate package: %s\n", package->name().characters());

        cmakelists_txt.appendf("add_subdirectory(../../Package/%s/%s %s)\n",
            package->machine_name().characters(),
            package->name().characters(),
            package->name().characters());
    }

    StringBuilder cmakelists_txt_filename;
    cmakelists_txt_filename.append(path);
    cmakelists_txt_filename.append("/CMakeLists.txt");
    FILE* fd = fopen(cmakelists_txt_filename.build().characters(), "w+");
    size_t bytes;

    if (!fd) {
        perror("fopen");
        return false;
    }

    auto cmakelists_txt_out = cmakelists_txt.build();
    bytes = fwrite(cmakelists_txt_out.characters(), 1, cmakelists_txt_out.length(), fd);
    if (bytes != cmakelists_txt_out.length()) {
        perror("fwrite");
        return false;
    }

    if (fclose(fd) < 0) {
        perror("fclose");
        return false;
    }

    return true;
}

const String replace_dest_vars(const String& haystack)
{
    String res = haystack;
    InstallDir inst = InstallDir::Undefined;
    for (size_t i = 0; inst != InstallDir::ManDir; i++) {
        inst = static_cast<InstallDir>(i);
        auto& dirname = Image::install_dir_to_string(inst);
        StringBuilder rep;
        rep.append("${CMAKE_INSTALL_");
        rep.append(dirname.to_uppercase());
        rep.append("}");
        res = replace_variables(res, dirname, rep.build());
    }
    return res;
}

String CMakeGenerator::gen_package_collection(const Package& package)
{
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

    auto& pkg_toolchain_steps = package.toolchain_steps();
    auto& pkg_toolchain_options = package.toolchain_options();

    StringBuilder package_collection;

    package_collection.append("ExternalProject_Add(");
    package_collection.append(package.name());
    package_collection.append("\n");

    StringBuilder depends_builder;
    depends_builder.append("");

    if (package.dependencies().size()) {
        for (auto& dependency : package.dependencies()) {
            if (dependency.value == LinkageType::Direct || dependency.value == LinkageType::HeaderOnly)
                continue;
            depends_builder.append(dependency.key);
            depends_builder.append(" ");
        }
    }
    String depends_str = depends_builder.build();

    if (!depends_str.is_empty()) {
        package_collection.append("    DEPENDS ");
        package_collection.append(depends_str);
        package_collection.append("\n");
    }

    package_collection.append("    PREFIX ${CMAKE_BINARY_DIR}/");
    package_collection.append(package.name());
    package_collection.append("\n");

    for (auto& step : pkg_toolchain_steps) {
        if (pkg_toolchain_options.contains(step)) {
            // found options
            auto options = pkg_toolchain_options.get(step).value();
            if (step == "download") {
                if (options.has("url")) {
                    auto url = options.get("url").as_string();
                    url = replace_variables(url, "version", package.version());

                    package_collection.append("    URL ");
                    package_collection.append(url);
                    package_collection.append("\n");
                    if (options.has("url_hash_type") && options.has("url_hash")) {
                        package_collection.append("    URL_HASH ");
                        package_collection.append(options.get("url_hash_type").as_string().to_uppercase());
                        package_collection.append("=");
                        package_collection.append(options.get("url_hash").as_string());
                        package_collection.append("\n");
                    }

                    package_collection.append("    DOWNLOAD_DIR ${DOWNLOAD_DIRECTORY}");
                    package_collection.append("\n");
                }
            } else if (step == "patch") {
                if (options.has("file")) {
                    JsonArray patch_filenames;
                    if (options.get("file").is_string()) {
                        patch_filenames.append(options.get("file").as_string());
                    } else if (options.get("file").is_array()) {
                        patch_filenames = options.get("file").as_array();
                    }
                    if (patch_filenames.size()) {
                        package_collection.append("    PATCH_COMMAND \n      ");
                        Vector<String> values;
                        for (auto& patch_filename : patch_filenames.values()) {
                            StringBuilder b;
                            b.appendf("${PATCH_EXE} -p1 --forward < %s", make_path_with_cmake_variables(patch_filename.as_string()).characters());
                            values.append(b.build());
                        }
                        package_collection.join("      \n", values);
                        package_collection.append(" || true");
                    }
                    package_collection.append("\n");
                }
            } else if (step == "configure") {
                package_collection.append("    CONFIGURE_COMMAND\n");
                // FIXME: do not hardcode gcc, g++ name
                if (package.machine() == MachineType::Target)
                    package_collection.append("      ${CMAKE_COMMAND} -E env \"CC=i686-pc-serenity-gcc;CXX=i686-pc-serenity-g++;PATH=${CMAKE_BINARY_DIR}/../Sysroots/Host/bin:$ENV{PATH}\"\n");
                package_collection.append("        ${CMAKE_BINARY_DIR}/");
                package_collection.append(package.name());
                package_collection.append("/src/");
                package_collection.append(package.name());

                if (options.has("path") && options.get("path").is_string()) {
                    package_collection.appendf("/%s", options.get("path").as_string().characters());
                }

                package_collection.append("/configure");
                package_collection.append("\n");

                if (options.has("flags")) {
                    auto flags_object = options.get("flags");
                    Vector<String> flags;
                    if (flags_object.is_string()) {
                        flags.append(options.get("flags").as_string());
                    } else if (flags_object.is_array()) {
                        auto values = options.get("flags").as_array().values();
                        for (auto value : values) {
                            flags.append(value.as_string());
                        }
                    }
                    for (auto& flag : flags) {
                        flag = replace_variables(flag, "target_sysroot", "${CMAKE_TARGET_SYSROOT}");
                        flag = replace_variables(flag, "sysroot", "${CMAKE_SYSROOT}");
                        package_collection.append("      ");
                        package_collection.append(flag);
                        package_collection.append("\n");
                    }
                }
            } else if (step == "build") {
                package_collection.append("    BUILD_COMMAND\n");
                // FIXME: do not hardcode gcc, g++ name
                if (package.machine() == MachineType::Target)
                    package_collection.append("      ${CMAKE_COMMAND} -E env \"CC=i686-pc-serenity-gcc;CXX=i686-pc-serenity-g++;PATH=${CMAKE_BINARY_DIR}/../Sysroots/Host/bin:$ENV{PATH}\"\n");

                package_collection.append(" ${BUILD_CMD} ");
                if (options.has("targets"))
                    package_collection.append(options.get("targets").as_string());
                package_collection.append("\n");
            } else if (step == "install") {
                package_collection.append("    INSTALL_COMMAND\n");
                // FIXME: do not hardcode gcc, g++ name
                if (package.machine() == MachineType::Target)
                    package_collection.append("      ${CMAKE_COMMAND} -E env \"CC=i686-pc-serenity-gcc;CXX=i686-pc-serenity-g++;PATH=${CMAKE_BINARY_DIR}/../Sysroots/Host/bin:$ENV{PATH}\"\n");
                package_collection.append(" ${BUILD_CMD} ");
                if (options.has("targets"))
                    package_collection.append(options.get("targets").as_string());
                package_collection.append("\n");
            }
        } else {
            if (step == "build") {
                // build step without options.... ok
                package_collection.append("    BUILD_COMMAND\n");
                if (package.machine() == MachineType::Target)
                    package_collection.append("      ${CMAKE_COMMAND} -E env \"CC=i686-pc-serenity-gcc;CXX=i686-pc-serenity-g++;PATH=${CMAKE_BINARY_DIR}/../Sysroots/Host/bin:$ENV{PATH}\"\n");
                package_collection.append("        ${BUILD_CMD}\n");
            } else if (step == "install") {
                package_collection.append("    INSTALL_COMMAND\n");
                if (package.machine() == MachineType::Target)
                    package_collection.append("      ${CMAKE_COMMAND} -E env \"CC=i686-pc-serenity-gcc;CXX=i686-pc-serenity-g++;PATH=${CMAKE_BINARY_DIR}/../Sysroots/Host/bin:$ENV{PATH}\"\n");
                package_collection.append("        ${BUILD_CMD} install\n");
                package_collection.append("\n");
            } else
                fprintf(stderr, "[%s] No options for step: %s\n", package.name().characters(), step.characters());
        }
    }
    package_collection.append(")\n\n");

    return package_collection.build();
}

String CMakeGenerator::make_path_with_cmake_variables(const String& path)
{
    String path_replaced = path;

    path_replaced = replace(path_replaced, SettingsProvider::the().get_string("gendata_directory").value_or("${package_gendata}"), "${CMAKE_CURRENT_LIST_DIR}");
    path_replaced = replace(path_replaced, SettingsProvider::the().get_string("root").value_or("${root}"), "${PROJECT_ROOT_DIR}");
    path_replaced = replace_variables(path_replaced, "root", "${PROJECT_ROOT_DIR}");
    path_replaced = replace_variables(path_replaced, "package_gendata", "${CMAKE_CURRENT_LIST_DIR}/${META_BUILD_IMAGE}");
    path_replaced = replace_variables(path_replaced, "host_sysroot", "${CMAKE_SYSROOT}");
    path_replaced = replace_variables(path_replaced, "image", "${META_BUILD_IMAGE}");

    return path_replaced;
}

bool CMakeGenerator::gen_test_executable(const Package& package, const TestExecutable& test_executable)
{
    auto gen_path = SettingsProvider::the().get_string("gendata_directory").value_or("");

    if (gen_path.is_empty()) {
        fprintf(stderr, "Empty gen path, check configuration!\n");
        return false;
    }

    if (!create_dir(gen_path, "Package"))
        return false;

    StringBuilder gen_sub_path;
    gen_sub_path.appendf("Package/%s", package.machine_name().characters());
    if (!create_dir(gen_path, gen_sub_path.build()))
        return false;

    StringBuilder test_path_builder;
    test_path_builder.appendf("%s/Package/%s/%s/Tests", gen_path.characters(), package.machine_name().characters(), package.name().characters());
    auto test_path = test_path_builder.build();

    if (!create_dir(test_path))
        return false;

    if (!create_dir(test_path, test_executable.name()))
        return false;

    StringBuilder cmakelists_txt;

    cmakelists_txt.append(gen_header());
    cmakelists_txt.append(cmake_minimum_version());
    cmakelists_txt.append(project_root_dir());
    cmakelists_txt.append(includes());

    HashTable<String> directories_to_watch;

    // sources
    cmakelists_txt.append("set(SOURCES\n");
    for (auto& source : test_executable.source()) {
        cmakelists_txt.append("    \"");
        cmakelists_txt.append(make_path_with_cmake_variables(source));
        cmakelists_txt.append("\"");
        cmakelists_txt.append("\n");

        FileSystemPath path { source };
        directories_to_watch.set(make_path_with_cmake_variables(path.dirname()));
    }
    cmakelists_txt.append(")\n");

    // includes
    cmakelists_txt.append("set(INCLUDE_DIRS\n");
    for (auto& include : test_executable.include()) {
        cmakelists_txt.append("    \"");
        cmakelists_txt.append(make_path_with_cmake_variables(include));

        cmakelists_txt.append("\"");
        cmakelists_txt.append("\n");

        FileSystemPath path { include };
        directories_to_watch.set(make_path_with_cmake_variables(path.dirname()));
    }
    cmakelists_txt.append(")\n");

    cmakelists_txt.append("set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS\n    ");
    cmakelists_txt.join("\n    ", directories_to_watch);
    cmakelists_txt.append(")\n\n");

    // dependencies
    cmakelists_txt.append("set(STATIC_LINK_LIBRARIES\n");
    for (auto& dependency : test_executable.dependency()) {
        if (package.get_dependency_linkage(dependency.value) == LinkageType::Static) {
            cmakelists_txt.append("    \"");
            cmakelists_txt.append(dependency.key);
            cmakelists_txt.append("\"");
            cmakelists_txt.append("\n");
        }
    }
    cmakelists_txt.append(")\n");

    for (auto& dependency : test_executable.dependency()) {
        if (package.get_dependency_linkage(dependency.value) == LinkageType::Direct) {
            cmakelists_txt.append("include(../../../");
            cmakelists_txt.append(dependency.key);
            cmakelists_txt.append("/direct_linkage.include)\n");
        }
    }
    cmakelists_txt.append("\n");

    if (test_executable.exclude_from_package_source().size()) {
        cmakelists_txt.append("foreach(s ${SOURCES})\n");
        for (auto& exclude_file : test_executable.exclude_from_package_source()) {
            cmakelists_txt.appendf("    if(\"${s}\" STREQUAL \"%s\")\n", make_path_with_cmake_variables(exclude_file).characters());
            cmakelists_txt.append("        list(REMOVE_ITEM SOURCES \"${s}\")\n");
            cmakelists_txt.append("    endif()\n");
        }
        cmakelists_txt.append("endforeach()\n");
    }

    cmakelists_txt.appendf("set(CMAKE_CXX_FLAGS ${CMAKE_CXX_TEST_FLAGS})\n");
    cmakelists_txt.appendf("set(CMAKE_C_FLAGS ${CMAKE_C_TEST_FLAGS})\n");

    // target
    StringBuilder target_builder;
    target_builder.appendf("%s_%s", package.name().characters(), test_executable.name().characters());
    auto target = target_builder.build();
    cmakelists_txt.appendf("add_executable(%s ${SOURCES})\n", target.characters());
    cmakelists_txt.appendf("target_include_directories(%s PUBLIC ${INCLUDE_DIRS})\n", target.characters());
    cmakelists_txt.appendf("target_link_libraries(%s PUBLIC ${STATIC_LINK_LIBRARIES})\n", target.characters());
    cmakelists_txt.append("\n");

    cmakelists_txt.appendf("add_test(NAME %s COMMAND %s)\n", target.characters(), target.characters());
    cmakelists_txt.append("\n");

    // symlink res folder into build folder, if existent
    for (auto& dir : directories_to_watch) {
        cmakelists_txt.appendf("if(EXISTS %s/resource)\n", dir.characters());
        cmakelists_txt.appendf("    execute_process(COMMAND ln -sf %s/resource ${CMAKE_CURRENT_BINARY_DIR})\n", dir.characters());
        cmakelists_txt.append("endif()\n\n");
    }

    // write out
    StringBuilder pathBuilder;
    pathBuilder.appendf("%s/%s", test_path.characters(), test_executable.name().characters());
    String path = pathBuilder.build();

    StringBuilder cmakelists_txt_filename;
    cmakelists_txt_filename.append(path);
    cmakelists_txt_filename.append("/CMakeLists.txt");

    //printf("%s\n", cmakelists_txt_filename.build().characters());
    FILE* fd = fopen(cmakelists_txt_filename.build().characters(), "w+");
    size_t bytes;

    if (!fd) {
        perror("fopen");
        return false;
    }

    auto cmakelists_txt_out = cmakelists_txt.build();
    bytes = fwrite(cmakelists_txt_out.characters(), 1, cmakelists_txt_out.length(), fd);
    if (bytes != cmakelists_txt_out.length()) {
        perror("fwrite");
        return false;
    }

    if (fclose(fd) < 0) {
        perror("fclose");
        return false;
    }

    return true;
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

    if (!create_dir(gen_path, "Package"))
        return false;

    StringBuilder gen_sub_path;
    gen_sub_path.appendf("Package/%s", package.machine_name().characters());
    if (!create_dir(gen_path, gen_sub_path.build()))
        return false;

    if (package.type() == PackageType::Script || package.type() == PackageType::Undefined) {
        fprintf(stderr, "Package %s not of type Deployment, Library or Executable.\n", package.name().characters());
        return false;
    }

    StringBuilder cmakelists_txt;

    cmakelists_txt.append(gen_header());
    cmakelists_txt.append(cmake_minimum_version());
    cmakelists_txt.append(project_root_dir());
    cmakelists_txt.append(includes());

    auto targetName = package.name();

    if (package.type() == PackageType::Library || package.type() == PackageType::Executable) {

        HashTable<String> directories_to_watch;

        // sources
        cmakelists_txt.append("set(SOURCES\n");
        for (auto& source : package.sources()) {
            cmakelists_txt.append("    \"");
            cmakelists_txt.append(make_path_with_cmake_variables(source));
            cmakelists_txt.append("\"");
            cmakelists_txt.append("\n");

            FileSystemPath path { source };
            directories_to_watch.set(make_path_with_cmake_variables(path.dirname()));
        }
        cmakelists_txt.append(")\n");

        // includes
        cmakelists_txt.append("set(INCLUDE_DIRS\n");
        for (auto& include : package.includes()) {
            cmakelists_txt.append("    \"");
            cmakelists_txt.append(make_path_with_cmake_variables(include));

            cmakelists_txt.append("\"");
            cmakelists_txt.append("\n");

            FileSystemPath path { include };
            directories_to_watch.set(make_path_with_cmake_variables(path.dirname()));
        }
        cmakelists_txt.append(")\n");

        cmakelists_txt.append("set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS\n");
        cmakelists_txt.join("\n    ", directories_to_watch);
        cmakelists_txt.append(")\n\n");

        // dependencies
        cmakelists_txt.append("set(STATIC_LINK_LIBRARIES\n");
        for (auto& dependency : package.dependencies()) {
            if (package.get_dependency_linkage(dependency.value) == LinkageType::Static) {
                cmakelists_txt.append("    \"");
                cmakelists_txt.append(dependency.key);
                cmakelists_txt.append("\"");
                cmakelists_txt.append("\n");
            }
        }
        cmakelists_txt.append(")\n");

        for (auto& dependency : package.dependencies()) {
            if (package.get_dependency_linkage(dependency.value) == LinkageType::Direct) {
                cmakelists_txt.append("include(../");
                cmakelists_txt.append(dependency.key);
                cmakelists_txt.append("/direct_linkage.include)\n");
            }
        }
        cmakelists_txt.append("\n");

        //        cmakelists_txt.append("set(INTERFACE_LINK_LIBRARIES\n");
        //        for (auto& dependency : package.dependencies()) {
        //            if (package.get_dependency_linkage(dependency.value) == LinkageType::HeaderOnly) {
        //                cmakelists_txt.append("    \"");
        //                cmakelists_txt.append(get_target_name(dependency.key));
        //                cmakelists_txt.append("\"");
        //                cmakelists_txt.append("\n");
        //            }
        //        }
        //        cmakelists_txt.append(")\n");

        // target
        cmakelists_txt.append("add_");
        cmakelists_txt.append(package.type() == PackageType::Library ? "library(" : (package.type() == PackageType::Executable ? "executable(" : "undefined("));
        cmakelists_txt.append(targetName);
        // TODO: Fixme: Support other types than static libraries...
        cmakelists_txt.append(package.type() == PackageType::Library ? " STATIC " : " ");
        cmakelists_txt.append(" ${SOURCES})\n");

        cmakelists_txt.append("target_include_directories(");
        cmakelists_txt.append(targetName);
        cmakelists_txt.append(" PUBLIC ${INCLUDE_DIRS})\n");

        cmakelists_txt.append("target_link_libraries(");
        cmakelists_txt.append(targetName);
        cmakelists_txt.append(" PUBLIC ${STATIC_LINK_LIBRARIES})\n");
        cmakelists_txt.append("\n");

        //        cmakelists_txt.append("target_link_libraries(");
        //        cmakelists_txt.append(targetName);
        //        cmakelists_txt.append(" INTERFACE ${INTERFACE_LINK_LIBRARIES})\n");
        //        cmakelists_txt.append("\n");

        // Add tool options....
        // cxx: target_compile_options(<target> [BEFORE] <INTERFACE|PUBLIC|PRIVATE> [items1...] ... )
        // link: target_link_libraries(foo INTERFACE "-Wl,--allow-multiple-definition")
        //       from CMake 3.13 it is possible also to use: target_link_options(<target> [BEFORE] <INTERFACE|PUBLIC|PRIVATE> [items1...] ... )

        if (package.host_tools().size()) {
            for (auto& tool : package.host_tools()) {
                if (tool.key == "cxx") {
                    if (tool.value.execution_result_definitions.size()) {
                        for (auto& definition : tool.value.execution_result_definitions) {
                            cmakelists_txt.append("execute_process(\n");
                            cmakelists_txt.append("    COMMAND ");
                            cmakelists_txt.append(definition.value);
                            cmakelists_txt.append("\n");
                            cmakelists_txt.append("    WORKING_DIRECTORY ${PROJECT_ROOT_DIR}\n");
                            cmakelists_txt.append("    RESULT_VARIABLE GIT_RESULT\n");
                            cmakelists_txt.append("    OUTPUT_VARIABLE GIT_OUTPUT\n");
                            cmakelists_txt.append("    OUTPUT_STRIP_TRAILING_WHITESPACE\n");
                            cmakelists_txt.append(")\n");
                            cmakelists_txt.append("if(NOT \"${GIT_RESULT}\")\n");
                            cmakelists_txt.appendf("    target_compile_definitions(%s PUBLIC -D", targetName.characters());
                            cmakelists_txt.append(definition.key);
                            cmakelists_txt.append("=\"${GIT_OUTPUT}\")\n");
                            cmakelists_txt.appendf("    info_message(\"Adding complex flags definition to target \\\"%s\\\": ", targetName.characters());
                            cmakelists_txt.append(definition.key);
                            cmakelists_txt.append("=${GIT_OUTPUT}\")\n");
                            cmakelists_txt.append("endif()\n");
                        }
                    }

                    if (tool.value.reset_toolchain_flags) {
                        cmakelists_txt.append("foreach(type DEBUG RELEASE MINSIZEREL RELWITHDEBINFO)\n");
                        cmakelists_txt.append("    set(CMAKE_CXX_FLAGS_${type} \"\")\n");
                        cmakelists_txt.append("endforeach()\n\n");
                    }

                    cmakelists_txt.append("set(CMAKE_ASM_SOURCE_FILE_EXTENSIONS s;S;asm)\n");
                    cmakelists_txt.append("foreach(file ${SOURCES})\n");
                    cmakelists_txt.append("    get_filename_component(extension ${file} EXT)\n");
                    cmakelists_txt.append("    string(SUBSTRING ${extension} 1 -1 extension)\n");
                    cmakelists_txt.append("    list(FIND CMAKE_ASM_SOURCE_FILE_EXTENSIONS ${extension} index)\n");
                    cmakelists_txt.append("    if(NOT \"${index}\" STREQUAL \"-1\")\n");
                    cmakelists_txt.append("        enable_language(ASM)\n");
                    cmakelists_txt.append("        set_property(SOURCE ${file} PROPERTY CMAKE_ASM_FLAGS \"-x assembler-with-cpp\")\n");
                    cmakelists_txt.append("    endif()\n");

                    if (tool.value.flags.length()) {
                        cmakelists_txt.append("    list(FIND CMAKE_CXX_SOURCE_FILE_EXTENSIONS ${extension} index)\n");
                        cmakelists_txt.append("    if(NOT \"${index}\" STREQUAL \"-1\")\n");
                        cmakelists_txt.append("        set_property(SOURCE ${file} PROPERTY COMPILE_FLAGS \"");
                        cmakelists_txt.append(tool.value.flags);
                        cmakelists_txt.append("\")\n");
                        cmakelists_txt.append("    endif()\n");
                    }

                    cmakelists_txt.append("endforeach()\n\n");
                } else if (tool.key == "link") {
                    if (tool.value.flags.length()) {
                        cmakelists_txt.append("target_link_libraries(");
                        cmakelists_txt.append(targetName);
                        cmakelists_txt.append(" PUBLIC ");
                        cmakelists_txt.append(tool.value.flags);
                        cmakelists_txt.append(")\n\n");
                    }
                }
            }
        }

        String target_dest;
        String target_symlink;
        for (auto& deployment : package.deploy()) {
            if (deployment.ptr()->type() == DeploymentType::Target) {
                target_dest = deployment.ptr()->dest();
                target_symlink = deployment.ptr()->symlink();
                break;
            }
        }

        cmakelists_txt.append("install(TARGETS ");
        cmakelists_txt.append(targetName);
        cmakelists_txt.append("\n");

        StringBuilder symlink_builder;

        if (package.type() == PackageType::Executable) {
            cmakelists_txt.append("   RUNTIME DESTINATION ");
            if (target_dest.is_empty())
                target_dest = "${BinDir}";
            cmakelists_txt.append(replace_dest_vars(target_dest));
            cmakelists_txt.append("\n");

            if (!target_symlink.is_empty()) {
                symlink_builder.appendf("add_custom_command(OUTPUT %s", target_symlink.characters());
                symlink_builder.appendf("    COMMAND sh -c \"ln -sf %s/%s ${CMAKE_CURRENT_BINARY_DIR}/%s\" VERBATIM)\n",
                    replace_dest_vars(target_dest).characters(), targetName.characters(), target_symlink.characters());
                symlink_builder.appendf("add_custom_target(%s_%s ALL DEPENDS %s)\n",
                    targetName.characters(), "target_symlink", target_symlink.characters());
                symlink_builder.appendf("install(FILES ${CMAKE_CURRENT_BINARY_DIR}/%s DESTINATION %s)\n\n",
                    target_symlink.characters(), replace_dest_vars(target_dest).characters());
            }
        }
        if (package.type() == PackageType::Library) {
            cmakelists_txt.append("   LIBRARY DESTINATION ");
            if (target_dest.is_empty())
                target_dest = "${LibDir}";
            cmakelists_txt.append(replace_dest_vars(target_dest));
            cmakelists_txt.append("\n");

            cmakelists_txt.append("   ARCHIVE DESTINATION ");
            cmakelists_txt.append(replace_dest_vars(target_dest));
            cmakelists_txt.append("\n");
        }
        cmakelists_txt.append(")\n\n");

        auto symlink = symlink_builder.build();
        if (!symlink.is_empty()) {
            cmakelists_txt.append(symlink);
        }
    }

    if (package.type() == PackageType::Collection) {
        cmakelists_txt.append(gen_package_collection(package));
    }

    // check if deployment contains "target" type that renames the output
    if (package.deploy().size()) {
        for (auto& deployment : package.deploy()) {
            if (deployment.ptr()->type() == DeploymentType::Target) {
                if (!deployment.ptr()->name().is_empty()) {
                    cmakelists_txt.append("set_target_properties(");
                    cmakelists_txt.append(targetName);
                    cmakelists_txt.append(" PROPERTIES OUTPUT_NAME ");
                    cmakelists_txt.append(deployment.ptr()->name());
                    cmakelists_txt.append(")\n");
                }
            } else if (deployment.ptr()->type() == DeploymentType::Directory) {
                cmakelists_txt.append("install(DIRECTORY ");
                cmakelists_txt.append(make_path_with_cmake_variables(deployment.ptr()->source()));
                cmakelists_txt.append(" DESTINATION ");
                cmakelists_txt.append(replace_dest_vars(deployment.ptr()->dest()));
                if (deployment.ptr()->pattern() != "") {
                    cmakelists_txt.append(" FILES_MATCHING PATTERN \"");
                    cmakelists_txt.append(deployment.ptr()->pattern());
                    cmakelists_txt.append("\"");
                }
                cmakelists_txt.append(")\n");
            } else if (deployment.ptr()->type() == DeploymentType::File || deployment.ptr()->type() == DeploymentType::Program) {
#ifdef DEBUG_META
                fprintf(stderr, "Install file: %s\n", deployment.ptr()->source().characters());
#endif
                cmakelists_txt.append("install(");
                cmakelists_txt.append(deployment.ptr()->type() == DeploymentType::File ? "FILES " : "PROGRAM ");
                cmakelists_txt.append(make_path_with_cmake_variables(deployment.ptr()->source()));
                cmakelists_txt.append(" DESTINATION ");
                cmakelists_txt.append(replace_dest_vars(deployment.ptr()->dest()));
                if (!deployment.ptr()->rename().is_empty()) {
                    cmakelists_txt.append(" RENAME ");
                    cmakelists_txt.append(deployment.ptr()->rename());
                }
                if (deployment.ptr()->permission().has_value()) {
                    cmakelists_txt.append(" PERMISSIONS ");
                    auto& permissions = deployment.ptr()->permission().value();
#ifdef DEBUG_META
                    fprintf(stderr, "User: %i, Group: %i, Other: %i\n", permissions.user, permissions.group, permissions.other);
#endif
                    if (permissions.user & RWXPermission::Read) {
                        cmakelists_txt.append("OWNER_READ ");
                    }
                    if (permissions.user & RWXPermission::Write) {
                        cmakelists_txt.append("OWNER_WRITE ");
                    }
                    if (permissions.user & RWXPermission::Execute) {
                        cmakelists_txt.append("OWNER_EXECUTE ");
                    }
                    if (permissions.group & RWXPermission::Read) {
                        cmakelists_txt.append("GROUP_READ ");
                    }
                    if (permissions.group & RWXPermission::Write) {
                        cmakelists_txt.append("GROUP_WRITE ");
                    }
                    if (permissions.group & RWXPermission::Execute) {
                        cmakelists_txt.append("GROUP_EXECUTE ");
                    }
                    if (permissions.other & RWXPermission::Read) {
                        cmakelists_txt.append("WORLD_READ ");
                    }
                    if (permissions.other & RWXPermission::Write) {
                        cmakelists_txt.append("WORLD_WRITE ");
                    }
                    if (permissions.other & RWXPermission::Execute) {
                        cmakelists_txt.append("WORLD_EXECUTE ");
                    }
                }
                cmakelists_txt.append(")\n");
            } else if (deployment.ptr()->type() == DeploymentType::Object) {
#ifdef DEBUG_META
                fprintf(stderr, "Install object: %s\n", deployment.ptr()->name().characters());
#endif

                cmakelists_txt.append("set(LIBNAME \"");
                cmakelists_txt.append(replace(deployment.ptr()->name(), "\\.", ""));
                cmakelists_txt.append("\")\n");
                cmakelists_txt.append("add_library(${LIBNAME} STATIC ");
                cmakelists_txt.append(make_path_with_cmake_variables(deployment.ptr()->source()));
                cmakelists_txt.append(")\n");
                cmakelists_txt.append("target_include_directories(${LIBNAME} PUBLIC ${INCLUDE_DIRS})\n");
                cmakelists_txt.append("target_link_libraries(${LIBNAME} PUBLIC ${STATIC_LINK_LIBRARIES})\n");
                //cmakelists_txt.append("target_link_libraries(${LIBNAME} INTERFACE ${INTERFACE_LINK_LIBRARIES})\n");
                cmakelists_txt.append("\n");

                cmakelists_txt.append("set_target_properties(${LIBNAME} PROPERTIES PREFIX \"\")\n");
                cmakelists_txt.append("set_target_properties(${LIBNAME} PROPERTIES OUTPUT_NAME \"");
                cmakelists_txt.append(deployment.ptr()->name());
                cmakelists_txt.append("\")\n");
                cmakelists_txt.append("set_target_properties(${LIBNAME} PROPERTIES SUFFIX \"\")\n");
                if (deployment.ptr()->source().contains(".S"))
                    cmakelists_txt.append("set_target_properties(${LIBNAME} PROPERTIES LINKER_LANGUAGE \"ASM\")\n");
                cmakelists_txt.append("install(TARGETS ${LIBNAME}");
                cmakelists_txt.append("\n");
                cmakelists_txt.append("   RUNTIME DESTINATION bin\n");
                cmakelists_txt.append("   LIBRARY DESTINATION lib\n");
                cmakelists_txt.append("   ARCHIVE DESTINATION lib\n");
                cmakelists_txt.append(")\n");
            }
            cmakelists_txt.append("\n");
        }
    }

    for (auto& generator : package.run_generators()) {
        cmakelists_txt.append("# Generator: ");
        cmakelists_txt.append(generator.key);
        cmakelists_txt.append("\n");

        Vector<String> output_dirs;
        cmakelists_txt.append("set(OUTPUT_FILES\n");
        for (auto& tuple : generator.value.input_output_tuples) {
            cmakelists_txt.append("    \"");
            cmakelists_txt.append(make_path_with_cmake_variables(tuple.output));
            cmakelists_txt.append("\"\n");

            AK::FileSystemPath path(tuple.output);
            output_dirs.append(make_path_with_cmake_variables(path.dirname()));
        }
        cmakelists_txt.append(")\n");

        StringBuilder dirs_str;
        dirs_str.join("\n    ", output_dirs);
        auto dirs_joined = dirs_str.build().characters();
        cmakelists_txt.appendf("set(OUTPUT_DIRS %s)\n", dirs_joined);
        cmakelists_txt.append("file(MAKE_DIRECTORY ${OUTPUT_DIRS})\n");
        cmakelists_txt.append("set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES \"${OUTPUT_DIRS}\")\n");

        cmakelists_txt.append("find_program(");
        cmakelists_txt.append(generator.key);
        cmakelists_txt.append(" ");
        cmakelists_txt.append(generator.key);
        cmakelists_txt.append(" PATHS \"");
        cmakelists_txt.append(SettingsProvider::the().get_string("build_directory").value_or(""));
        cmakelists_txt.append("/Sysroots");
        cmakelists_txt.append("\")\n");
        cmakelists_txt.append("if(NOT ");
        cmakelists_txt.append(generator.key);
        cmakelists_txt.append(")\n    message(FATAL_ERROR \"Did not find generator ");
        cmakelists_txt.append(generator.key);
        cmakelists_txt.append("\")\nendif()\n");
        cmakelists_txt.append("add_custom_command(OUTPUT ${OUTPUT_FILES}\n");
        for (auto& tuple : generator.value.input_output_tuples) {
            cmakelists_txt.append("    COMMAND ${");
            cmakelists_txt.append(generator.key);
            cmakelists_txt.append("} ");
            cmakelists_txt.append(tuple.flags);
            cmakelists_txt.append(" ");
            cmakelists_txt.append(make_path_with_cmake_variables(tuple.input));
            // Fixme: for now, we only accept generators that put the output to stdout. Has to be configurable...
            cmakelists_txt.append(" > ");
            cmakelists_txt.append(make_path_with_cmake_variables(tuple.output));
            cmakelists_txt.append("\n");
        }
        cmakelists_txt.append("    COMMENT \"Executing ");
        cmakelists_txt.append(generator.key);
        cmakelists_txt.append(" for target ");
        cmakelists_txt.append(targetName);
        cmakelists_txt.append("\"\n");
        cmakelists_txt.append("    DEPENDS ${");
        cmakelists_txt.append(generator.key);
        cmakelists_txt.append("}\n");
        cmakelists_txt.append(")\n");

        cmakelists_txt.append("add_custom_target(");
        cmakelists_txt.append(targetName);
        cmakelists_txt.append("_");
        cmakelists_txt.append(generator.key);
        cmakelists_txt.append(" DEPENDS ${OUTPUT_FILES})\n");
        cmakelists_txt.append("add_dependencies(");
        cmakelists_txt.append(targetName);
        cmakelists_txt.append(" ");
        cmakelists_txt.append(targetName);
        cmakelists_txt.append("_");
        cmakelists_txt.append(generator.key);
        cmakelists_txt.append(")\n\n\n\n");
    }
    cmakelists_txt.append("\n");

    if (!package.test().is_null()) {
        cmakelists_txt.append("if(ENABLE_TESTS)\n");
        for (auto& test_executable : package.test()->executables()) {
            gen_test_executable(package, test_executable);
            cmakelists_txt.appendf("    add_subdirectory(Tests/%s)\n", test_executable.name().characters());
        }
        cmakelists_txt.append("endif()\n");
    }
    cmakelists_txt.append("\n");

    StringBuilder direct_linkage_include;
    // sources for source file linkage
    direct_linkage_include.append("list(APPEND SOURCES\n");
    for (auto& source : package.sources()) {
        direct_linkage_include.append("    \"");
        direct_linkage_include.append(make_path_with_cmake_variables(source));
        direct_linkage_include.append("\"");
        direct_linkage_include.append("\n");
    }
    direct_linkage_include.append(")\n");

    direct_linkage_include.append("list(APPEND INCLUDE_DIRS\n");
    for (auto& include : package.includes()) {
        direct_linkage_include.append("    \"");
        direct_linkage_include.append(make_path_with_cmake_variables(include));
        direct_linkage_include.append("\"");
        direct_linkage_include.append("\n");
    }
    direct_linkage_include.append(")\n");

    //Fixme: check if it is possible to also include the direct linkage include file of the dependencies of this package into this generated file

    // includes
    //    cmakelists_txt.append("list(APPEND INCLUDE_DIRS\n");
    //    for (auto& include : package.includes()) {
    //        direct_linkage_include.append("    \"");
    //        direct_linkage_include.append(make_path_with_cmake_variables(incl)));
    //        direct_linkage_include.append("\"");
    //        direct_linkage_include.append("\n");
    //    }
    //    direct_linkage_include.append(")\n");
    direct_linkage_include.append("\n");

    // write out
    StringBuilder pathBuilder;
    pathBuilder.append(gen_path);
    pathBuilder.appendf("/Package/%s/%s", package.machine_name().characters(), package.name().characters());
    String path = pathBuilder.build();

    if (!create_dir(path))
        return false;

    StringBuilder cmakelists_txt_filename;
    cmakelists_txt_filename.append(path);
    cmakelists_txt_filename.append("/CMakeLists.txt");
    FILE* fd = fopen(cmakelists_txt_filename.build().characters(), "w+");
    size_t bytes;

    if (!fd) {
        perror("fopen");
        return false;
    }

    auto cmakelists_txt_out = cmakelists_txt.build();
    bytes = fwrite(cmakelists_txt_out.characters(), 1, cmakelists_txt_out.length(), fd);
    if (bytes != cmakelists_txt_out.length()) {
        perror("fwrite");
        return false;
    }

    if (fclose(fd) < 0) {
        perror("fclose");
        return false;
    }

    StringBuilder direct_linkage_include_filename;
    direct_linkage_include_filename.append(path);
    direct_linkage_include_filename.append("/direct_linkage.include");
    fd = fopen(direct_linkage_include_filename.build().characters(), "w+");

    if (!fd) {
        perror("fopen");
        return false;
    }

    auto sources_include_out = direct_linkage_include.build();
    bytes = fwrite(sources_include_out.characters(), 1, sources_include_out.length(), fd);
    if (bytes != sources_include_out.length()) {
        perror("fwrite");
        return false;
    }

    if (fclose(fd) < 0) {
        perror("fclose");
        return false;
    }

    return true;
}

const String CMakeGenerator::gen_cmake_toolchain_content(const HashMap<String, Tool>& tools, Optional<const HashMap<String, HashMap<String, ToolConfiguration>>> toolchain_configuration)
{
    StringBuilder target_toolchain_cmake;
    target_toolchain_cmake.append(gen_header());

    target_toolchain_cmake.append("if(LOADED)\n     return()\nendif()\nset(LOADED true)\n\n");
    target_toolchain_cmake.append("if(NOT CMAKE_BUILD_TYPE)\n     set(CMAKE_BUILD_TYPE Debug)\nendif()\n\n");

    for (auto tool : tools) {
        if (tool.key == "cxx") {
            target_toolchain_cmake.append("set(CMAKE_CXX_COMPILER ");
            target_toolchain_cmake.append(tool.value.executable);
            target_toolchain_cmake.append(")");
            target_toolchain_cmake.append("\n");
            target_toolchain_cmake.append("set(CMAKE_CXX_FLAGS \"");
            target_toolchain_cmake.append(tool.value.flags);
            target_toolchain_cmake.append("\" CACHE STRING \"\" FORCE)");
            target_toolchain_cmake.append("\n");
            target_toolchain_cmake.append("set(CMAKE_CXX_TEST_FLAGS \"");
            target_toolchain_cmake.append(tool.value.test_flags);
            target_toolchain_cmake.append("\" CACHE STRING \"\" FORCE)");
            target_toolchain_cmake.append("\n\n");

        } else if (tool.key == "cc") {
            target_toolchain_cmake.append("set(CMAKE_C_COMPILER ");
            target_toolchain_cmake.append(tool.value.executable);
            target_toolchain_cmake.append(")");
            target_toolchain_cmake.append("\n");
            target_toolchain_cmake.append("set(CMAKE_C_FLAGS \"");
            target_toolchain_cmake.append(tool.value.flags);
            target_toolchain_cmake.append("\" CACHE STRING \"\" FORCE)");
            target_toolchain_cmake.append("\n");
            target_toolchain_cmake.append("set(CMAKE_C_TEST_FLAGS \"");
            target_toolchain_cmake.append(tool.value.test_flags);
            target_toolchain_cmake.append("\" CACHE STRING \"\" FORCE)");
            target_toolchain_cmake.append("\n\n");

        } else if (tool.key == "link") {
            target_toolchain_cmake.append("set(CMAKE_EXE_LINKER_FLAGS \"");
            target_toolchain_cmake.append(tool.value.flags);
            target_toolchain_cmake.append("\" CACHE INTERNAL \"\" FORCE)");
            target_toolchain_cmake.append("\n");
            target_toolchain_cmake.append("set(CMAKE_SHARED_LINKER_FLAGS \"");
            target_toolchain_cmake.append(tool.value.flags);
            target_toolchain_cmake.append("\" CACHE INTERNAL \"\" FORCE)");
            target_toolchain_cmake.append("\n");
            target_toolchain_cmake.append("set(CMAKE_MODULE_LINKER_FLAGS \"");
            target_toolchain_cmake.append(tool.value.flags);
            target_toolchain_cmake.append("\" CACHE INTERNAL \"\" FORCE)");
            target_toolchain_cmake.append("\n\n");
        }
    }

    target_toolchain_cmake.append("set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)");
    target_toolchain_cmake.append("\n");

    if (toolchain_configuration.has_value()) {
        for (auto& configuration : toolchain_configuration.value()) {
            target_toolchain_cmake.append("if(CMAKE_BUILD_TYPE STREQUAL \"");
            target_toolchain_cmake.append(configuration.key);
            target_toolchain_cmake.append("\")");
            target_toolchain_cmake.append("\n");
            for (auto& tool : configuration.value) {

                if (tool.key == "cxx") {
                    target_toolchain_cmake.append("    set(CMAKE_CXX_FLAGS \"${CMAKE_CXX_FLAGS} ");
                    target_toolchain_cmake.append(tool.value.flags);
                    target_toolchain_cmake.append("\" CACHE INTERNAL \"\" FORCE)");
                    target_toolchain_cmake.append("\n");
                } else if (tool.key == "cc") {
                    target_toolchain_cmake.append("    set(CMAKE_C_FLAGS \"${CMAKE_C_FLAGS} ");
                    target_toolchain_cmake.append(tool.value.flags);
                    target_toolchain_cmake.append("\" CACHE INTERNAL \"\" FORCE)");
                    target_toolchain_cmake.append("\n");
                }
            }
            target_toolchain_cmake.append("endif()");
            target_toolchain_cmake.append("\n");
        }
    }

    target_toolchain_cmake.append("set(CMAKE_ASM_CREATE_STATIC_LIBRARY \"<CMAKE_AR> crT <TARGET> <LINK_FLAGS> <OBJECTS>\")\n\n");
    target_toolchain_cmake.append("set(CMAKE_ASM_FLAGS_DEBUG \"\" CACHE INTERNAL \"\" FORCE)\n");
    target_toolchain_cmake.append("set(CMAKE_ASM_FLAGS \"\" CACHE INTERNAL \"\" FORCE)\n");
    target_toolchain_cmake.append("\n");

    return target_toolchain_cmake.build();
}

String CMakeGenerator::gen_toolchain_package(const Package& package)
{
    StringBuilder toolchain_package;
#ifdef DEBUG_META
    fprintf(stderr, "Package to build: %s\n", package.name().characters());
#endif

    toolchain_package.append("# ");
    toolchain_package.append(package.machine_name());
    toolchain_package.append(" package: ");
    toolchain_package.append(package.name());
    toolchain_package.append("\n");

    // get package toolchain!
    auto& pkg_toolchain_steps = package.toolchain_steps();
    auto& pkg_toolchain_options = package.toolchain_options();

    if (pkg_toolchain_steps.size()) {
        toolchain_package.append(gen_package_collection(package));
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
        toolchain_package.append("ExternalProject_Add(");
        toolchain_package.append(package.name());
        toolchain_package.append("\n");

        StringBuilder depends_builder;
        depends_builder.append("");

        if (package.dependencies().size()) {
            for (auto& dependency : package.dependencies()) {
                if (dependency.value == LinkageType::Direct || dependency.value == LinkageType::HeaderOnly)
                    continue;
                depends_builder.append(dependency.key);
                depends_builder.append(" ");
            }
        }
        String depends_str = depends_builder.build();

        if (!depends_str.is_empty()) {
            toolchain_package.append("    DEPENDS ");
            toolchain_package.append(depends_str);
            toolchain_package.append("\n");
        }

        toolchain_package.appendf("    SOURCE_DIR ${CMAKE_SOURCE_DIR}/../../Package/%s/%s\n", package.machine_name().characters(), package.name().characters());
        toolchain_package.append("    CMAKE_ARGS\n");

        bool toolchain_override = false;
        if (package.toolchain_options().contains("build")) {
            auto options = pkg_toolchain_options.get("build").value();

            if (options.has("use_toolchain")) {
                if (options.get("use_toolchain").as_string() == "target") {
                    toolchain_package.append("      -DCMAKE_SYSROOT=${CMAKE_SYSROOT}\n");
                    toolchain_package.append("      -DCMAKE_TOOLCHAIN_FILE=${CMAKE_CURRENT_LIST_DIR}/../Target/toolchain.cmake\n");
                    toolchain_override = true;
                }
            }
        }

        if (!toolchain_override)
            toolchain_package.append("      -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}\n");

        toolchain_package.append("      -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}\n");
        toolchain_package.append("    BINARY_DIR ${CMAKE_BINARY_DIR}/");
        toolchain_package.append(package.name());
        toolchain_package.append("\n");
        toolchain_package.append("    INSTALL_COMMAND DESTDIR=${CMAKE_SYSROOT} cmake --build . --target install");
        //            toolchain_package.append("\n");
        //            toolchain_package.append("    BUILD_ALWAYS true");
        toolchain_package.append(")\n");

        // ensure that the package CMakeLists.txt file is also generated
        if (!gen_package(package))
            fprintf(stderr, "Could not generate package: %s\n", package.name().characters());
    }

    toolchain_package.append("set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${CMAKE_BINARY_DIR}/");
    toolchain_package.append(package.name());
    toolchain_package.append(")\n\n");

    return toolchain_package.build();
}

StringBuilder CMakeGenerator::gen_toolchain_cmakelists_txt()
{
    StringBuilder cmakelists_txt;

    cmakelists_txt.append(gen_header());
    cmakelists_txt.append(cmake_minimum_version());
    cmakelists_txt.append("project(toolchain)\n");
    cmakelists_txt.append(project_root_dir());
    cmakelists_txt.append(colorful_message());

    cmakelists_txt.append("set(CMAKE_INSTALL_PREFIX \"/usr\" CACHE INTERNAL \"\" FORCE)\n\n");

    cmakelists_txt.append("include(tools.cmake)\n\n");
    cmakelists_txt.append(make_command_workaround());
    cmakelists_txt.append(includes());

    return cmakelists_txt;
}

const String CMakeGenerator::find_tools_not_in_toolchain(const HashMap<String, Tool>& tools) const
{
    StringBuilder find_tools_not_in_toolchain;
    for (auto tool : tools) {
        if (!HostPackageDB::the().find_package_that_provides(tool.key)) {
            find_tools_not_in_toolchain.append("find_program(");
            find_tools_not_in_toolchain.append(tool.key.to_uppercase());
            find_tools_not_in_toolchain.append("_EXE ");
            find_tools_not_in_toolchain.append(tool.value.executable);
            find_tools_not_in_toolchain.append(")");
            find_tools_not_in_toolchain.append("\n");

            find_tools_not_in_toolchain.append("if(");
            find_tools_not_in_toolchain.append(tool.key.to_uppercase());
            find_tools_not_in_toolchain.append("_EXE-NOTFOUND)\n");
            find_tools_not_in_toolchain.append("    warning_message(\"Did not find program ");
            find_tools_not_in_toolchain.append(tool.value.executable);
            find_tools_not_in_toolchain.append("\")");
            find_tools_not_in_toolchain.append("\n");
            find_tools_not_in_toolchain.append("endif()");
            find_tools_not_in_toolchain.append("\n\n");
        }
    }
    find_tools_not_in_toolchain.append("\n");
    return find_tools_not_in_toolchain.build();
}

bool CMakeGenerator::gen_toolchain(const Toolchain& toolchain, const Vector<String>& json_input_files)
{
    /**
     * This generates the toolchain file: Build/toolchain.cmake
     * This generates the toolchain file: Host/toolchain.cmake
     * This generates the toolchain file: Target/toolchain.cmake
     * This generates the toolchain file: Build/CMakeLists.txt
     * This generates the toolchain file: Target/CMakeLists.txt
     * This generates the tool reconfiguration input file: meta_json_files.depend
     */

    /**
     * Example toolchain.cmake:
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
     * set(CMAKE_SYSROOT "Build/Toolchain/Sysroot")
     */

    auto gen_path = SettingsProvider::the().get_string("gendata_directory").value_or("");
    if (gen_path.is_empty()) {
        return false;
    }

    if (!create_dir(gen_path, "Toolchain"))
        return false;

    if (!create_dir(gen_path, "Toolchain/Target"))
        return false;

    if (!create_dir(gen_path, "Toolchain/Host"))
        return false;

    if (!create_dir(gen_path, "Toolchain/Build"))
        return false;
    //fprintf(stdout, "Gendata directory: %s\n", gen_path.value().characters());

    String target_toolchain_cmake = gen_cmake_toolchain_content(toolchain.target_tools(), toolchain.configuration());
    String build_toolchain_cmake = gen_cmake_toolchain_content(toolchain.build_tools(), {});
    String host_toolchain_cmake = gen_cmake_toolchain_content(toolchain.host_tools(), {});

    FILE* fd;

    // write out
    // target/toolchain.cmake
    StringBuilder target_toolchain_cmake_filename;
    target_toolchain_cmake_filename.append(gen_path);
    target_toolchain_cmake_filename.append("/Toolchain/Target/toolchain.cmake");
    fd = fopen(target_toolchain_cmake_filename.build().characters(), "w+");

    if (!fd) {
        perror("fopen");
        return false;
    }

    auto bytes = fwrite(target_toolchain_cmake.characters(), 1, target_toolchain_cmake.length(), fd);
    if (bytes != target_toolchain_cmake.length()) {
        perror("fwrite");
        return false;
    }

    if (fclose(fd) < 0) {
        perror("fclose");
        return false;
    }

    // host/toolchain.cmake
    StringBuilder host_toolchain_cmake_filename;
    host_toolchain_cmake_filename.append(gen_path);
    host_toolchain_cmake_filename.append("/Toolchain/Host/toolchain.cmake");
    fd = fopen(host_toolchain_cmake_filename.build().characters(), "w+");

    if (!fd) {
        perror("fopen");
        return false;
    }

    bytes = fwrite(host_toolchain_cmake.characters(), 1, host_toolchain_cmake.length(), fd);
    if (bytes != host_toolchain_cmake.length()) {
        perror("fwrite");
        return false;
    }

    if (fclose(fd) < 0) {
        perror("fclose");
        return false;
    }

    // build/toolchain.cmake
    StringBuilder build_toolchain_cmake_filename;
    build_toolchain_cmake_filename.append(gen_path);
    build_toolchain_cmake_filename.append("/Toolchain/Build/toolchain.cmake");
    fd = fopen(build_toolchain_cmake_filename.build().characters(), "w+");

    if (!fd) {
        perror("fopen");
        return false;
    }

    bytes = fwrite(build_toolchain_cmake.characters(), 1, build_toolchain_cmake.length(), fd);
    if (bytes != build_toolchain_cmake.length()) {
        perror("fwrite");
        return false;
    }

    if (fclose(fd) < 0) {
        perror("fclose");
        return false;
    }

    auto root = SettingsProvider::the().get_string("root").value_or("");

    // Build/tools.cmake
    String build_tools_cmake = find_tools_not_in_toolchain(toolchain.build_tools());

    // write out
    StringBuilder build_tools_cmake_filename;
    build_tools_cmake_filename.append(gen_path);
    build_tools_cmake_filename.append("/Toolchain/Build/tools.cmake");
    fd = fopen(build_tools_cmake_filename.build().characters(), "w+");

    if (!fd) {
        perror("fopen");
        return false;
    }

    bytes = fwrite(build_tools_cmake.characters(), 1, build_tools_cmake.length(), fd);
    if (bytes != build_tools_cmake.length()) {
        perror("fwrite");
        return false;
    }

    if (fclose(fd) < 0) {
        perror("fclose");
        return false;
    }

    // Build/CMakeLists.txt
    auto build_cmakelists_txt = gen_toolchain_cmakelists_txt();

    Vector<Package> build_packages_to_build;

    BuildPackageDB::the().for_each_entry([&](auto&, auto& package) {
        if (package.type() != PackageType::Script) {
            build_packages_to_build.append(package);
        }
        return IterationDecision::Continue;
    });

    Vector<String> build_processed_packages;
    for (auto& package : build_packages_to_build) {
        auto node = DependencyResolver::the().get_dependency_tree(package);
        if (node) {
            // go to leaves
            DependencyNode::start_by_leave(node, [&](auto& package) {
                if (!build_processed_packages.contains_slow(package.name())) {
                    build_cmakelists_txt.append(gen_toolchain_package(package));
                    build_processed_packages.append(package.name());
                }
            });
        }
    }

    // write out
    StringBuilder build_cmakelists_txt_filename;
    build_cmakelists_txt_filename.append(gen_path);
    build_cmakelists_txt_filename.append("/Toolchain/Build/CMakeLists.txt");
    fd = fopen(build_cmakelists_txt_filename.build().characters(), "w+");

    if (!fd) {
        perror("fopen");
        return false;
    }

    auto build_cmakelists_txt_out = build_cmakelists_txt.build();
    bytes = fwrite(build_cmakelists_txt_out.characters(), 1, build_cmakelists_txt_out.length(), fd);
    if (bytes != build_cmakelists_txt_out.length()) {
        perror("fwrite");
        return false;
    }

    if (fclose(fd) < 0) {
        perror("fclose");
        return false;
    }

    // Host/tools.cmake
    String host_tools_cmake = find_tools_not_in_toolchain(toolchain.host_tools());

    // write out
    StringBuilder host_tools_cmake_filename;
    host_tools_cmake_filename.append(gen_path);
    host_tools_cmake_filename.append("/Toolchain/Host/tools.cmake");
    fd = fopen(host_tools_cmake_filename.build().characters(), "w+");

    if (!fd) {
        perror("fopen");
        return false;
    }

    bytes = fwrite(host_tools_cmake.characters(), 1, host_tools_cmake.length(), fd);
    if (bytes != host_tools_cmake.length()) {
        perror("fwrite");
        return false;
    }

    if (fclose(fd) < 0) {
        perror("fclose");
        return false;
    }

    // Host/CMakeLists.txt
    auto host_cmakelists_txt = gen_toolchain_cmakelists_txt();

    //FIXME: Make it configureable, if tests should be run (maybe depend this on configruation 'when tests shall be run')
    host_cmakelists_txt.append("set(ENABLE_TESTS ON)\n\n");
    host_cmakelists_txt.append("if(ENABLE_TESTS)\n");
    host_cmakelists_txt.append("    enable_testing()\n");
    host_cmakelists_txt.append("endif()\n\n");

    Vector<Package> host_packages_to_build;

    HostPackageDB::the().for_each_entry([&](auto&, auto& package) {
        if (package.type() != PackageType::Script) {
            host_packages_to_build.append(package);
        }
        return IterationDecision::Continue;
    });

    Vector<String> host_processed_packages;
    for (auto& package : host_packages_to_build) {
        auto node = DependencyResolver::the().get_dependency_tree(package);
        if (node) {
            // go to leaves
            DependencyNode::start_by_leave(node, [&](auto& package) {
                if (!host_processed_packages.contains_slow(package.name())) {
                    if (package.machine() == MachineType::Host) {
                        //host_cmakelists_txt.append(gen_toolchain_package(package));
                        if (!gen_package(package))
                            fprintf(stderr, "Could not generate package: %s\n", package.name().characters());

                        host_cmakelists_txt.appendf("add_subdirectory(../../Package/%s/%s %s)\n",
                            package.machine_name().characters(),
                            package.name().characters(),
                            package.name().characters());

                        host_processed_packages.append(package.name());
                    }
                }
            });
        }
    }

    // write out
    StringBuilder host_cmakelists_txt_filename;
    host_cmakelists_txt_filename.append(gen_path);
    host_cmakelists_txt_filename.append("/Toolchain/Host/CMakeLists.txt");
    fd = fopen(host_cmakelists_txt_filename.build().characters(), "w+");

    if (!fd) {
        perror("fopen");
        return false;
    }

    auto host_cmakelists_txt_out = host_cmakelists_txt.build();
    bytes = fwrite(host_cmakelists_txt_out.characters(), 1, host_cmakelists_txt_out.length(), fd);
    if (bytes != host_cmakelists_txt_out.length()) {
        perror("fwrite");
        return false;
    }

    if (fclose(fd) < 0) {
        perror("fclose");
        return false;
    }

    // meta_json_files.depend
    StringBuilder meta_json_files_depends;
    for (auto& file : json_input_files) {
        meta_json_files_depends.append(file);
        meta_json_files_depends.append("\n");
    }

    // write out
    StringBuilder meta_json_files_depends_filename;
    meta_json_files_depends_filename.append(gen_path);
    meta_json_files_depends_filename.append("/Toolchain/meta_json_files.depend");
    fd = fopen(meta_json_files_depends_filename.build().characters(), "w+");

    if (!fd) {
        perror("fopen");
        return false;
    }

    auto meta_json_files_depends_out = meta_json_files_depends.build();
    bytes = fwrite(meta_json_files_depends_out.characters(), 1, meta_json_files_depends_out.length(), fd);
    if (bytes != meta_json_files_depends_out.length()) {
        perror("fwrite");
        return false;
    }

    if (fclose(fd) < 0) {
        perror("fclose");
        return false;
    }

    return true;
}

bool CMakeGenerator::gen_root(const Toolchain& toolchain, int argc, char** argv)
{

    auto gen_path = SettingsProvider::the().get_string("gendata_directory").value_or("");

    if (gen_path.is_empty()) {
        return false;
    }

    StringBuilder cmakelists_txt;

    cmakelists_txt.append(gen_header());
    cmakelists_txt.append(cmake_minimum_version());

    cmakelists_txt.append("project(root)\n");

    cmakelists_txt.append(project_root_dir());

    cmakelists_txt.append("SET(DOWNLOAD_DIRECTORY ${CMAKE_BINARY_DIR}/Download)\n");

    cmakelists_txt.append("SET(META_BINARY ");
    for (int i = 0; i < argc; ++i) {
        cmakelists_txt.append(argv[i]);
        cmakelists_txt.append(" ");
    }
    cmakelists_txt.append(")\n");

    cmakelists_txt.append("SET(WORKING_DIRECTORY ");
    cmakelists_txt.append(FileProvider::the().current_dir());
    cmakelists_txt.append(")\n");

    cmakelists_txt.append("if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/Toolchain/meta_json_files.depend)\n");
    cmakelists_txt.append("    file(STRINGS \"${CMAKE_CURRENT_LIST_DIR}/Toolchain/meta_json_files.depend\" META_JSON_FILES_DEPEND)\n");
    cmakelists_txt.append("endif()\n");
    cmakelists_txt.append("add_custom_command(\n");
    cmakelists_txt.append("    OUTPUT ${CMAKE_CURRENT_LIST_DIR}/Toolchain/meta_json_files.depend\n");
    cmakelists_txt.append("    COMMAND ${META_BINARY}\n");
    cmakelists_txt.append("    DEPENDS ${META_JSON_FILES_DEPEND}\n");
    cmakelists_txt.append("    WORKING_DIRECTORY ${WORKING_DIRECTORY}\n");
    cmakelists_txt.append("    COMMENT \"Execute META to re-generate CMake files.\"\n");
    cmakelists_txt.append(")\n");
    cmakelists_txt.append("add_custom_target(auto-meta-generation ALL DEPENDS ${CMAKE_CURRENT_LIST_DIR}/Toolchain/meta_json_files.depend)\n\n");

    cmakelists_txt.append("add_custom_target(meta-generation\n");
    cmakelists_txt.append("    COMMAND ${META_BINARY}\n");
    cmakelists_txt.append("    WORKING_DIRECTORY ${WORKING_DIRECTORY}\n");
    cmakelists_txt.append("    COMMENT \"Execute META to re-generate CMake files.\"\n");
    cmakelists_txt.append(")\n");
    cmakelists_txt.append("\n\n");

    cmakelists_txt.append(includes());
    cmakelists_txt.append("set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${CMAKE_BINARY_DIR}/Sysroots)\n\n");

    // build toolchain
    cmakelists_txt.append("ExternalProject_Add(BuildToolchain\n");
    cmakelists_txt.append("    DEPENDS auto-meta-generation\n");
    cmakelists_txt.append("    PREFIX ${CMAKE_BINARY_DIR}/BuildToolchain\n");
    cmakelists_txt.append("    SOURCE_DIR ${CMAKE_SOURCE_DIR}/Toolchain/Build\n");
    cmakelists_txt.append("    CMAKE_ARGS\n");
    cmakelists_txt.append("        -DCMAKE_TOOLCHAIN_FILE=${CMAKE_CURRENT_LIST_DIR}/Toolchain/Build/toolchain.cmake\n");
    cmakelists_txt.append("        -DCMAKE_SYSROOT=${CMAKE_BINARY_DIR}/Sysroots/Host\n");
    cmakelists_txt.append("        -DDOWNLOAD_DIRECTORY=${DOWNLOAD_DIRECTORY}\n");
    cmakelists_txt.append("    BINARY_DIR ${CMAKE_BINARY_DIR}/BuildToolchain\n");
    cmakelists_txt.append("    INSTALL_COMMAND \"\"\n");
    cmakelists_txt.append(")\n");
    cmakelists_txt.append("# To clean up toolchain on make clean, uncomment the next line\n");
    cmakelists_txt.append("#set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES BuildToolchain)\n");
    cmakelists_txt.append("\n");

    // host toolchain
    cmakelists_txt.append("ExternalProject_Add(HostToolchain\n");
    cmakelists_txt.append("    DEPENDS BuildToolchain auto-meta-generation\n");
    cmakelists_txt.append("    PREFIX ${CMAKE_BINARY_DIR}/HostToolchain\n");
    cmakelists_txt.append("    SOURCE_DIR ${CMAKE_SOURCE_DIR}/Toolchain/Host\n");
    cmakelists_txt.append("    CMAKE_ARGS\n");
    cmakelists_txt.append("        -DCMAKE_TOOLCHAIN_FILE=${CMAKE_CURRENT_LIST_DIR}/Toolchain/Host/toolchain.cmake\n");
    cmakelists_txt.append("        -DDOWNLOAD_DIRECTORY=${DOWNLOAD_DIRECTORY}\n");
    cmakelists_txt.append("    BINARY_DIR ${CMAKE_BINARY_DIR}/HostToolchain\n");
    cmakelists_txt.append("    INSTALL_COMMAND DESTDIR=${CMAKE_BINARY_DIR}/Sysroots/Host cmake --build . --target install\n");
    cmakelists_txt.append("    BUILD_ALWAYS true\n");
    cmakelists_txt.append("    TEST_COMMAND ${CMAKE_CTEST_COMMAND} --verbose\n");
    //FIXME: Make it configureable, when tests should be run
    //cmakelists_txt.append("    TEST_BEFORE_INSTALL true\n");
    cmakelists_txt.append("    TEST_EXCLUDE_FROM_MAIN true\n");
    cmakelists_txt.append(")\n");
    cmakelists_txt.append("set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${CMAKE_BINARY_DIR}/HostToolchain)\n");
    cmakelists_txt.append("ExternalProject_Add_StepTargets(HostToolchain test)\n");
    cmakelists_txt.append("add_custom_target(run-tests DEPENDS HostToolchain-test)\n");
    cmakelists_txt.append("\n");

    // target build
    cmakelists_txt.append("ExternalProject_Add(Target\n");
    cmakelists_txt.append("    DEPENDS HostToolchain auto-meta-generation\n");
    cmakelists_txt.append("    PREFIX ${CMAKE_BINARY_DIR}/Target\n");
    cmakelists_txt.append("    SOURCE_DIR ${CMAKE_SOURCE_DIR}/Image/default-image\n");
    cmakelists_txt.append("    CMAKE_ARGS\n");
    cmakelists_txt.append("        -DCMAKE_TOOLCHAIN_FILE=${CMAKE_CURRENT_LIST_DIR}/Toolchain/Target/toolchain.cmake\n");
    cmakelists_txt.append("        -DCMAKE_SYSROOT=${CMAKE_BINARY_DIR}/Sysroots/Host\n");
    cmakelists_txt.append("        -DCMAKE_TARGET_SYSROOT=${CMAKE_BINARY_DIR}/Sysroots/Target\n");
    cmakelists_txt.append("        -DDOWNLOAD_DIRECTORY=${DOWNLOAD_DIRECTORY}\n");
    cmakelists_txt.append("    BINARY_DIR ${CMAKE_BINARY_DIR}/Target\n");
    cmakelists_txt.append("    INSTALL_COMMAND DESTDIR=${CMAKE_BINARY_DIR}/Sysroots/Target cmake --build . --target install\n");
    cmakelists_txt.append("    BUILD_ALWAYS true\n");
    cmakelists_txt.append(")\n");
    cmakelists_txt.append("set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${CMAKE_BINARY_DIR}/Target)\n");
    cmakelists_txt.append("\n\n");

    for (auto& tool : toolchain.host_tools()) {
        if (tool.value.add_as_target) {
            cmakelists_txt.append("add_custom_target(");
            cmakelists_txt.append(tool.key);
            cmakelists_txt.append("\n");
            cmakelists_txt.append("    COMMAND");
            if (tool.value.run_as_su)
                cmakelists_txt.append(" sudo ");

            cmakelists_txt.append(" ${CMAKE_COMMAND} -E env \"PATH=${CMAKE_BINARY_DIR}/Sysroots/Host/bin:$ENV{PATH}\" ");
            auto filename = FileSystemPath(toolchain.filename());
            auto abs_executable = FileProvider::the().make_absolute_path(tool.value.executable, filename.dirname());
            cmakelists_txt.append(abs_executable);
            if (!tool.value.flags.is_empty()) {
                cmakelists_txt.append(" ");
                auto flags = tool.value.flags;
                flags = replace_variables(flags, "root", "${PROJECT_ROOT_DIR}");
                flags = replace_variables(flags, "host_sysroot", "${CMAKE_BINARY_DIR}/Sysroots/Host");
                flags = replace_variables(flags, "target_sysroot", "${CMAKE_BINARY_DIR}/Sysroots/Target");
                cmakelists_txt.append(flags);
                cmakelists_txt.append("\n");
            }
            cmakelists_txt.append(")\n");
            cmakelists_txt.append("\n");
        }
    }

    // write out
    StringBuilder cmakelists_txt_filename;
    cmakelists_txt_filename.append(gen_path);
    cmakelists_txt_filename.append("/CMakeLists.txt");
    FILE* fd = fopen(cmakelists_txt_filename.build().characters(), "w+");

    if (!fd) {
        perror("fopen");
        return false;
    }

    auto cmakelists_txt_out = cmakelists_txt.build();
    auto bytes = fwrite(cmakelists_txt_out.characters(), 1, cmakelists_txt_out.length(), fd);
    if (bytes != cmakelists_txt_out.length()) {
        perror("fwrite");
        return false;
    }

    if (fclose(fd) < 0) {
        perror("fclose");
        return false;
    }

    return true;
}
