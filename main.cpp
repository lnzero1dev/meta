#include "FileProvider.h"
#include "PackageDB.h"
#include "SettingsProvider.h"
#include "ToolchainDB.h"
#include <AK/JsonObject.h>
#include <AK/JsonValue.h>
#include <AK/String.h>
#include <AK/Types.h>
#include <algorithm>
#include <cstring>
#include <stdio.h>

enum class PrimaryCommand : u8 {
    None = 0,
    Build,
    Config,
    Run
};

enum class ConfigSubCommand : u8 {
    None = 0,
    Get,
    Set,
    List
};

void load_meta_json_files(Vector<String> files, bool settings_only = false)
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

        auto file_contents = file->read_all();
        auto json = JsonValue::from_string(file_contents);
        auto parse_settings = [&](auto& key, auto& value) {
            SettingsPriority priority { SettingsPriority::Undefined };
            if (key == "user")
                priority = SettingsPriority::User;
            else if (key == "project")
                priority = SettingsPriority::Project;

            if (priority != SettingsPriority::Undefined)
                SettingsProvider::the().add(filename, priority, value.as_object());
            else
                fprintf(stderr, "Unknown settings priority %s found in JSON file.\n", key.characters());
        };

        if (json.is_object()) {
            json.as_object().for_each_member([&](auto& key, auto& value) {
                if (settings_only) {
                    if (key == "settings")
                        value.as_object().for_each_member(parse_settings);
                } else {
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
                            PackageDB::the().add(filename, key, value.as_object());
                        });
                    } else if (key == "settings") {
                        // TODO: shall settings are allowed in the second round?
                        //value.as_object().for_each_member(parse_settings);
                    } else if (key == "image") {
                        // TODO: Fixme
                    } else {
                        fprintf(stderr, "Unknown key %s found in JSON file.\n", key.characters());
                    }
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
        if (cmd == PrimaryCommand::None || cmd == PrimaryCommand::Build) {
            fprintf(stderr, "  Build:\n");
        fprintf(stderr, "    meta build <toolchain>\n");
        fprintf(stderr, "    meta build <application>\n");
        fprintf(stderr, "    meta build <image>\n");
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
    load_meta_json_files(files, true);

    if (cmd == PrimaryCommand::Config) {
        switch (config_subcmd) {
        case ConfigSubCommand::Set: {
            break;
        }
        case ConfigSubCommand::List: {
            settingsProvider.list();
            break;
        }
        case ConfigSubCommand::Get: {
            String parameter { argv[3], strlen(argv[3]) };
            String value;
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

    load_meta_json_files(files);

    //    Toolchain& toolchain = Toolchain::the();
    //    if (!toolchain.check_native_apps()) {
    //        fprintf(stderr, "Some native apps missing!\n");
    //        return -1;
    //    }

    if (cmd == PrimaryCommand::Build) {
        fprintf(stderr, "Build!\n");
    }

    statistics();

    // GeneratorPluginsLoader::the().Initialize(); // Find all loadable plugins and initialize them
    // GeneratorPluginsLoader::the().Generate(); // Generate everything

    return 0;
}
