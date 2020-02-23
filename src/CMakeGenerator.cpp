#include "CMakeGenerator.h"
#include "DependencyResolver.h"
#include "FileProvider.h"
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

    cmakelists_txt.append(gen_header());
    cmakelists_txt.append(cmake_minimum_version());

    cmakelists_txt.append("project(");
    cmakelists_txt.append(image.name());
    cmakelists_txt.append(" C CXX ASM)\n\n");

    // generate install variables
    cmakelists_txt.append("include(GNUInstallDirs)\n");
    for (auto& installDir : image.install_dirs()) {
        cmakelists_txt.append("set(CMAKE_INSTALL_");
        cmakelists_txt.append(image.install_dir_to_string(installDir.key).to_uppercase());
        cmakelists_txt.append(" ");
        cmakelists_txt.append(installDir.value);
        cmakelists_txt.append(")\n");
    }
    cmakelists_txt.append("\n");

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

    if (package.type() != PackageType::Library && package.type() != PackageType::Executable && package.type() != PackageType::Deployment) {
        fprintf(stderr, "Package %s not of type Deployment, Library or Executable.\n", package.name().characters());
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

    cmakelists_txt.append("include(GNUInstallDirs)\n");

    auto targetName = package.name(); //get_target_name(package.name());

    if (package.type() == PackageType::Library || package.type() == PackageType::Executable) {

        // sources
        cmakelists_txt.append("set(SOURCES\n");
        for (auto& source : package.sources()) {
            cmakelists_txt.append("    \"");
            String source_replaced = replace_variables(source, "root", "${PROJECT_ROOT_DIR}");
            source_replaced = replace_variables(source_replaced, "gendata", "${CMAKE_CURRENT_LIST_DIR}");
            cmakelists_txt.append(source_replaced);
            cmakelists_txt.append("\"");
            cmakelists_txt.append("\n");
        }
        cmakelists_txt.append(")\n");

        // includes
        cmakelists_txt.append("set(INCLUDE_DIRS\n");
        for (auto& include : package.includes()) {
            cmakelists_txt.append("    \"");
            String incl = include;
            incl = replace_variables(include, "host_sysroot", "${CMAKE_SYSROOT}");

            StringBuilder gendata;
            gendata.append(SettingsProvider::the().get_string("gendata_directory").value_or(""));
            gendata.append("/package/");
            gendata.append(package.name());
            incl = replace_variables(include, "gendata", "${CMAKE_CURRENT_LIST_DIR}"); //gendata.build()

            cmakelists_txt.append(replace_variables(incl, "root", "${PROJECT_ROOT_DIR}"));

            cmakelists_txt.append("\"");
            cmakelists_txt.append("\n");
        }
        cmakelists_txt.append(")\n");

        // dependencies
        cmakelists_txt.append("set(STATIC_LINK_LIBRARIES\n");
        for (auto& dependency : package.dependencies()) {
            if (package.get_dependency_linkage(dependency.value) == LinkageType::Static) {
                cmakelists_txt.append("    \"");
                cmakelists_txt.append(dependency.key); //get_target_name(dependency.key));
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
                if (tool.value.flags.length()) {
                    if (tool.key == "cxx") {
                        cmakelists_txt.append("foreach(type DEBUG RELEASE MINSIZEREL RELWITHDEBINFO)\n");
                        cmakelists_txt.append("    set(CMAKE_CXX_FLAGS_${type} \"\")\n");
                        cmakelists_txt.append("endforeach()\n\n");
                        cmakelists_txt.append("foreach(file ${SOURCES})\n");
                        cmakelists_txt.append("    get_filename_component(extension ${file} EXT)\n");
                        cmakelists_txt.append("    string(SUBSTRING ${extension} 1 -1 extension)\n");
                        cmakelists_txt.append("    list(FIND CMAKE_CXX_SOURCE_FILE_EXTENSIONS ${extension} index)\n");
                        cmakelists_txt.append("    if(NOT \"${index}\" STREQUAL \"-1\")\n");
                        cmakelists_txt.append("        set_property(SOURCE ${file} PROPERTY COMPILE_FLAGS \"");
                        cmakelists_txt.append(tool.value.flags);
                        cmakelists_txt.append("\")\n");
                        cmakelists_txt.append("    endif()\n");
                        cmakelists_txt.append("endforeach()\n\n");

                    } else if (tool.key == "link") {
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
        for (auto& deployment : package.deploy()) {
            if (deployment.ptr()->type() == DeploymentType::Target) {
                target_dest = deployment.ptr()->dest();
                break;
            }
        }

        cmakelists_txt.append("install(TARGETS ");
        cmakelists_txt.append(targetName);
        cmakelists_txt.append("\n");

        if (package.type() == PackageType::Executable) {
            cmakelists_txt.append("   RUNTIME DESTINATION ");
            if (target_dest.is_empty())
                target_dest = "${BinDir}";
            cmakelists_txt.append(replace_dest_vars(target_dest));
            cmakelists_txt.append("\n");
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
                cmakelists_txt.append(replace_variables(deployment.ptr()->source(), "root", "${PROJECT_ROOT_DIR}"));
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
                cmakelists_txt.append(replace_variables(deployment.ptr()->source(), "root", "${PROJECT_ROOT_DIR}"));
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
                cmakelists_txt.append(replace_variables(deployment.ptr()->source(), "root", "${PROJECT_ROOT_DIR}"));
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

        cmakelists_txt.append("set(OUTPUT_FILES\n");
        for (auto& tuple : generator.value.input_output_tuples) {
            cmakelists_txt.append("    \"");
            String file_replaced = tuple.output;
            file_replaced = replace_variables(file_replaced, "gendata", "${CMAKE_CURRENT_LIST_DIR}");
            cmakelists_txt.append(file_replaced);
            cmakelists_txt.append("\"\n");
        }
        cmakelists_txt.append(")\n");

        cmakelists_txt.append("find_program(");
        cmakelists_txt.append(generator.key);
        cmakelists_txt.append(" ");
        cmakelists_txt.append(generator.key);
        cmakelists_txt.append(" PATHS \"");
        cmakelists_txt.append(SettingsProvider::the().get_string("build_directory").value_or(""));
        cmakelists_txt.append("/Toolchain/sysroot");
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
            String file_replaced = tuple.input;
            file_replaced = replace_variables(file_replaced, "root", "${PROJECT_ROOT_DIR}");
            file_replaced = replace_variables(file_replaced, "gendata", "${CMAKE_CURRENT_LIST_DIR}");
            cmakelists_txt.append(file_replaced);
            // Fixme: for now, we only accept generators that put the output to stdout. Has to be configurable...
            cmakelists_txt.append(" > ");

            file_replaced = tuple.output;
            file_replaced = replace_variables(file_replaced, "gendata", "${CMAKE_CURRENT_LIST_DIR}");
            cmakelists_txt.append(file_replaced);
            cmakelists_txt.append("\n");
        }
        cmakelists_txt.append("    COMMENT \"Executing ");
        cmakelists_txt.append(generator.key);
        cmakelists_txt.append(" for target ");
        cmakelists_txt.append(targetName);
        cmakelists_txt.append("\"\n)\n");

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

    StringBuilder direct_linkage_include;
    // sources for source file linkage
    direct_linkage_include.append("list(APPEND SOURCES\n");
    for (auto& source : package.sources()) {
        direct_linkage_include.append("    \"");
        String source_replaced = replace_variables(source, "root", "${PROJECT_ROOT_DIR}");
        source_replaced = replace_variables(source_replaced, "gendata", "${CMAKE_CURRENT_LIST_DIR}");
        direct_linkage_include.append(source_replaced);
        direct_linkage_include.append("\"");
        direct_linkage_include.append("\n");
    }
    direct_linkage_include.append(")\n");
    direct_linkage_include.append("\n");

    //Fixme: check if it is possible to also include the direct linkage include file of the dependencies of this package into this generated file

    // includes
    //    cmakelists_txt.append("list(APPEND INCLUDE_DIRS\n");
    //    for (auto& include : package.includes()) {
    //        direct_linkage_include.append("    \"");
    //        String incl = include;
    //        incl = replace_variables(include, "host_sysroot", "${CMAKE_SYSROOT}");

    //        direct_linkage_include.append(replace_variables(incl, "root", "${PROJECT_ROOT_DIR}"));

    //        direct_linkage_include.append("\"");
    //        direct_linkage_include.append("\n");
    //    }
    //    direct_linkage_include.append(")\n");
    direct_linkage_include.append("\n");

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

    StringBuilder direct_linkage_include_filename;
    direct_linkage_include_filename.append(path);
    direct_linkage_include_filename.append("/direct_linkage.include");
    fd = fopen(direct_linkage_include_filename.build().characters(), "w+");

    if (!fd)
        perror("fopen");

    auto sources_include_out = direct_linkage_include.build();
    bytes = fwrite(sources_include_out.characters(), 1, sources_include_out.length(), fd);
    if (bytes != sources_include_out.length())
        perror("fwrite");

    if (fclose(fd) < 0)
        perror("fclose");

    return true;
}

const String CMakeGenerator::gen_toolchain_content(const HashMap<String, Tool>& tools, Optional<const HashMap<String, HashMap<String, ToolConfiguration>>> toolchain_configuration)
{
    StringBuilder serenity_cmake;
    serenity_cmake.append(gen_header());

    serenity_cmake.append("if(LOADED)\n     return()\nendif()\nset(LOADED true)\n\n");
    serenity_cmake.append("if(NOT CMAKE_BUILD_TYPE)\n     set(CMAKE_BUILD_TYPE Debug)\nendif()\n\n");

    for (auto tool : tools) {
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
    serenity_cmake.append("set(CMAKE_INSTALL_PREFIX \"/usr\" CACHE INTERNAL \"\" FORCE)");
    serenity_cmake.append("\n");
    serenity_cmake.append("\n");

    if (toolchain_configuration.has_value()) {
        for (auto& configuration : toolchain_configuration.value()) {
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
    }

    serenity_cmake.append("set(CMAKE_ASM_CREATE_STATIC_LIBRARY \"<CMAKE_AR> crT <TARGET> <LINK_FLAGS> <OBJECTS>\")\n\n");
    serenity_cmake.append("set(CMAKE_ASM_FLAGS_DEBUG \"\" CACHE INTERNAL \"\" FORCE)\n");
    serenity_cmake.append("set(CMAKE_ASM_FLAGS \"\" CACHE INTERNAL \"\" FORCE)\n");
    serenity_cmake.append("\n");

    return serenity_cmake.build();
}

void CMakeGenerator::gen_toolchain(const Toolchain& toolchain, const Vector<Package>& packages_to_build)
{
    /**
     * This generates the toolchain file: serenity.cmake
     * This generates the toolchain file: host.cmake
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
     * set(CMAKE_SYSROOT "build/toolchain/sysroot")
     */

    auto gen_path = SettingsProvider::the().get_string("gendata_directory").value_or("");
    if (gen_path.is_empty()) {
        return;
    }

    if (!create_dir(gen_path, "toolchain"))
        return;

    //fprintf(stdout, "Gendata directory: %s\n", gen_path.value().characters());

    String serenity_cmake = gen_toolchain_content(toolchain.host_tools(), toolchain.configuration());
    String host_cmake = gen_toolchain_content(toolchain.build_tools(), {});

    FILE* fd;

    // write out
    StringBuilder serenity_cmake_filename;
    serenity_cmake_filename.append(gen_path);
    serenity_cmake_filename.append("/toolchain/serenity.cmake");
    fd = fopen(serenity_cmake_filename.build().characters(), "w+");

    if (!fd)
        perror("fopen");

    auto bytes = fwrite(serenity_cmake.characters(), 1, serenity_cmake.length(), fd);
    if (bytes != serenity_cmake.length())
        perror("fwrite");

    if (fclose(fd) < 0)
        perror("fclose");

    StringBuilder host_cmake_filename;
    host_cmake_filename.append(gen_path);
    host_cmake_filename.append("/toolchain/host.cmake");
    fd = fopen(host_cmake_filename.build().characters(), "w+");

    if (!fd)
        perror("fopen");

    bytes = fwrite(host_cmake.characters(), 1, host_cmake.length(), fd);
    if (bytes != host_cmake.length())
        perror("fwrite");

    if (fclose(fd) < 0)
        perror("fclose");

    StringBuilder cmakelists_txt;

    cmakelists_txt.append(gen_header());
    cmakelists_txt.append(cmake_minimum_version());
    cmakelists_txt.append(project_root_dir());

    cmakelists_txt.append("string(ASCII 27 ESCAPE_CHAR)\n");
    cmakelists_txt.append("macro(warning_message msg)\n");
    cmakelists_txt.append("    message(STATUS \"${ESCAPE_CHAR}[1;${92}m${msg}${ESCAPE_CHAR}[0m\")\n");
    cmakelists_txt.append("endmacro()\n\n");

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
#ifdef DEBUG_META
        fprintf(stderr, "Package to build: %s\n", package.name().characters());
#endif

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
            cmakelists_txt.append("    DEPENDS ");
            cmakelists_txt.append(depends_str);
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
                                flags.append(options.get("flags").as_string());
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
            cmakelists_txt.append("    CMAKE_ARGS\n");

            if (package.toolchain_options().contains("build")) {
                auto options = pkg_toolchain_options.get("build").value();

                if (options.has("use_toolchain")) {
                    if (options.get("use_toolchain").as_string() == "target") {
                        cmakelists_txt.append("      -DCMAKE_SYSROOT=${CMAKE_BINARY_DIR}/sysroot\n");
                        cmakelists_txt.append("      -DCMAKE_TOOLCHAIN_FILE=${CMAKE_CURRENT_LIST_DIR}/serenity.cmake\n");
                    }
                }
            } else if (package.machine() == "host") {
                cmakelists_txt.append("      -DCMAKE_TOOLCHAIN_FILE=${CMAKE_CURRENT_LIST_DIR}/host.cmake\n");
            }
            cmakelists_txt.append("    BINARY_DIR ${CMAKE_BINARY_DIR}/");
            cmakelists_txt.append(package.name());
            cmakelists_txt.append("\n");
            cmakelists_txt.append("    INSTALL_COMMAND DESTDIR=${CMAKE_BINARY_DIR}/sysroot cmake --build . --target install");
            //            cmakelists_txt.append("\n");
            //            cmakelists_txt.append("    BUILD_ALWAYS true");
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

void CMakeGenerator::gen_root(const Toolchain& toolchain)
{

    auto gen_path = SettingsProvider::the().get_string("gendata_directory").value_or("");

    if (gen_path.is_empty()) {
        return;
    }

    StringBuilder cmakelists_txt;

    cmakelists_txt.append(gen_header());
    cmakelists_txt.append(cmake_minimum_version());
    cmakelists_txt.append(project_root_dir());

    cmakelists_txt.append("include(ExternalProject)\n\n");
    cmakelists_txt.append("ExternalProject_Add(Toolchain\n");
    cmakelists_txt.append("    PREFIX ${CMAKE_BINARY_DIR}/Toolchain\n");
    cmakelists_txt.append("    SOURCE_DIR ${CMAKE_SOURCE_DIR}/toolchain\n");
    cmakelists_txt.append("    CMAKE_ARGS\n");
    cmakelists_txt.append("        -DCMAKE_TOOLCHAIN_FILE=${CMAKE_CURRENT_LIST_DIR}/toolchain/host.cmake\n");
    cmakelists_txt.append("    BINARY_DIR ${CMAKE_BINARY_DIR}/Toolchain\n");
    cmakelists_txt.append("    INSTALL_COMMAND \"\"\n");
    cmakelists_txt.append(")\n");
    cmakelists_txt.append("# To clean up toolchain on make clean, uncomment the next line\n");
    cmakelists_txt.append("#set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES Toolchain)\n");
    cmakelists_txt.append("\n");
    cmakelists_txt.append("ExternalProject_Add(Target\n");
    cmakelists_txt.append("    DEPENDS Toolchain\n");
    cmakelists_txt.append("    PREFIX ${CMAKE_BINARY_DIR}/Target\n");
    cmakelists_txt.append("    SOURCE_DIR ${CMAKE_SOURCE_DIR}/image/default-image\n");
    cmakelists_txt.append("    CMAKE_ARGS\n");
    cmakelists_txt.append("        -DCMAKE_TOOLCHAIN_FILE=${CMAKE_CURRENT_LIST_DIR}/toolchain/serenity.cmake\n");
    cmakelists_txt.append("        -DCMAKE_SYSROOT=${CMAKE_BINARY_DIR}/Toolchain/sysroot\n");
    cmakelists_txt.append("    BINARY_DIR ${CMAKE_BINARY_DIR}/Target\n");
    cmakelists_txt.append("    INSTALL_COMMAND DESTDIR=${CMAKE_BINARY_DIR}/Target/sysroot cmake --build . --target install\n");
    cmakelists_txt.append("    BUILD_ALWAYS true\n");
    cmakelists_txt.append(")\n");
    cmakelists_txt.append("set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES Target)\n");
    cmakelists_txt.append("\n");
    cmakelists_txt.append("\n");
    cmakelists_txt.append("\n");

    for (auto& tool : toolchain.host_tools()) {
        if (tool.value.add_as_target) {
            cmakelists_txt.append("add_custom_target(");
            cmakelists_txt.append(tool.key);
            cmakelists_txt.append("\n");
            cmakelists_txt.append("    COMMAND");
            if (tool.value.run_as_su)
                cmakelists_txt.append(" sudo -E PATH=\"$PATH\"");

            cmakelists_txt.append(" ${CMAKE_COMMAND} -E env \"PATH=${CMAKE_BINARY_DIR}/Toolchain/sysroot/bin:$ENV{PATH}\" ");
            auto filename = FileSystemPath(toolchain.filename());
            auto abs_executable = tool.value.executable;
            FileProvider::the().update_if_relative(abs_executable, filename.dirname());
            cmakelists_txt.append(abs_executable);
            if (!tool.value.flags.is_empty()) {
                cmakelists_txt.append(" ");
                auto flags = tool.value.flags;
                flags = replace_variables(flags, "root", "${PROJECT_ROOT_DIR}");
                flags = replace_variables(flags, "target_sysroot", "${CMAKE_BINARY_DIR}/Target/sysroot");
                cmakelists_txt.append(flags);
                cmakelists_txt.append("\n");
            }
            cmakelists_txt.append("    EXCLUDE_FROM_ALL\n");
            cmakelists_txt.append(")\n");
            cmakelists_txt.append("\n");
        }
    }

    // write out
    StringBuilder cmakelists_txt_filename;
    cmakelists_txt_filename.append(gen_path);
    cmakelists_txt_filename.append("/CMakeLists.txt");
    FILE* fd = fopen(cmakelists_txt_filename.build().characters(), "w+");

    if (!fd)
        perror("fopen");

    auto cmakelists_txt_out = cmakelists_txt.build();
    auto bytes = fwrite(cmakelists_txt_out.characters(), 1, cmakelists_txt_out.length(), fd);
    if (bytes != cmakelists_txt_out.length())
        perror("fwrite");

    if (fclose(fd) < 0)
        perror("fclose");
}
