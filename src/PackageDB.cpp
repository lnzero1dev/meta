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

bool PackageDB::add(String filename, String name, JsonObject json_obj)
{
    if (m_packages.find(name) != m_packages.end())
        return false;

    m_packages.set(name, { filename, name, json_obj });
    return true;
}

Package* PackageDB::get(StringView name)
{
    auto it = m_packages.find(name);
    if (it == m_packages.end())
        return nullptr;

    return &(*it).value;
}

Package* PackageDB::find_package_that_provides(const String& executable)
{
    Package* ret { nullptr };
    for_each_package([&](auto& name, auto& package) {
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
