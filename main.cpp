#include "FileProvider.h"
#include "Settings.h"
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

    if (cmd == PrimaryCommand::Build) {
        fprintf(stderr, "Build!\n");

        String root;
        if (settings.get("root", &root)) {
            fprintf(stderr, "Searching for files in: %s\n", root.characters());
            Vector<String> files = FileProvider::the().glob_all_meta_json_files(root);
            for (auto& file : files) {
                fprintf(stderr, "File: %s\n", file.characters());
            }
        } else {
            fprintf(stderr, "Root directory is missing!\n");
            return -1;
        }
    }

    //    Toolchain& toolchain = Toolchain::the();
    //    if (!toolchain.find_and_load_files()) {
    //        fprintf(stderr, "Failed loading settings!\n");
    //        return -1;
    //    }

    // GeneratorPluginsLoader::the().Initialize(); // Find all loadable plugins and initialize them
    // GeneratorPluginsLoader::the().Generate(); // Generate everything

    return 0;
}
