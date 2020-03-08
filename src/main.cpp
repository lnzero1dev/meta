#include "CMakeGenerator.h"
#include "DependencyResolver.h"
#include "FileProvider.h"
#include "ImageDB.h"
#include "PackageDB.h"
#include "SettingsProvider.h"
#include "ToolchainDB.h"
#include <AK/JsonValue.h>
#include <AK/String.h>
#include <AK/Types.h>
#include <LibCore/File.h>
#include <stdio.h>

enum class PrimaryCommand : u8 {
    None = 0,
    Build,
    Generate,
    Config,
    Run,
    Statistics
};

enum class ConfigSubCommand : u8 {
    None = 0,
    Get,
    Set,
    List
};

Vector<String> s_loaded_settings_files;

void load_meta_settings(Vector<String> files)
{
    auto file = Core::File::construct();
    for (auto& filename : files) {
        file->set_filename(filename);
        if (file->filename().is_empty() || !file->exists(filename))
            continue;

        /* load json file */
        if (!file->open(Core::IODevice::ReadOnly)) {
            fprintf(stderr, "Couldn't open %s for reading: %s\n", file->filename().characters(), file->error_string());
            continue;
        }

        auto json = JsonValue::from_string(file->read_all());

        if (json.is_object())
            json.as_object().for_each_member([&](auto& key, auto& value) {
                if (key == "settings")
                    value.as_object().for_each_member([&](auto& key, auto& value) {
                        SettingsPriority priority { SettingsPriority::Undefined };
                        if (key == "user")
                            priority = SettingsPriority::User;
                        else if (key == "project")
                            priority = SettingsPriority::Project;

                        if (priority != SettingsPriority::Undefined) {
                            if (SettingsProvider::the().add(filename, priority, value.as_object())) {
                                s_loaded_settings_files.append(filename);
                            }
                        } else
                            fprintf(stderr, "Unknown settings priority %s found in JSON file.\n", key.characters());
                    });
            });
    }
}

void load_meta_all(Vector<String> files)
{
    auto file = Core::File::construct();
    for (auto& filename : files) {
        file->set_filename(filename);
        if (file->filename().is_empty() || !file->exists(filename))
            continue;

        /* load json file */
        if (!file->open(Core::IODevice::ReadOnly)) {
            fprintf(stderr, "Couldn't open %s for reading: %s\n", file->filename().characters(), file->error_string());
            continue;
        }

        auto json = JsonValue::from_string(file->read_all());

        if (json.is_object()) {
            json.as_object().for_each_member([&](auto& key, auto& value) {
                if (key == "toolchain") {
                    value.as_object().for_each_member([&](auto& key, auto& value) {
#ifdef DEBUG_META
                        fprintf(stderr, "Found toolchain %s, adding to DB.\n", key.characters());
#endif
                        ToolchainDB::the().add(filename, key, value.as_object());
                    });
                } else if (key == "package") {
                    value.as_object().for_each_member([&](auto& key, auto& value) {
#ifdef DEBUG_META
                        fprintf(stderr, "Found package %s, adding to DB.\n", key.characters());
#endif
                        bool result = PackageDB::the().add(filename, key, value.as_object());
                        if (!result) {
                            fprintf(stderr, "Could not add package to DB: Already existing\n");
                            fprintf(stderr, "- Current Try: %s from file %s\n", key.characters(), filename.characters());
                            //                            auto package = PackageDB::the().get(key);
                            //                            fprintf(stderr, "- Existing: %s from file %s\n", key.characters(), package->filename().characters());
                        }
                    });
                } else if (key == "settings") {
                    if (s_loaded_settings_files.find(filename) == s_loaded_settings_files.end()) // not already loaded
                        value.as_object().for_each_member([&](auto& key, auto& value) {
                            SettingsPriority priority { SettingsPriority::Undefined };
                            if (key == "user")
                                priority = SettingsPriority::User;
                            else if (key == "project")
                                priority = SettingsPriority::Project;

                            if (priority != SettingsPriority::Undefined)
                                SettingsProvider::the().add(filename, priority, value.as_object());
                            else
                                fprintf(stderr, "Unknown settings priority %s found in JSON file.\n", key.characters());
                        });
                } else if (key == "image") {
                    value.as_object().for_each_member([&](auto& key, auto& value) {
#ifdef DEBUG_META
                        fprintf(stderr, "Found image %s, adding to DB.\n", key.characters());
#endif
                        bool result = ImageDB::the().add(filename, key, value.as_object());
                        if (!result) {
                            fprintf(stderr, "Could not add image to DB: Already existing\n");
                            fprintf(stderr, "- Current Try: %s from file %s\n", key.characters(), filename.characters());
                            auto image = ImageDB::the().get(key);
                            fprintf(stderr, "- Existing: %s from file %s\n", key.characters(), image->filename().characters());
                        }
                    });
                } else {
                    fprintf(stderr, "Unknown key %s found in JSON file.\n", key.characters());
                }
            });
        } else if (json.is_array()) {
            fprintf(stderr, "JSON file malformed: %s\n", file->filename().characters());
            continue;
        }
    }
}

void statistics()
{
    auto& toolchains = ToolchainDB::the().toolchains();
    StringBuilder toolchain_list;
    u32 number_of_target_tools = 0;
    u32 number_of_host_tools = 0;
    u32 number_of_build_tools = 0;
    u32 number_of_file_tool_mappings = 0;
    ToolchainDB::the().for_each_toolchain([&](auto& name, auto& toolchain) {
        toolchain_list.append(name);
        toolchain_list.append(", ");
        number_of_target_tools += toolchain.target_tools().size();
        number_of_host_tools += toolchain.host_tools().size();
        number_of_build_tools += toolchain.build_tools().size();
        number_of_file_tool_mappings += toolchain.file_tool_mapping().size();
        return IterationDecision::Continue;
    });

    StringBuilder package_list;
    u32 number_of_source_files = 0;
    u32 number_of_include_directories = 0;
    u16 type_library = 0;
    u16 type_executable = 0;
    u16 type_collection = 0;
    u16 type_deployment = 0;
    u16 type_script = 0;
    u16 type_undefined = 0;
    u32 packages_count = 0;

    auto package_iterator = [&](auto& name, auto& package) {
        package_list.append(name);
        package_list.append(", ");
        number_of_source_files += package.sources().size();
        number_of_include_directories += package.includes().size();
        switch (package.type()) {
        case PackageType::Library:
            ++type_library;
            break;
        case PackageType::Executable:
            ++type_executable;
            break;
        case PackageType::Collection:
            ++type_collection;
            break;
        case PackageType::Deployment:
            ++type_deployment;
            break;
        case PackageType::Script:
            ++type_script;
            break;
        case PackageType::Undefined:
            ++type_undefined;
            break;
        }
        ++packages_count;

        return IterationDecision::Continue;
    };

    PackageDB::the().for_each_build_package(package_iterator);
    PackageDB::the().for_each_host_package(package_iterator);
    PackageDB::the().for_each_target_package(package_iterator);

    auto& images = ImageDB::the().images();
    StringBuilder image_list;
    ImageDB::the().for_each_image([&](auto& name, auto&) {
        image_list.append(name);
        image_list.append(", ");
        return IterationDecision::Continue;
    });

    fprintf(stdout, "----- STATISTICS -----\n");
    fprintf(stdout, "Toolchains: %i\n", toolchains.size());
    if (toolchains.size())
        fprintf(stdout, "* %s\033[2D \n", toolchain_list.build().characters());
    fprintf(stdout, "Build tools: %i\n", number_of_build_tools);
    fprintf(stdout, "Host tools: %i\n", number_of_host_tools);
    fprintf(stdout, "Target tools: %i\n", number_of_target_tools);
    fprintf(stdout, "File extension tool mappings: %i\n", number_of_file_tool_mappings);
    fprintf(stdout, "----- ---------- -----\n");
    fprintf(stdout, "Packages: %i\n", packages_count);
    if (packages_count)
        fprintf(stdout, "* %s\033[2D \n", package_list.build().characters());
    fprintf(stdout, "Packages with type Library: %i\n", type_library);
    fprintf(stdout, "Packages with type Executable: %i\n", type_executable);
    fprintf(stdout, "Packages with type Collection: %i\n", type_collection);
    fprintf(stdout, "Packages with type Deployment: %i\n", type_deployment);
    fprintf(stdout, "Packages with type Script: %i\n", type_script);
    fprintf(stdout, "Packages with undefined type: %i\n", type_undefined);
    fprintf(stdout, "Number of source files: %i\n", number_of_source_files);
    fprintf(stdout, "Number of include directories: %i\n", number_of_include_directories);
    fprintf(stdout, "----- ---------- -----\n");
    fprintf(stdout, "Images: %i\n", images.size());
    if (images.size())
        fprintf(stdout, "* %s\033[2D \n", image_list.build().characters());
    fprintf(stdout, "----------------------\n");
}

int main(int argc, char** argv)
{
    int minarg = 2;
    PrimaryCommand cmd = PrimaryCommand::None;
    ConfigSubCommand config_subcmd = ConfigSubCommand::None;

    if (argc >= minarg) {
        String arg1 { argv[1], strlen(argv[1]) };

        if (arg1 == "build") {
            minarg = 3;
            cmd = PrimaryCommand::Build;
        } else if (arg1 == "st" || arg1 == "stats") {
            minarg = 2;
            cmd = PrimaryCommand::Statistics;
        } else if (arg1 == "gen") {
            minarg = 3;
            cmd = PrimaryCommand::Generate;
        } else if (arg1 == "config") {
            cmd = PrimaryCommand::Config;
            ++minarg;
            if (argc >= minarg) {
                String arg2 { argv[2], strlen(argv[2]) };

                if (arg2 == "set") {
                    minarg = 5;
                    config_subcmd = ConfigSubCommand::Set;
                } else if (arg2 == "get") {
                    minarg = 4;
                    config_subcmd = ConfigSubCommand::Get;
                } else if (arg2 == "list") {
                    minarg = 3;
                    config_subcmd = ConfigSubCommand::List;
                } else {
                    minarg = -1;
                }
            }
        } else if (arg1 == "run") {
            cmd = PrimaryCommand::Run;
            minarg = 2;
        }
    }

    if (argc < minarg || minarg < 0 || cmd == PrimaryCommand::None) {
        fprintf(stderr, "usage: \n");
        if (cmd == PrimaryCommand::None || (cmd == PrimaryCommand::Config && config_subcmd == ConfigSubCommand::None)) {
            fprintf(stderr, "  Config:\n");
            fprintf(stderr, "    meta config list\n");
            fprintf(stderr, "    meta config get <param>\n");
            fprintf(stderr, "    meta config set <param> <value>\n");
        }
        if (cmd == PrimaryCommand::None || cmd == PrimaryCommand::Generate) {
            fprintf(stderr, "  Generate:\n");
            //fprintf(stderr, "    meta gen <toolchain>\n");
            fprintf(stderr, "    meta gen <package>\n");
            fprintf(stderr, "    meta gen <image>\n");
        }
        if (cmd == PrimaryCommand::None || cmd == PrimaryCommand::Build) {
            fprintf(stderr, "  Build:\n");
            fprintf(stderr, "    meta build <package>\n");
        }
        if (cmd == PrimaryCommand::None || cmd == PrimaryCommand::Run) {
            fprintf(stderr, "  Run:\n");
            fprintf(stderr, "    meta run [<image>]\n");
        }
        if (cmd == PrimaryCommand::None) {
            fprintf(stderr, "  Statistics:\n");
            fprintf(stderr, "    meta st\n");
            fprintf(stderr, "    meta stats\n");
        }
        return 0;
    }

    SettingsProvider& settingsProvider = SettingsProvider::the();

    Vector<String> files;
    String root = FileProvider::the().current_dir();
    FileProvider::the().for_each_parent_directory_including_current_dir([&](auto& directory) {
        auto dir_files = FileProvider::the().glob("*.m.json", directory);
        for (auto& file : dir_files) {
            files.append(file);
        }
        return IterationDecision::Continue;
    });

#ifdef DEBUG_META
    fprintf(stderr, "Searching for meta json files in: %s\n", root.characters());
    for (auto& file : files) {
        fprintf(stderr, "* %s\n", file.characters());
    }
#endif

    // First, load all files that we can find from the current directory.
    // This is needed to find the project settings. After loading the
    // Settings, we likely know that we
    load_meta_settings(files);

    if (cmd == PrimaryCommand::Config) {
        switch (config_subcmd) {
        case ConfigSubCommand::Set: {
            break;
        }
        case ConfigSubCommand::List: {
            settingsProvider.list_all();
            break;
        }
        case ConfigSubCommand::Get: {
            String parameter { argv[3], strlen(argv[3]) };
            Optional<SettingsParameter> param = settingsProvider.get(parameter);
            if (param.has_value()) {
                fprintf(stdout, "%s: %s (set in %s)\n", parameter.characters(), param.value().as_string().characters(), param.value().filename().characters());
            } else {
                fprintf(stderr, "No valid parameter: %s\n", parameter.characters());
                return -1;
            }
            break;
        }
        case ConfigSubCommand::None:
        default: {
            return -1;
            break;
        }
        }
        return 0;
    }

    files = FileProvider::the().glob_all_meta_json_files(SettingsProvider::the().get_string("root").value_or(root));
    load_meta_all(files);

    if (cmd == PrimaryCommand::Generate) {
#ifdef DEBUG_META
        fprintf(stderr, "Generate!\n");
#endif
        auto configured_toolchain = SettingsProvider::the().get_string("toolchain");
        auto toolchain = ToolchainDB::the().get(configured_toolchain.value_or("default"));
        if (!toolchain) {
            if (configured_toolchain.has_value()) {
                fprintf(stderr, "Wrong toolchain configured: %s!\n", configured_toolchain.value().characters());
                StringBuilder toolchain_list;
                ToolchainDB::the().for_each_toolchain([&](auto& name, auto&) {
                    toolchain_list.append(name);
                    toolchain_list.append(", ");
                    return IterationDecision::Continue;
                });
                fprintf(stdout, "Available toolchains: %s\033[2D \n", toolchain_list.build().characters());
            }
        }

        // TODO: For the toolchain it is essential that only the tools of the used toolchain are beeing checked.
        // For example, tools for a different toolchain may be not available on your system. Thererfore, the
        // Dependency resolver shall only check the tools of the used toolchain. Furthermore, it could also check
        // only the dependencies of the selected package, or the packages contained in the selecte image.
        // For now, all dependencies are checked, regardless if they must be built or not.

        bool isImage = false;
        bool isPackage = false;
        String parameter { argv[2], strlen(argv[2]) };

        // check if given argument is an image or an package
        ImageDB::the().for_each_image([&](auto& name, auto&) {
            if (name == parameter) {
                isImage = true;
                return IterationDecision::Break;
            }
            return IterationDecision::Continue;
        });

        if (!isImage)
            PackageDB::the().for_each_target_package([&](auto& name, auto&) {
                if (name == parameter) {
                    isPackage = true;
                    return IterationDecision::Break;
                }
                return IterationDecision::Continue;
            });

        if (!isImage && !isPackage) {
            fprintf(stderr, "No package or image name matching provided name: %s.\n", parameter.characters());
            return -1;
        }

        // TODO: Do what the framework must do, before the generator is invoked:
        // * calculate dependencies
        // * -> check for missing executables / dependencies and exit if something cannot be found
        // * calculate needed native tools
        // optional for build [
        // * create package build queue (in order... native tools, packages)
        // * work on queue that contains all leaves, if all dependencies of a package are satisfied, enqueue package
        //   _ run generator(s) depending on package sources (file extension tool mappings --> generators must be natively built before!)
        //   _ build
        //   _ on_finish() callback's
        // ]
        // * if image is being built, execute image tools (build)

        Vector<String> missing_dependencies;

        PackageDB::the().for_each_target_package([&](auto&, auto& package) {
            auto node = DependencyResolver::the().get_dependency_tree(package);
            auto& missing = DependencyResolver::the().missing_dependencies(node);
            for (auto& dependency : missing)
                missing_dependencies.append(dependency);

            return IterationDecision::Continue;
        });

        if (missing_dependencies.size()) {
            fprintf(stderr, "Could not resolve all dependencies. Missing dependencies:\n");
            for (auto& dependency : missing_dependencies) {
                fprintf(stderr, "* %s\n", dependency.characters());
            }
            return -1;
        }

        // TODO: Lookahead into the future, that would be nice to have plugins to load
        // Find/load the generator plugin and execute it... for now, everything is static.
        // auto buildGenerator = settingsProvider.get("build_generator").value_or({ "internal", BuildGenerator::Undefined }).as_buildgenerator();
        // GeneratorPluginsLoader::the().Initialize(buildGenerator); // Find all loadable plugins and initialize them
        // GeneratorPluginsLoader::the().Generate(); // Generate everything

        auto optBuildGenerator = settingsProvider.get("build_generator");
        if (optBuildGenerator.has_value()) {
            auto buildGenerator = optBuildGenerator.value().as_buildgenerator();
            switch (buildGenerator) {
            case BuildGenerator::CMake: {
                auto& cmakegen = CMakeGenerator::the();

                ASSERT(toolchain);

                if (isImage) {

                    Vector<const Package*> host_packages_in_order;
                    auto image = ImageDB::the().get(parameter);
                    ASSERT(image);
                    if (image->install_all()) {
                        PackageDB::the().for_each_target_package([&](auto&, auto& data) {
                            if (cmakegen.gen_package(data)) {
                                auto node = DependencyResolver::the().get_dependency_tree(data);
#ifdef DEBUG_META
                                fprintf(stderr, "Resolving dependency tree for package: %s\n", data.name().characters());
#endif
                                // add all packages, beginning from leave
                                DependencyNode::start_by_leave(node, [&](auto& package) {
                                    bool found = false;
                                    for (auto& p : host_packages_in_order) {
                                        if (p->name() == package.name()) {
                                            found = true;
#ifdef DEBUG_META
                                            fprintf(stderr, " -> Found dep: %s\n", package.name().characters());
#endif
                                            break;
                                        }
                                    }

                                    if (!found) {
                                        host_packages_in_order.append(&package);
#ifdef DEBUG_META
                                        fprintf(stderr, " --> Adding dep: %s\n", package.name().characters());
#endif
                                    }
                                });
                            }
                            return IterationDecision::Continue;
                        });
                    } else {
                        for (auto& package_name : image->install()) {
                            Package* package;
                            if (!(package = PackageDB::the().get(MachineType::Target, package_name))) {
                                fprintf(stderr, "Image %s configured to install package %s. Package not found!", parameter.characters(), package_name.characters());
                                return -1;
                            }
                            ASSERT(package);
                            if (cmakegen.gen_package(*package)) {
                                auto node = DependencyResolver::the().get_dependency_tree(*package);
                                // add all packages, beginning from leave
                                DependencyNode::start_by_leave(node, [&](auto& package) {
                                    bool found = false;
                                    for (auto& p : host_packages_in_order) {
                                        if (p->name() == package.name()) {
                                            found = true;
                                            break;
                                        }
                                    }

                                    if (!found)
                                        host_packages_in_order.append(&package);
                                });
                            }
                        }
                    }
                    fprintf(stdout, "Generate Image: %s!\n", image->name().characters());
                    cmakegen.gen_image(*image, host_packages_in_order);
                    cmakegen.gen_root(*toolchain, argc, argv);

                } else if (isPackage) {
                    Package* package = nullptr;
                    if (!(package = PackageDB::the().get(MachineType::Target, parameter))) {
                        fprintf(stderr, "Package %s not found!", parameter.characters());
                        return -1;
                    }
                    ASSERT(package);
                    cmakegen.gen_package(*package);
                }

                cmakegen.gen_toolchain(*toolchain, files);
                break;
            }
            default:
                fprintf(stderr, "Invalid build configurator configured.");
            }
        } else {
            fprintf(stderr, "Invalid build configurator configured.");
        }
    }

    if (cmd == PrimaryCommand::Build) {
        fprintf(stdout, "Build!\n");
    }

    if (cmd == PrimaryCommand::Run) {
        fprintf(stdout, "Run!\n");
    }

    if (cmd == PrimaryCommand::Statistics) {
        statistics();
    }

    return 0;
}
