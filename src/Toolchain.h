#pragma once

#include <AK/HashMap.h>
#include <AK/JsonObject.h>

struct ToolConfiguration {
    String flags;
};

struct Tool {
    String executable;
    String flags;
};

class Toolchain {

public:
    Toolchain(JsonObject json_obj);
    ~Toolchain();

    bool add_target_tool(String name, JsonObject tool);
    bool add_native_tool(String name, JsonObject tool);
    bool add_file_tool_mapping(String file_extension, String tool);

    const HashMap<String, String>& file_tool_mapping() const { return m_file_tool_mapping; }
    const HashMap<String, HashMap<String, ToolConfiguration>>& configuration() const { return m_configuration; }
    const HashMap<String, Tool>& target_tools() const { return m_target_tools; }
    const HashMap<String, Tool>& native_tools() const { return m_native_tools; }

private:
    void insert_tool(HashMap<String, Tool>&, JsonObject);

    HashMap<String, String> m_file_tool_mapping;
    HashMap<String, HashMap<String, ToolConfiguration>> m_configuration;
    HashMap<String, Tool> m_target_tools;
    HashMap<String, Tool> m_native_tools;
};
