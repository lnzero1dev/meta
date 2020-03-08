#include "Package.h"
#include "FileProvider.h"
#include "SettingsProvider.h"
#include "StringUtils.h"

Deployment::Deployment(const String& type)
{
    if (type == "target")
        m_type = DeploymentType::Target;
    else if (type == "file")
        m_type = DeploymentType::File;
    else if (type == "directory")
        m_type = DeploymentType::Directory;
    else if (type == "program")
        m_type = DeploymentType::Program;
    else
        m_type = DeploymentType::Undefined;
}

Deployment::Deployment(DeploymentType type)
    : m_type(type)
{
}

Deployment::~Deployment() {}

void Deployment::set_name(const String& name)
{
    m_name = name;
}

void Deployment::set_rename(const String& rename)
{
    m_rename = rename;
}

void Deployment::set_dest(const String& dest)
{
    m_dest = dest;
}

void Deployment::set_pattern(const String& pattern)
{
    m_pattern = pattern;
}

void Deployment::set_source(const String& source)
{
    m_source = source;
}

void Deployment::set_permission(const DeploymentPermission& permission)
{
    m_permission = permission;
}

LinkageType string_to_linkage_type(String type)
{
    if (type.matches("static"))
        return LinkageType::Static;
    else if (type.matches("dynamic"))
        return LinkageType::Dynamic;
    else if (type.matches("direct"))
        return LinkageType::Direct;
    else if (type.matches("header_only"))
        return LinkageType::HeaderOnly;
    else
        return LinkageType::Undefined;
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

DeploymentPermission Package::parse_permission(const String& permission)
{
    DeploymentPermission res;
    for (size_t i = 0; i < permission.length(); i++) {
        char c = permission[i];
        int n = c - '0';
        ((RWXPermission*)&res)[i] = (RWXPermission)n;
    }
    return res;
}

Package::Package(const String& filename, const String& name, MachineType machine, const JsonObject& json_obj)
{
    FileSystemPath path { filename };
    m_directory = path.dirname();
    m_filename = filename;
    m_name = name;
    m_machine = machine;

    json_obj.for_each_member([&](auto& key, auto& value) {
        if (key == "type") {
            if (value.as_string().matches("library"))
                m_type = PackageType::Library;
            else if (value.as_string().matches("executable")) {
                m_type = PackageType::Executable;
            } else if (value.as_string().matches("collection")) {
                m_type = PackageType::Collection;
            } else if (value.as_string().matches("deployment")) {
                m_type = PackageType::Deployment;
            } else if (value.as_string().matches("script")) {
                m_type = PackageType::Script;
            } else {
                m_consistent = false;
            }
            return;
        }
        if (key == "version") {
            auto parts = value.as_string().split('.');
            if (parts.size() == 1)
                m_version.other = value.as_string();
            else {
                bool ok1 = true, ok2 = true, ok3 = true;
                m_version.major = parts[0].to_int(ok1);
                if (parts.size() > 1)
                    m_version.minor = parts[1].to_int(ok2);
                if (parts.size() > 2)
                    m_version.bugfix = parts[2].to_int(ok3);

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

                    PackageType type = PackageType::Undefined;
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
            if (value.is_object()) {
                value.as_object().for_each_member([&](auto& key, auto& value) {
                    if (key == "steps") {
                        if (value.is_array()) {
                            auto values = value.as_array().values();
                            for (auto& step_value : values) {
                                m_toolchain_steps.append(step_value.as_string());
                            }
                        }
                        return;
                    }
                    if (key == "options") {
                        if (value.is_object()) {
                            value.as_object().for_each_member([&](auto& key, auto& value) {
                                m_toolchain_options.set(key, value.as_object());
                            });
                        }
                        return;
                    }
                });
                return;
            }
            return;
        }
        if (key == "dependency_linkage") {
            m_dependency_linkage = string_to_linkage_type(value.as_string());
            if (m_dependency_linkage == LinkageType::Undefined)
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
#ifdef DEBUG_META
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
                    // Fixme: Globbing is broken/incomplete here, do it like above with sources, check performance... ^^
                } else
                    m_includes.append(value.as_string());
            }
#ifdef DEBUG_META
            for (auto& include : m_includes) {
                fprintf(stdout, "include: %s\n", include.characters());
            }
#endif
            return;
        }
        if (key == "deploy") {
            auto values = value.as_array().values();

#ifdef DEBUG_META
            fprintf(stderr, "Found %i install items for %s\n", values.size(), m_name.characters());
#endif

            for (auto& value : values) {
                // value is an object
                auto& obj = value.as_object();
                if (obj.has("type")) {
                    auto type = obj.get("type").as_string();
#ifdef DEBUG_META
                    fprintf(stderr, " * Type: %s\n", type.characters());
#endif
                    if (type == "target") {
                        auto depl = adopt(*new Deployment(DeploymentType::Target));
                        if (obj.has("name"))
                            depl->set_name(obj.get("name").as_string());
                        if (obj.has("dest"))
                            depl->set_dest(obj.get("dest").as_string());
                        if (obj.has("permission"))
                            depl->set_permission(parse_permission(obj.get("permission").as_string()));

                        m_deploy.append(depl);
                    } else if (type == "file" || type == "program") {
#ifdef DEBUG_META
                        fprintf(stderr, "Install file: %s\n", obj.get("source").as_string().characters());
#endif
                        auto depl = adopt(*new Deployment(type == "file" ? DeploymentType::File : DeploymentType::Program));
                        if (obj.has("source"))
                            depl->set_source(obj.get("source").as_string());
                        if (obj.has("dest"))
                            depl->set_dest(obj.get("dest").as_string());
                        if (obj.has("rename"))
                            depl->set_rename(obj.get("rename").as_string());
                        if (obj.has("permission"))
                            depl->set_permission(parse_permission(obj.get("permission").as_string()));
                        m_deploy.append(depl);
                    } else if (type == "directory") {
                        auto depl = adopt(*new Deployment(DeploymentType::Directory));
                        if (obj.has("source"))
                            depl->set_source(obj.get("source").as_string());
                        if (obj.has("dest"))
                            depl->set_dest(obj.get("dest").as_string());
                        if (obj.has("pattern"))
                            depl->set_pattern(obj.get("pattern").as_string());
                        if (obj.has("permission"))
                            depl->set_permission(parse_permission(obj.get("permission").as_string()));
                        m_deploy.append(depl);
                    } else if (type == "object") {
                        auto depl = adopt(*new Deployment(DeploymentType::Object));
                        if (obj.has("source"))
                            depl->set_source(obj.get("source").as_string());
                        if (obj.has("dest"))
                            depl->set_dest(obj.get("dest").as_string());
                        if (obj.has("name"))
                            depl->set_name(obj.get("name").as_string());
                        if (obj.has("permission"))
                            depl->set_permission(parse_permission(obj.get("permission").as_string()));
                        m_deploy.append(depl);
                    } else {
                        fprintf(stderr, "Unknown deployment type %s is specified in %s\n", obj.get("type").as_string().characters(), m_filename.characters());
                    }
                } else {
                    fprintf(stderr, "Unknown deployment in %s as no type is specified.\n", m_filename.characters());
                }
            }
            return;
        }
        if (key == "target_tools") {
            Toolchain::insert_tool(m_target_tools, value.as_object(), filename);
            return;
        }
        if (key == "build_tools") {
            Toolchain::insert_tool(m_build_tools, value.as_object(), filename);
            return;
        }
        if (key == "host_tools") {
            Toolchain::insert_tool(m_host_tools, value.as_object(), filename);
            return;
        }
        if (key == "run_generators") {
            if (value.is_object()) {
                value.as_object().for_each_member([&](auto& key, auto& value) {
                    Vector<InputOutputTuple> input_output_tuples;
                    auto& values = value.as_array().values();
                    for (auto& value : values) {
                        InputOutputTuple tuple;
                        value.as_object().for_each_member([&](auto& key, auto& value) {
                            if (key == "input") {
                                if (value.is_string()) {
                                    tuple.input = value.as_string();
                                } else {
                                    // error
                                }
                                return;
                            }
                            if (key == "output") {
                                if (value.is_string()) {
                                    tuple.output = value.as_string();
                                } else {
                                    // error
                                }
                                return;
                            }
                            if (key == "flags") {
                                if (value.is_string()) {
                                    tuple.flags = value.as_string();
                                } else {
                                    // error
                                }
                                return;
                            }
                            fprintf(stderr, "Unknown value within run_generators in %s.\n", m_filename.characters());
                        });
                        input_output_tuples.append(tuple);
                    }

                    m_run_generators.set(key, { input_output_tuples });
                });
            } else {
                fprintf(stderr, "Unknown value for run_generators in %s.\n", m_filename.characters());
            }
            return;
        }
    });
}

Package::~Package()
{
}
LinkageType Package::get_dependency_linkage(LinkageType type) const
{
    if (type == LinkageType::Inherit) {
        return m_dependency_linkage;
    }
    return type;
}
