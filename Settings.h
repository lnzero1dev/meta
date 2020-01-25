#pragma once

#include <LibCore/CFile.h>
#include <LibCore/CObject.h>

static constexpr const char* s_settings_file_name { "settings.m.json" };

class Settings : public CObject {
    C_OBJECT(Settings)

public:
    static Settings& the();
    ~Settings();

    bool load();
    bool get(StringView parameter, String* value);
    void list();

private:
    Settings();

    CFile& find_file(String file_name);

    String m_settings_filename;
    String m_build_directory;
    String m_toolchain;
    String m_build_configuration;
    String m_root;

    bool update_paths();
};
