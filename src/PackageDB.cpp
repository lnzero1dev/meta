#include "PackageDB.h"

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

Package* PackageDB::find_package_that_provides(const String& executable)
{
    Package* ret { nullptr };
    for_each_entry([&](auto& name, auto& package) {
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

DataBase<Package>& package_db_for_machine(MachineType machine)
{
    ASSERT(machine != MachineType::Undefined);

    if (machine == MachineType::Build)
        return BuildPackageDB::the();
    else if (machine == MachineType::Host)
        return HostPackageDB::the();
    else if (machine == MachineType::Target)
        return TargetPackageDB::the();

    ASSERT_NOT_REACHED();
}

bool add_package(const String& name, const String& filename, const JsonObject& json_obj)
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

            package_db_for_machine(machine).add(name, filename, machine, merged_json_obj);
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
            package_db_for_machine(machine).add(name, filename, machine, json_obj);
        }
    }

    return ret;
}
