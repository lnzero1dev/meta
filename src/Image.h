#pragma once

#include <AK/JsonObject.h>
#include <AK/JsonValue.h>
#include <AK/String.h>

enum class InstallDir : uint8_t {
    Undefined,
    BinDir,
    SbinDir,
    LibexecDir,
    SysconfDir,
    SharedstateDir,
    LocalstateDir,
    IncludeDir,
    DatarootDir,
    DataDir,
    InfoDir,
    LocaleDir,
    ManDir,
};

namespace AK {
template<>
struct Traits<InstallDir> : public GenericTraits<InstallDir> {
    static constexpr bool is_trivial() { return true; }
    static unsigned hash(InstallDir i) { return int_hash((int)i); }
    static void dump(InstallDir i) { kprintf("%d", (int)i); }
};
}

class Image {

public:
    Image(const String&, const String&, JsonObject);
    ~Image();

    const String& filename() const { return m_filename; }
    const String& name() const { return m_name; }
    const Vector<String>& install() const { return m_install; }
    bool install_all() const { return m_install_all; }
    const String& build_tool() { return m_build_tool; }
    const String& run_tool() { return m_run_tool; }

private:
    String m_filename;
    String m_name;
    Vector<String> m_install;
    bool m_install_all = false;

    String m_build_tool;
    String m_run_tool;
    HashMap<InstallDir, String> imageInstallDirs;
    void set_default_install_dirs();
};
