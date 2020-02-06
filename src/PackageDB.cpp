#include "PackageDB.h"
#include <AK/JsonObject.h>
#include <AK/JsonValue.h>

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

void PackageDB::add(String filename, String name, JsonObject json_obj)
{
    m_packages.set(name, { filename, json_obj });
}

Package* PackageDB::get(StringView name)
{
    auto it = m_packages.find(name);
    if (it == m_packages.end())
        return nullptr;

    return &(*it).value;
}
