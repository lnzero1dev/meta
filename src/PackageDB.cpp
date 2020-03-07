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

bool PackageDB::add(const String& filename, const String& name, const JsonObject& json_obj)
{
    Package p { filename, name, json_obj };
    auto& packages = (p.machine() == MachineType::Build) ? m_packages_build : ((p.machine() == MachineType::Host) ? m_packages_host : m_packages_target);
    if (packages.find(name) != packages.end())
        return false;

    packages.set(name, move(p));
    return true;
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
