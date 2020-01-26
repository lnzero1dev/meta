#include "Package.h"
#include <AK/JsonObject.h>
#include <AK/JsonValue.h>

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

Package::Package(JsonObject json_obj)
{

    json_obj.for_each_member([&](auto& key, auto& value) {
        //fprintf(stderr, "Package Key: %s\n", key.characters());
        if (key == "type") {
            fprintf(stderr, "type_start\n");
            if (((String)value.as_string()).matches("library"))
                m_type = PackageType::Library;
            else if (((String)value.as_string()).matches("executable"))
                m_type = PackageType::Executable;
            else if (((String)value.as_string()).matches("collection"))
                m_type = PackageType::Collection;
            else
                m_consistent = false;
            fprintf(stderr, "type_end\n");
            return;
        }
        if (key == "version") {
            fprintf(stderr, "version_start\n");
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
            fprintf(stderr, "version_end\n");
            return;
        }
        if (key == "toolchain") {
            fprintf(stderr, "toolchain_start\n");
            // get steps and options...
            if (value.as_object().has("steps")) {
                auto values = value.as_object().get("steps").as_array().values();
                for (auto& value : values) {
                    m_toolchain_steps.append(value.as_string());
                }
            }
            fprintf(stderr, "toolchain_end\n");
            return;
        }
        if (key == "dependency_linkage") {
            m_dependency_linkage = string_to_linkage_type(value.as_string());
            if (m_dependency_linkage == LinkageType::Unknown)
                m_consistent = false;

            return;
        }
        if (key == "dependency") {
            fprintf(stderr, "dependency_start\n");
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
            fprintf(stderr, "dependency_end\n");
            return;
        }
        if (key == "source") {
            fprintf(stderr, "source_start\n");
            auto values = value.as_array().values();
            for (auto& value : values) {
                m_sources.append(value.as_string());
            }
            // do the globbing from file's directory... which is... not handed over yet.
            fprintf(stderr, "source_end\n");
            return;
        }
        if (key == "include") {
            fprintf(stderr, "include_start\n");
            auto includes = value.as_array().values();
            // do the globbing from file's directory... which is... not handed over yet.
            fprintf(stderr, "include_end\n");
            return;
        }
    });
}

Package::~Package()
{
}

