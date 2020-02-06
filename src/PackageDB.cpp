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

    m_packages.set(name, { filename, json_obj });
    return true;
}

Package* PackageDB::get(StringView name)
{
    auto it = m_packages.find(name);
    if (it == m_packages.end())
        return nullptr;

    return &(*it).value;
}
