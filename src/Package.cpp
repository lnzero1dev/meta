#include "Package.h"
#include "FileProvider.h"
#include "SettingsProvider.h"
#include "StringUtils.h"

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

static bool is_glob(const String& s)
{
    for (size_t i = 0; i < s.length(); i++) {
        char c = s.characters()[i];
        if (c == '*' || c == '?')
            return true;
    }
    return false;
}

static const String get_max_path_without_glob(const String& s)
{
    auto parts = s.split_view('/');
    StringBuilder builder;
    for (auto& part : parts) {
        if (is_glob(part))
            break;
        builder.append("/");
        builder.append(part);
    }
    return builder.build();
}

Package::Package(String filename, String name, JsonObject json_obj)
{
    FileSystemPath path { filename };
    m_directory = path.dirname();
    m_filename = filename;
    m_name = name;

    m_is_native = filename.contains("_native.m.json");

    json_obj.for_each_member([&](auto& key, auto& value) {
        if (key == "type") {
            if (value.as_string().matches("library"))
                m_type = PackageType::Library;
            else if (value.as_string().matches("executable"))
                m_type = PackageType::Executable;
            else if (value.as_string().matches("collection"))
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
        if (key == "provides") {
            if (value.is_object()) {
                value.as_object().for_each_member([&](auto& key, auto& value) {
                    Vector<String> values;
                    value.as_array().for_each([&](auto& value) {
                        values.append(value.as_string());
                    });

                    PackageType type = PackageType::Unknown;
                    if (key.matches("library"))
                        type = PackageType::Library;
                    else if (key.matches("executable"))
                        type = PackageType::Executable;
                    else if (key.matches("collection"))
                        type = PackageType::Collection;

                    m_provides.set(type, values);
                });
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
                if (potentially_contains_variable(source)) {
                    source = replace_variables(source, "root", SettingsProvider::the().get_string("root").value_or(""));
                }
                if (is_glob(source)) {

                    // get the last path element, before the glob sign occurs
                    search_dir = get_max_path_without_glob(source);
                    if (search_dir.is_empty()) {
                        search_dir = SettingsProvider::the().get_string("root").value_or("");
                    }

                    auto files = FileProvider::the().recursive_glob(source, search_dir);
                    for (auto& file : files) {
                        m_sources.append(file);
                    }
                } else
                    m_sources.append(value.as_string());
            }
#ifdef META_DEBUG
            for (auto& source : m_sources) {
                fprintf(stdout, "source: %s\n", source.characters());
            }
#endif
            return;
        }
        if (key == "include") {
            auto values = value.as_array().values();
            for (auto& value : values) {
                String include = value.as_string();
                String search_dir = m_directory;
                if (potentially_contains_variable(include)) {
                    include = replace_variables(include, "root", SettingsProvider::the().get_string("root").value_or(""));
                }
                if (is_glob(include)) {
                    // get the last path element, before the glob sign occurs
                    search_dir = get_max_path_without_glob(include);
                    if (search_dir.is_empty()) {
                        search_dir = SettingsProvider::the().get_string("root").value_or("");
                    }

                } else
                    m_includes.append(value.as_string());
            }
#ifdef META_DEBUG
            for (auto& include : m_includes) {
                fprintf(stdout, "include: %s\n", include.characters());
            }
#endif
            return;
        }
    });
}

Package::~Package()
{
}
