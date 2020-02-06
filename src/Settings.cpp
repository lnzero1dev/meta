#include "Settings.h"
#include "FileProvider.h"
#include <AK/JsonValue.h>
#include <limits.h>
#include <stdlib.h>

Settings::Settings()
{
}

Settings::~Settings()
{
}

bool Settings::load(const String& filename, const JsonObject& settings_object)
{
    bool failed = false;
    settings_object.for_each_member([&](auto& key, auto& value) {
        if (key == "toolchain") {
            if (m_toolchain.has_value()) {
                failed = true;
                return IterationDecision::Break;
            } else
                m_toolchain = SettingsParameter { filename, value.as_string() };
        } else if (key == "build_generator_configuration") {
            if (m_build_generator_configuration.has_value()) {
                failed = true;
                return IterationDecision::Break;
            } else
                m_build_generator_configuration = SettingsParameter { filename, value.as_object() };

        } else if (key == "gendata_directory") {
            if (m_gendata_directory.has_value()) {
                failed = true;
                return IterationDecision::Break;
            } else
                m_gendata_directory = SettingsParameter { filename, value.as_string() };
        } else if (key == "build_generator") {
            if (m_build_generator.has_value()) {
                failed = true;
                return IterationDecision::Break;
            } else
                m_build_generator = SettingsParameter { filename, value.as_string() };
        } else if (key == "build_directory") {
            if (m_build_directory.has_value()) {
                failed = true;
                return IterationDecision::Break;
            } else
                m_build_directory = SettingsParameter { filename, value.as_string() };
        } else if (key == "root") {
            if (m_root.has_value()) {
                failed = true;
                return IterationDecision::Break;
            } else
                m_root = SettingsParameter { filename, value.as_string() };
        } else {
            fprintf(stderr, "Unknown key '%s' in JSON file %s\n", key.characters(), filename.characters());
        }
        return IterationDecision::Continue;
    });

    if (failed)
        return false;
    else
        return update_paths(filename);
}

bool Settings::update_paths(const String& filename)
{
    // first, update build root
    // if relative path, append path of settings file
    FileSystemPath path { filename };
    String settings_file_dir = path.dirname();
    bool ret = true;

    if (m_root.has_value()) {
        auto root = String { m_root.value().as_string() };
        ret &= FileProvider::the().update_if_relative(root, settings_file_dir);
        if (ret)
            m_root.value() = { filename, root };
    }
    if (m_build_directory.has_value()) {
        auto build_directory = String { m_build_directory.value().as_string() };
        ret &= FileProvider::the().update_if_relative(build_directory, settings_file_dir);
        if (ret)
            m_build_directory.value() = { filename, build_directory };
    }
    if (m_gendata_directory.has_value()) {
        auto gendata_directory = String { m_gendata_directory.value().as_string() };
        ret &= FileProvider::the().update_if_relative(gendata_directory, settings_file_dir);
        if (ret)
            m_gendata_directory.value() = { filename, gendata_directory };
    }
    return ret;
}

Optional<SettingsParameter> Settings::get(Badge<SettingsProvider>, const String parameter)
{
    Optional<SettingsParameter> ret;
    if (parameter == "root") {
        return m_root;
    } else if (parameter == "toolchain") {
        return m_toolchain;
    } else if (parameter == "build_generator") {
        return m_build_generator;
    } else if (parameter == "build_generator_configuration") {
        return m_build_generator_configuration;
    } else if (parameter == "build_directory") {
        return m_build_directory;
    } else if (parameter == "gendata_directory") {
        return m_gendata_directory;
    }

    fprintf(stderr, "Unknown config parameter: %s\n", parameter.characters());
    return ret;
}
