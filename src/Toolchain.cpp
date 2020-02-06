#include "Toolchain.h"

bool set_flags(Flags& flags, String key, String value)
{
    if (key == "standard") {
        flags.standard = value;
        return true;
    }
    if (key == "warning") {
        flags.warning = value;
        return true;
    }
    if (key == "flavor") {
        flags.flavor = value;
        return true;
    }
    if (key == "optimization") {
        flags.optimization = value;
        return true;
    }
    if (key == "arch") {
        flags.arch = value;
        return true;
    }
    if (key == "include") {
        flags.include = value;
        return true;
    }
    if (key == "defines") {
        flags.defines = value;
        return true;
    }
    return false;
}

Toolchain::Toolchain(JsonObject json_obj)
{
    json_obj.for_each_member([&](auto& key, auto& value) {
        if (key == "file_tool_mapping") {
            value.as_object().for_each_member([&](auto& key, auto& value) {
                m_file_tool_mapping.set(key, value.as_string());
            });
            return;
        }
        if (key == "configuration") {
            value.as_object().for_each_member([&](auto& key, auto& value) {
                auto& config_name = key;
                HashMap<String, ToolConfiguration> tool_configurations;
                value.as_object().for_each_member([&](auto& key, auto& value) {
                    auto& tool_name = key;
                    ToolConfiguration tool_configuration;
                    value.as_object().for_each_member([&](auto& key, auto& value) {
                        if (key == "flags") {
                            if (value.is_string()) {
                                tool_configuration.flags.flavor = value.as_string();
                            } else if (value.is_object()) {
                                value.as_object().for_each_member([&](auto& key, auto& value) {
                                    if (!set_flags(tool_configuration.flags, key, value.as_string()))
                                        fprintf(stderr, "Unknown flag configuration: %s\n", key.characters());
                                });
                            }
                            return;
                        }
                        fprintf(stderr, "Unknown tool configuration: %s\n", key.characters());
                    });
                    tool_configurations.set(tool_name, tool_configuration);
                });

                m_configuration.set(config_name, tool_configurations);
            });

            return;
        }
        if (key == "target_tools") {
            insert_tool(m_target_tools, value.as_object());
            return;
        }
        if (key == "native_tools") {
            insert_tool(m_native_tools, value.as_object());
            return;
        }
        fprintf(stderr, "Unknown toolchain key found: %s\n", key.characters());
    });
}

Toolchain::~Toolchain()
{
}

void Toolchain::insert_tool(HashMap<String, Tool>& map, JsonObject tool_data)
{
    tool_data.for_each_member([&](auto& key, auto& value) {
        auto& tool_name = key;
        Tool tool;
        value.as_object().for_each_member([&](auto& key, auto& value) {
            if (key == "executable") {
                tool.executable = value.as_string();
                return;
            }
            if (key == "flags") {
                if (value.is_string()) {
                    tool.flags.flavor = value.as_string();
                } else if (value.is_object()) {
                    value.as_object().for_each_member([&](auto& key, auto& value) {
                        if (!set_flags(tool.flags, key, value.as_string()))
                            fprintf(stderr, "Unknown flag configuration: %s\n", key.characters());
                    });
                }
                return;
            }
        });
        map.set(tool_name, tool);
    });
}
