#include "Package.h"
#include "FileProvider.h"
#include "Settings.h"
#include <AK/JsonObject.h>
#include <AK/JsonValue.h>
#include <regex>
#include <string>

LinkageType string_to_linkage_type(String type)
{
    if (type.matches("static"))
        return LinkageType::Static;
    else if (type.matches("dynamic"))
        return LinkageType::Dynamic;
    else if (type.matches("nolib"))
        return LinkageType::NoLib;
    else
        return LinkageType::Unknown;
}

static bool is_glob(const StringView& s)
{
    for (size_t i = 0; i < s.length(); i++) {
        char c = s.characters_without_null_termination()[i];
        if (c == '*' || c == '?')
            return true;
    }
    return false;
}

static bool contains_variable(const StringView& s)
{
    for (size_t i = 0; i < s.length(); i++) {
        char c = s.characters_without_null_termination()[i];
        if (c == '$') {
            return true;
        }
    }
    return false;
}

String replace_variables(const StringView& s)
{
    std::string str(s.characters_without_null_termination(), s.length());
    std::string replaced = std::regex_replace(str, std::regex("\\$\\{root\\}"), String { Settings::the().root() }.characters());
    return replaced.c_str();
}

Package::Package(String filename, JsonObject json_obj)
{
    FileSystemPath path { filename };
    m_directory = path.dirname();
    m_filename = filename;

    json_obj.for_each_member([&](auto& key, auto& value) {
        //fprintf(stderr, "Package Key: %s\n", key.characters());
        if (key == "type") {
            if (((String)value.as_string()).matches("library"))
                m_type = PackageType::Library;
            else if (((String)value.as_string()).matches("executable"))
                m_type = PackageType::Executable;
            else if (((String)value.as_string()).matches("collection"))
                m_type = PackageType::Collection;
            else
                m_consistent = false;
            return;
        }
        if (key == "version") {
            auto parts = value.as_string().split('.');
            if (parts.size() == 1)
                m_version.other = value.as_string();
            else {
                bool ok1, ok2, ok3;
                m_version.major = parts[0].to_int(ok1);
                m_version.minor = parts[1].to_int(ok2);
                if (parts.size() > 2) {
                    m_version.bugfix = parts[2].to_int(ok3);
                }
                if (!ok1 || !ok2 || !ok3) {
                    m_version.major = 0;
                    m_version.minor = 0;
                    m_version.bugfix = 0;
                    m_version.other = value.as_string();
                    fprintf(stderr, "Version cannot be parsed correctly: %s\n", value.as_string().characters());
                }
            }
            return;
        }
        if (key == "toolchain") {
            // get steps and options...
            if (value.as_object().has("steps")) {
                auto values = value.as_object().get("steps").as_array().values();
                for (auto& value : values) {
                    m_toolchain_steps.append(value.as_string());
                }
            }
            return;
        }
        if (key == "dependency_linkage") {
            m_dependency_linkage = string_to_linkage_type(value.as_string());
            if (m_dependency_linkage == LinkageType::Unknown)
                m_consistent = false;

            return;
        }
        if (key == "dependency") {
            // there are two ways to define a dependency:
            // "dependency_linkage": "static" (static is default if setting is absent)
            // "dependency": [
            //     "libxyz"
            // ],
            // Or with the linkage type given
            // "dependency": {
            //     "libxyz": "static"
            // },

            if (value.is_array()) {
                auto values = value.as_array().values();
                for (auto& value : values) {
                    m_dependencies.set(value.as_string(), LinkageType::Inherit);
                }
            } else if (value.is_object()) {
                value.as_object().for_each_member([&](auto& key, auto& value) {
                    m_dependencies.set(key, string_to_linkage_type(value.as_string()));
                });
            }
            return;
        }
        if (key == "source") {
            auto values = value.as_array().values();
            for (auto& value : values) {
                String source = value.as_string();
                String search_dir = m_directory;
                if (contains_variable(source)) {
                    source = replace_variables(source);
                    // instead of setting the place to search here directly,
                    // it would be also possible (and maybe more performant)
                    // to first search in m_directory, and if nothing is being
                    // found, search again in root.
                    search_dir = Settings::the().root();
                }
                if (is_glob(source)) {
                    auto files = FileProvider::the().recursive_glob(source, search_dir);
                    for (auto& file : files) {
                        m_sources.append(file);
                    }
                } else
                    m_sources.append(value.as_string());
            }
            return;
        }
        if (key == "include") {
            auto values = value.as_array().values();
            for (auto& value : values) {
                String include = value.as_string();
                String search_dir = m_directory;
                if (contains_variable(include)) {
                    include = replace_variables(include);
                    search_dir = Settings::the().root();
                }
                if (is_glob(include)) {
                    auto files = FileProvider::the().recursive_glob(include, search_dir);
                    for (auto& file : files)
                        m_includes.append(file);
                } else
                    m_includes.append(value.as_string());
            }
            return;
        }
    });
}

Package::~Package()
{
}

