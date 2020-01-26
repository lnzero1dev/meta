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

void PackageDB::add(String name, JsonObject json_obj)
{
    packages.set(name, { json_obj });
}

Package* PackageDB::get(StringView name)
{
    auto it = packages.find(name);
    if (it == packages.end())
        return nullptr;

    return &(*it).value;
}
