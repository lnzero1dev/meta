#include "CMakeGenerator.h"
#include "FileProvider.h"
#include "ImageDB.h"
#include "PackageDB.h"
#include "SettingsProvider.h"
#include "ToolchainDB.h"
#include <AK/JsonValue.h>
#include <AK/String.h>
#include <AK/Types.h>
#include <stdio.h>

enum class PrimaryCommand : u8 {
    None = 0,
    Build,
    Generate,
    Config,
    Run
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
        if (file->filename().is_empty() || !file->exists())
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
        if (file->filename().is_empty() || !file->exists())
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
#ifdef META_DEBUG
                        fprintf(stderr, "Found toolchain %s, adding to DB.\n", key.characters());
#endif
                        ToolchainDB::the().add(key, value.as_object());
                    });
                } else if (key == "package") {
                    value.as_object().for_each_member([&](auto& key, auto& value) {
#ifdef META_DEBUG
                        fprintf(stderr, "Found package %s, adding to DB.\n", key.characters());
#endif
                        bool result = PackageDB::the().add(filename, key, value.as_object());
                        if (!result) {
                            fprintf(stderr, "Could not add package to DB: Already existing\n");
                            fprintf(stderr, "- Current Try: %s from file %s\n", key.characters(), filename.characters());
                            auto package = PackageDB::the().get(key);
                            fprintf(stderr, "- Existing: %s from file %s\n", key.characters(), package->filename().characters());
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
#ifdef META_DEBUG
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
    u32 number_of_native_tools = 0;
    u32 number_of_file_tool_mappings = 0;
    ToolchainDB::the().for_each_toolchain([&](auto& name, auto& toolchain) {
        toolchain_list.append(name);
        toolchain_list.append(", ");
        number_of_target_tools += toolchain.target_tools().size();
        number_of_native_tools += toolchain.native_tools().size();
        number_of_file_tool_mappings += toolchain.file_tool_mapping().size();
        return IterationDecision::Continue;
    });

    auto& packages = PackageDB::the().packages();
    StringBuilder package_list;
    u32 number_of_source_files = 0;
    u32 number_of_include_directories = 0;
    u16 type_library = 0;
    u16 type_executable = 0;
    u16 type_collection = 0;
    u16 type_unknown = 0;
    PackageDB::the().for_each_package([&](auto& name, auto& package) {
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
        case PackageType::Unknown:
            ++type_unknown;
            break;
        }

        return IterationDecision::Continue;
    });

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
    fprintf(stdout, "Target tools: %i\n", number_of_target_tools);
    fprintf(stdout, "Native tools: %i\n", number_of_native_tools);
    fprintf(stdout, "File extension tool mappings: %i\n", number_of_file_tool_mappings);
    fprintf(stdout, "----- ---------- -----\n");
    fprintf(stdout, "Packages: %i\n", packages.size());
    if (packages.size())
        fprintf(stdout, "* %s\033[2D \n", package_list.build().characters());
    fprintf(stdout, "Packages that are Libraries: %i\n", type_library);
    fprintf(stdout, "Packages that are Executables: %i\n", type_executable);
    fprintf(stdout, "Packages that are Collections: %i\n", type_collection);
    fprintf(stdout, "Packages that are Unknown type: %i\n", type_unknown);
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
        int arg_len = strlen(argv[1]);

        if (strncmp("build", argv[1], arg_len) == 0 && arg_len == 5) {
            minarg = 3;
            cmd = PrimaryCommand::Build;
        } else if (strncmp("gen", argv[1], arg_len) == 0 && arg_len == 3) {
            minarg = 3;
            cmd = PrimaryCommand::Generate;
        } else if (strncmp("config", argv[1], arg_len) == 0 && arg_len == 6) {
            cmd = PrimaryCommand::Config;
            ++minarg;
            if (argc >= minarg) {
                int subarg_len = strlen(argv[2]);
                if (strncmp(argv[2], "set", subarg_len) == 0 && subarg_len == 3) {
                    minarg = 5;
                    config_subcmd = ConfigSubCommand::Set;
                } else if (strncmp(argv[2], "get", subarg_len) == 0 && subarg_len == 3) {
                    minarg = 4;
                    config_subcmd = ConfigSubCommand::Get;
                } else if (strncmp(argv[2], "list", subarg_len) == 0 && subarg_len == 4) {
                    minarg = 3;
                    config_subcmd = ConfigSubCommand::List;
                } else {
                    minarg = -1;
                }
            }
        } else if (strncmp(argv[1], "run", arg_len) == 0 && arg_len == 3) {
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
        return 0;
    }

    SettingsProvider& settingsProvider = SettingsProvider::the();

    Vector<String> files;
    String root = FileProvider::the().current_dir();
    files = FileProvider::the().glob_all_meta_json_files(root);

#ifdef META_DEBUG
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
        fprintf(stderr, "Generate!\n");

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
            PackageDB::the().for_each_package([&](auto& name, auto&) {
                if (name == parameter) {
                    isPackage = true;
                    return IterationDecision::Break;
                }
                return IterationDecision::Continue;
            });

        // TODO: Do what the framework must do, before the generator is invoked:
        // * calculate dependencies
        // * check for missing executables / dependencies
        // * calculate other requirements
        // * do plausbility check, resolution of missing things....

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
                if (isImage) {
                    cmakegen.gen_image(parameter);
                } else if (isPackage) {
                    cmakegen.gen_package(parameter);
                }
                break;
            }
            default:
                fprintf(stderr, "Invalid build configurator configured.");
            }
        } else
            fprintf(stderr, "Invalid build configurator configured.");

        //
        //    Toolchain& toolchain = Toolchain::the();
        //    if (!toolchain.check_native_apps()) {
        //        fprintf(stderr, "Some native apps missing!\n");
        //        return -1;
        //    }
    }

    if (cmd == PrimaryCommand::Build) {
        fprintf(stderr, "Build!\n");
    }

    statistics();

    return 0;
}
