#include "Settings.h"
#include <AK/JsonObject.h>
#include <AK/JsonValue.h>
#include <FileProvider.h>
#include <limits.h>
#include <stdlib.h>
#include <sys/stat.h>

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
    auto filename = FileProvider::the().find_in_working_directory_and_parents(s_settings_file_name);

    auto file = CFile::construct();
    file->set_filename(filename);

    if (!file->filename().is_empty() && file->exists()) {
        m_settings_filename = filename;

        /* load json file */
        if (!file->open(CIODevice::ReadOnly)) {
            fprintf(stderr, "Couldn't open %s for reading: %s\n", file->filename().characters(), file->error_string());
            return false;
        }

        auto file_contents = file->read_all();
        auto json = JsonValue::from_string(file_contents);

        if (json.is_object()) {
            json.as_object().for_each_member([&](auto& key, auto& value) {
                if (key == "toolchain") {
                    m_toolchain = value.as_string();
                } else if (key == "build_configuration") {
                    m_build_configuration = value.as_string();
                } else if (key == "root") {
                    m_root = value.as_string();
                } else if (key == "build_directory") {
                    m_build_directory = value.as_string();
                } else {
                    fprintf(stderr, "Unknown key in JSON file: %s, %s\n", key.characters(), file->filename().characters());
                }
            });

        } else if (json.is_array()) {
            fprintf(stderr, "JSON file malformed: %s\n", file->filename().characters());
            return false;
        }

        return update_paths();
    }
    return false;
}

bool update_if_relative(String* path, String base)
{
    if (!path->starts_with("/")) {
        StringBuilder builder;
        builder.append(base);
        builder.append("/");
        builder.append(*path);

        String path2 = builder.build().characters();

        char buf[PATH_MAX];
        char* res = realpath(path2.characters(), buf);
        if (res) {
            *path = buf;
            return true;
        } else {
            // path its not existing, create it!
            // FIXME: This likely needs mkdir_p!
            int rc = mkdir(path2.characters(), 0755);
            if (rc < 0) {
                perror("mkdir");
                return false;
            }
            // after creating the path, we have to call realpath again to fully resolve the path
            char* res = realpath(path2.characters(), buf);
            if (res) {
                *path = buf;
                return true;
            }
            return false;
        }
    }
    return true;
}

bool Settings::update_paths()
{
    // first, update build root
    // if relative path, append path of settings file
    FileSystemPath path { m_settings_filename };
    String settings_file_dir = path.dirname();
    bool ret = true;
    ret &= update_if_relative(&m_root, settings_file_dir);
    ret &= update_if_relative(&m_build_directory, settings_file_dir);
    return ret;
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
    if (parameter == "root") {
        *value = m_root;
        return true;
    }

    if (parameter == "build_directory") {
        *value = m_build_directory;
        return true;
    }

    if (parameter == "filename") {
        *value = m_settings_filename;
    }

    fprintf(stderr, "Unknown config parameter: %s\n", parameter.characters_without_null_termination());
    return false;
}

void Settings::list()
{
    fprintf(stdout, "filename: %s\n", m_settings_filename.characters());
    fprintf(stdout, "root: %s\n", m_root.characters());
    fprintf(stdout, "toolchain: %s\n", m_toolchain.characters());
    fprintf(stdout, "build_directory: %s\n", m_build_directory.characters());
    fprintf(stdout, "build_configuration: %s\n", m_build_configuration.characters());
}
