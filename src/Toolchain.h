#pragma once

#include <AK/HashMap.h>
#include <AK/JsonObject.h>

struct ToolConfiguration {
    String flags;
};

struct Tool {
    String executable;
    String flags;
    bool run_as_su = false;
    bool add_as_target = false;
    bool reset_toolchain_flags = false;
    HashMap<String, String> execution_result_definitions;
};

class Toolchain {

public:
    Toolchain(const String&, const String&, JsonObject);
    ~Toolchain();
    bool add_file_tool_mapping(String file_extension, String tool);
    const String filename() const { return m_filename; }
    const HashMap<String, String>& file_tool_mapping() const { return m_file_tool_mapping; }
    const HashMap<String, HashMap<String, ToolConfiguration>>& configuration() const { return m_configuration; }
    const HashMap<String, Tool>& target_tools() const { return m_target_tools; }
    const HashMap<String, Tool>& host_tools() const { return m_host_tools; }
    const HashMap<String, Tool>& build_tools() const { return m_build_tools; }

    const Vector<String>& build_machine_build_targets() const { return m_build_machine_build_targets; }

    static void insert_tool(HashMap<String, Tool>&, JsonObject, const String&);

private:
    String m_name;
    String m_filename;
    HashMap<String, String> m_file_tool_mapping;
    HashMap<String, HashMap<String, ToolConfiguration>> m_configuration;
    HashMap<String, Tool> m_target_tools;
    HashMap<String, Tool> m_build_tools;
    HashMap<String, Tool> m_host_tools;
    Vector<String> m_build_machine_build_targets;
};
