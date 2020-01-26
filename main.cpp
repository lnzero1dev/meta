#include "FileProvider.h"
#include "PackageDB.h"
#include "Settings.h"
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

void load_meta_json_files(Vector<String> files)
{
    auto file = CFile::construct();
    for (auto& filename : files) {
        file->set_filename(filename);
        if (file->filename().is_empty() || !file->exists())
            continue;

        /* load json file */
        if (!file->open(CIODevice::ReadOnly)) {
            fprintf(stderr, "Couldn't open %s for reading: %s\n", file->filename().characters(), file->error_string());
            continue;
        }

        auto file_contents = file->read_all();
        auto json = JsonValue::from_string(file_contents);

        if (json.is_object()) {
            json.as_object().for_each_member([&](auto& key, auto& value) {
                if (key == "toolchain") {
                    value.as_object().for_each_member([&](auto& key2, auto& value2) {
                        fprintf(stderr, "Adding toolchain: %s\n", key2.characters());
                        ToolchainDB::the().add(key2, value2.as_object());
                    });
                } else if (key == "package") {
                    value.as_object().for_each_member([&](auto& key2, auto& value2) {
                        fprintf(stderr, "Adding package: %s\n", key2.characters());
                        PackageDB::the().add(key2, value2.as_object());
                    });
                } else if (key == "settings") {
                    // do nothing ...

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

    // needs always to be done
    Settings& settings = Settings::the();
    if (!settings.load()) {
        fprintf(stderr, "Failed loading settings!\n");
        return -1;
    }

    if (cmd == PrimaryCommand::Config) {
        switch (config_subcmd) {
        case ConfigSubCommand::Set: {
            break;
        }
        case ConfigSubCommand::List: {
            settings.list();
            break;
        }
        case ConfigSubCommand::Get: {
            String parameter { argv[3], strlen(argv[3]) };
            String value;
            if (settings.get(parameter, &value)) {
                fprintf(stdout, "%s: %s\n", parameter.characters(), value.characters());
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

    String root;
    Vector<String> files;
    if (settings.get("root", &root)) {
        fprintf(stderr, "Searching for files in: %s\n", root.characters());
        files = FileProvider::the().glob_all_meta_json_files(root);
        for (auto& file : files) {
            fprintf(stderr, "File: %s\n", file.characters());
        }
    } else {
        fprintf(stderr, "Root directory is missing!\n");
        return -1;
    }

    load_meta_json_files(files);

    PackageDB::the().for_each_package([&](auto& name, auto& package) {
        fprintf(stderr, "Package %s:\n", name.characters());
        fprintf(stderr, "Toolchain Steps: %i\n", package.toolchain_steps().size());
        return IterationDecision::Continue;
    });

    //    Toolchain& toolchain = Toolchain::the();
    //    if (!toolchain.find_and_load_files()) {
    //        fprintf(stderr, "Failed loading settings!\n");
    //        return -1;
    //    }

    if (cmd == PrimaryCommand::Build) {
        fprintf(stderr, "Build!\n");
    }

    // GeneratorPluginsLoader::the().Initialize(); // Find all loadable plugins and initialize them
    // GeneratorPluginsLoader::the().Generate(); // Generate everything

    return 0;
}
