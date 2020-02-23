#include "Toolchain.h"
#include "SettingsProvider.h"
#include "StringUtils.h"
#include <AK/FileSystemPath.h>

Toolchain::Toolchain(const String& filename, JsonObject json_obj)
{
    m_filename = filename;
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
                                tool_configuration.flags = replace_variables(value.as_string(), "root", SettingsProvider::the().get_string("root").value_or(""));
                            } else if (value.is_array()) {
                            } else if (value.is_array()) {
                                auto values = value.as_array().values();
                                StringBuilder builder;
                                builder.append(tool_configuration.flags);
                                for (auto& value : values) {
                                    builder.append(" ");
                                    builder.append(replace_variables(value.as_string(), "root", SettingsProvider::the().get_string("root").value_or("")));
                                }
                                tool_configuration.flags = builder.build();
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
            insert_tool(m_target_tools, value.as_object(), filename);
            return;
        }
        if (key == "build_tools") {
            insert_tool(m_build_tools, value.as_object(), filename);
            return;
        }
        if (key == "host_tools") {
            insert_tool(m_host_tools, value.as_object(), filename);
            return;
        }
        if (key == "build_machine_build_targets") {
            if (value.is_array()) {
                auto values = value.as_array().values();
                for (auto value : values) {
                    m_build_machine_build_targets.append(value.as_string());
                }
            } else if (value.is_string()) {
                m_build_machine_build_targets.append(value.as_string());
            }
            return;
        }
        if (key == "build_machine_inject_dependencies") {
            if (value.is_object()) {
                value.as_object().for_each_member([&](auto& key, auto& value) {
                    ASSERT(value.is_string());
                    m_build_machine_inject_dependencies.set(key, value.as_string());
                });
            }
            return;
        }
        fprintf(stderr, "Unknown toolchain key found: %s\n", key.characters());
    });
}

Toolchain::~Toolchain()
{
}

void Toolchain::insert_tool(HashMap<String, Tool>& map, JsonObject tool_data, const String& filename)
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
                auto filepath = FileSystemPath(filename);

                if (value.is_string()) {
                    auto str = replace_variables(value.as_string(), "root", SettingsProvider::the().get_string("root").value_or(""));
                    str = replace_variables(str, "current_dir", filepath.dirname());
                    tool.flags = replace_variables(str, "host_sysroot", "${CMAKE_SYSROOT}");
                } else if (value.is_array()) {
                    auto values = value.as_array().values();
                    StringBuilder builder;
                    builder.append(tool.flags);
                    for (auto& value : values) {
                        builder.append(" ");

                        auto str = replace_variables(value.as_string(), "root", SettingsProvider::the().get_string("root").value_or(""));
                        str = replace_variables(str, "current_dir", filepath.dirname());
                        builder.append(replace_variables(str, "host_sysroot", "${CMAKE_SYSROOT}"));
                    }
                    tool.flags = builder.build();
                }
                return;
            }
            if (key == "run_as_su") {
                if (value.is_bool()) {
                    tool.run_as_su = value.as_bool();
                }
                return;
            }
            if (key == "add_as_target") {
                if (value.is_bool()) {
                    tool.add_as_target = value.as_bool();
                }
                return;
            }
        });
        map.set(tool_name, tool);
    });
}
