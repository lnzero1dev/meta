#include "PackageDB.h"

PackageDB::PackageDB()
{
}

PackageDB::~PackageDB()
{
}

PackageDB& PackageDB::the()
{
    static PackageDB* s_the;
    if (!s_the)
        s_the = &PackageDB::construct().leak_ref();
    return *s_the;
}

MachineType machine_to_machine_type(const String& machine)
{
    MachineType m = MachineType::Undefined;

    if (machine == "target") {
        m = MachineType::Target;
    } else if (machine == "build") {
        m = MachineType::Build;
    } else if (machine == "host") {
        m = MachineType::Host;
    } else {
        fprintf(stderr, "Unknown machine: %s\n", machine.characters());
    }

    return m;
}

bool PackageDB::add(const String& filename, const String& name, const JsonObject& json_obj)
{
    bool ret = true;
    auto machine_json = json_obj.get("machine");
    Vector<MachineType> machines;

    if (machine_json.is_object()) {
        machine_json.as_object().for_each_member([&](auto& key, auto& value) {
            auto machine = machine_to_machine_type(key);
            auto merged_json_obj = json_obj;
            value.as_object().for_each_member([&](auto& key, auto& value) {
                merged_json_obj.set(key, value);
            });

            Package p { filename, name, machine, merged_json_obj };

            auto& packages = (machine == MachineType::Build) ? m_packages_build : ((machine == MachineType::Host) ? m_packages_host : m_packages_target);
            if (packages.find(name) == packages.end())
                packages.set(name, move(p));
            else
                ret = false;
        });

    } else {
        if (machine_json.is_array()) {
            for (auto machine : machine_json.as_array().values()) {
                machines.append(machine_to_machine_type(machine.as_string()));
            }
        } else {
            machines.append(machine_to_machine_type(machine_json.as_string_or("target")));
        }

        // insert package into each machine specific database
        for (auto machine : machines) {
            Package p { filename, name, machine, json_obj };
            auto& packages = (machine == MachineType::Build) ? m_packages_build : ((machine == MachineType::Host) ? m_packages_host : m_packages_target);
            if (packages.find(name) == packages.end())
                packages.set(name, move(p));
            else
                ret = false;
        }
    }

    return ret;
}

Package* PackageDB::get(MachineType machine, StringView name)
{
    auto& packages = (machine == MachineType::Build) ? m_packages_build : ((machine == MachineType::Host) ? m_packages_host : m_packages_target);

    auto it = packages.find(name);
    if (it != packages.end())
        return &(*it).value;

    return nullptr;
}

Package* PackageDB::find_host_package_that_provides(const String& executable)
{
    Package* ret { nullptr };
    for_each_host_package([&](auto& name, auto& package) {
        if (package.type() == PackageType::Executable && name == executable) {
            ret = &package;
            return IterationDecision::Break;
        }

        for (auto& provides : package.provides()) {
            for (auto& provide_value : provides.value) {
                if (provide_value == executable) {
                    ret = &package;
                    return IterationDecision::Break;
                }
            }
        }

        return IterationDecision::Continue;
    });
    return ret;
}
