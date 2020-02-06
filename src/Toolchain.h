#pragma once

#include <AK/HashMap.h>
#include <AK/JsonObject.h>
#include <LibCore/CFile.h>
#include <LibCore/CObject.h>

struct Flags {
    String standard;
    String warning;
    String flavor;
    String optimization;
    String arch;
    String include;
    String defines;
};

struct ToolConfiguration {
    Flags flags;
};

struct Tool {
    String executable;
    Flags flags;
};

class Toolchain {

public:
    Toolchain(JsonObject json_obj);
    ~Toolchain();

    bool add_target_tool(String name, JsonObject tool);
    bool add_native_tool(String name, JsonObject tool);
    bool add_file_tool_mapping(String file_extension, String tool);

    const HashMap<String, String>& file_tool_mapping() { return m_file_tool_mapping; }
    const HashMap<String, HashMap<String, ToolConfiguration>>& configuration() { return m_configuration; }
    const HashMap<String, Tool>& target_tools() { return m_target_tools; }
    const HashMap<String, Tool>& native_tools() { return m_native_tools; }

private:
    void insert_tool(HashMap<String, Tool>&, JsonObject);

    HashMap<String, String> m_file_tool_mapping;
    HashMap<String, HashMap<String, ToolConfiguration>> m_configuration;
    HashMap<String, Tool> m_target_tools;
    HashMap<String, Tool> m_native_tools;
};
