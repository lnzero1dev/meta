#include "Settings.h"
#include <AK/JsonObject.h>
#include <AK/JsonValue.h>
#include <FileProvider.h>

Settings::Settings()
{
}

Settings::~Settings()
{
}

Settings& Settings::the()
{
    static Settings* s_the;
    if (!s_the)
        s_the = &Settings::construct().leak_ref();
    return *s_the;
}

bool Settings::load()
{
    CFile& file = FileProvider::the().find_in_working_directory_and_parents("settings.json");
    if (!file.filename().is_null() && file.exists()) {
        /* load json file */
        if (!file.open(CIODevice::ReadOnly)) {
            fprintf(stderr, "Couldn't open %s for reading: %s\n", file.filename().characters(), file.error_string());
            return false;
        }

        auto file_contents = file.read_all();
        auto json = JsonValue::from_string(file_contents);

        if (json.is_object()) {

            json.as_object().for_each_member([&](auto& key, auto& value) {
                if (key == "toolchain") {
                    m_toolchain = value.as_string();
                } else if (key == "build_configuration") {
                    m_build_configuration = value.as_string();
                } else if (key == "build_root") {
                    m_build_root = value.as_string();
                } else if (key == "build_directory") {
                    m_build_directory = value.as_string();
                } else {
                    fprintf(stderr, "Unknown key in JSON file: %s, %s\n", key.characters(), file.filename().characters());
                }
            });

        } else if (json.is_array()) {
            fprintf(stderr, "JSON file malformed: %s\n", file.filename().characters());
            return false;
        }

        return true;
    }
    return false;
}

bool Settings::get(StringView parameter, String* value)
{
    if (parameter == "toolchain") {
        *value = m_toolchain;
        return true;
    }
    if (parameter == "build_configuration") {
        *value = m_build_configuration;
        return true;
    }
    if (parameter == "build_root") {
        *value = m_build_root;
        return true;
    }

    if (parameter == "build_directory") {
        *value = m_build_directory;
        return true;
    }

    fprintf(stderr, "Unknown config parameter: %s\n", parameter.characters_without_null_termination());
    return false;
}
