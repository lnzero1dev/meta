#pragma once

#include "SettingsParameter.h"
#include <AK/Badge.h>
#include <AK/JsonObject.h>
#include <AK/Optional.h>
#include <LibCore/File.h>
#include <LibCore/Object.h>

class SettingsProvider;

class Settings {

public:
    Settings();
    ~Settings();
    bool load(const String& filename, const JsonObject& settings_object);

    Optional<SettingsParameter> get(Badge<SettingsProvider>, String parameter);

    void list();

private:
    Core::File& find_file(String file_name);

    Optional<SettingsParameter> m_root;
    Optional<SettingsParameter> m_toolchain;
    Optional<SettingsParameter> m_build_directory;
    Optional<SettingsParameter> m_gendata_directory;
    Optional<SettingsParameter> m_build_generator;
    Optional<SettingsParameter> m_build_generator_configuration;

    bool update_paths(const String& filename);
};
