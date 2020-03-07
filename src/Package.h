#pragma once

#include "Toolchain.h"
#include <AK/JsonObject.h>
#include <AK/Traits.h>
#include <LibCore/Object.h>
#include <string>

enum class LinkageType : uint8_t {
    Inherit = 0,
    Static,
    Dynamic,
    Direct,
    HeaderOnly,

    Unknown = 0xFF
};

enum class PackageType : uint8_t {
    Library = 0, // Exactly one library
    Executable,  // Exactly one executable
    Collection,  // Collection of 1..n libraries/executables
    Deployment,  // Just deployment for specific machine
    Script,      // Script

    Unknown = 0xFF
};

enum class MachineType : uint8_t {
    Build = 0,
    Host,
    Target
};

namespace AK {
template<>
struct Traits<PackageType> : public GenericTraits<PackageType> {
    static constexpr bool is_trivial() { return true; }
    static unsigned hash(PackageType i) { return int_hash((int)i); }
    static void dump(PackageType i) { kprintf("%d", (int)i); }
};
}

enum class DeploymentType : u8 {
    Undefined = 0,
    Target,
    File,
    Directory,
    Object,
    Program,
};

enum RWXPermission {
    Read = 1,
    Write = 2,
    Execute = 4,
    ReadWrite = 3,
    ReadExecute = 5,
    WriteExecute = 6,
    ReadWriteExecute = 7
};

struct DeploymentPermission {
    RWXPermission user;
    RWXPermission group;
    RWXPermission other;
};

class Deployment : public Core::Object {
    C_OBJECT(Deployment)
public:
    explicit Deployment(DeploymentType);
    explicit Deployment(const String&);
    ~Deployment();

    void set(const String&, const String&);
    void set(const String&, const String&, const String&);

    DeploymentType type() const { return m_type; }

    void set_name(const String&);
    void set_rename(const String&);
    void set_dest(const String&);
    void set_pattern(const String&);
    void set_source(const String&);
    void set_permission(const DeploymentPermission& permission);

    const String& name() const { return m_name; }
    const String& source() const { return m_source; }
    const String& rename() const { return m_rename; }
    const String& pattern() const { return m_pattern; }
    const String& dest() const { return m_dest; }
    const Optional<DeploymentPermission>& permission() const { return m_permission; }

private:
    DeploymentType m_type;
    String m_name;
    String m_source;
    String m_rename;
    String m_pattern;
    String m_dest;
    Optional<DeploymentPermission> m_permission;
};

struct PackageVersion {
    int major = 0;
    Optional<int> minor;
    Optional<int> bugfix;
    String other;
};

struct InputOutputTuple {
    String input;
    String output;
    String flags;
};

struct Generator {
    Vector<InputOutputTuple> input_output_tuples;
};

class Package {

public:
    Package(String, String, JsonObject);
    ~Package();

    const Vector<String>& toolchain_steps() const { return m_toolchain_steps; }
    const HashMap<String, JsonObject>& toolchain_options() const { return m_toolchain_options; }
    const Vector<String>& sources() const { return m_sources; }
    const Vector<String>& includes() const { return m_includes; }
    const String& name() const { return m_name; }
    PackageType type() const { return m_type; }
    const String& filename() const { return m_filename; }
    const String version() const
    {
        if (!m_version.other.is_empty())
            return m_version.other;
        else {
            StringBuilder builder;
            builder.appendf("%i", m_version.major);
            if (m_version.minor.has_value()) {
                builder.append(".");
                builder.appendf("%i", m_version.minor.value());
            }
            if (m_version.bugfix.has_value()) {
                builder.append(".");
                builder.appendf("%i", m_version.bugfix.value());
            }
            return builder.build();
        }
    }

    bool is_consistent() const { return m_consistent; }
    MachineType machine() const { return m_machine; }
    const String machine_name() const
    {
        if (m_machine == MachineType::Host)
            return "Host";
        if (m_machine == MachineType::Build)
            return "Build";
        if (m_machine == MachineType::Target)
            return "Target";
        return "Unknown";
    }

    const HashMap<String, LinkageType>& dependencies() const { return m_dependencies; }
    const HashMap<PackageType, Vector<String>>& provides() const { return m_provides; }
    const Vector<NonnullRefPtr<Deployment>>& deploy() const { return m_deploy; }

    LinkageType get_dependency_linkage(LinkageType) const;

    const HashMap<String, Tool>& target_tools() const { return m_target_tools; }
    const HashMap<String, Tool>& host_tools() const { return m_host_tools; }
    const HashMap<String, Tool>& build_tools() const { return m_build_tools; }

    const HashMap<String, Generator>& run_generators() const { return m_run_generators; }

    void remove_dependency(const String& name)
    {
        m_dependencies.remove(name);
    }

private:
    DeploymentPermission parse_permission(const String&);

    bool m_consistent = true;

    String m_filename;
    String m_directory;

    String m_name;
    PackageType m_type;
    PackageVersion m_version;

    // For collections, provide the ability to define which libraries and executables are contained.
    HashMap<PackageType, Vector<String>> m_provides;

    Vector<String> m_sources;
    Vector<String> m_includes;

    Vector<String> m_toolchain_steps;
    HashMap<String, JsonObject> m_toolchain_options;

    HashMap<String, LinkageType> m_dependencies;
    LinkageType m_dependency_linkage = LinkageType::Static;

    Vector<NonnullRefPtr<Deployment>> m_deploy;

    HashMap<String, Tool> m_target_tools;
    HashMap<String, Tool> m_build_tools;
    HashMap<String, Tool> m_host_tools;

    HashMap<String, Generator> m_run_generators;

    MachineType m_machine;
};
