#pragma once

#include <LibCore/CFile.h>
#include <LibCore/CObject.h>

class Settings : public CObject {
    C_OBJECT(Settings)
public:
    static Settings& the();

    bool load();
    bool get(StringView parameter, String* value);

    ~Settings();

private:
    Settings();

    CFile& find_file(String file_name);

    String m_build_directory;
    String m_toolchain;
    String m_build_configuration;
    String m_build_root;
};
