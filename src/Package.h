#pragma once

#include <AK/JsonObject.h>
#include <AK/Traits.h>

enum class LinkageType : uint8_t {
    Inherit = 0,
    Static,
    Dynamic,
    NoLib,

    Unknown = 0xFF
};

enum class PackageType : uint8_t {
    Library = 0, // Exactly one library
    Executable,  // Exactly one executable
    Collection,  // Collection of 1..n libraries/executables

    Unknown = 0xFF
};

namespace AK {
template<>
struct Traits<PackageType> : public GenericTraits<PackageType> {
    static constexpr bool is_trivial() { return true; }
    static unsigned hash(PackageType i) { return int_hash((int)i); }
    static void dump(PackageType i) { kprintf("%d", (int)i); }
};
}

struct PackageVersion {
    int major;
    int minor;
    int bugfix;
    String other;
};

class Package {

public:
    Package(String, String, JsonObject);
    ~Package();

    const Vector<String>& toolchain_steps() const { return m_toolchain_steps; }
    const Vector<String>& sources() const { return m_sources; }
    const Vector<String>& includes() const { return m_includes; }
    const String& name() const { return m_name; }
    PackageType type() const { return m_type; }
    const String& filename() const { return m_filename; }

    bool is_consistent() const { return m_consistent; }
    bool is_native() const { return m_is_native; }

    const HashMap<String, LinkageType>& dependencies() const { return m_dependencies; }
    const HashMap<PackageType, Vector<String>>& provides() const { return m_provides; }

private:
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

    bool m_is_native { false };
};
