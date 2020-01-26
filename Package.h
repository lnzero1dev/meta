#pragma once

#include <AK/JsonObject.h>
#include <LibCore/CFile.h>
#include <LibCore/CObject.h>

enum class LinkageType : uint8_t {
    Inherit = 0,
    Static,
    Dynamic,
    NoLib,

    Unknown,
};

enum class PackageType : uint8_t {
    Library = 0, // Exactly one library
    Executable,  // Exactly one executable
    Collection,  // Collection of 1..n libraries/executables
};

struct PackageVersion {
    int major;
    int minor;
    int bugfix;
    String other;
};

class Package {

public:
    Package(JsonObject);
    ~Package();

    const Vector<String>& toolchain_steps() { return m_toolchain_steps; }

private:
    bool m_consistent = true;

    PackageType m_type;
    PackageVersion m_version;

    Vector<String> m_sources;
    Vector<String> m_includes;

    Vector<String> m_toolchain_steps;
    HashMap<String, JsonObject> m_toolchain_options;

    HashMap<String, LinkageType> m_dependencies;
    LinkageType m_dependency_linkage = LinkageType::Static;
};
