#include "ToolchainDB.h"
#include <AK/JsonObject.h>
#include <AK/JsonValue.h>

ToolchainDB::ToolchainDB()
{
}

ToolchainDB::~ToolchainDB()
{
}

ToolchainDB& ToolchainDB::the()
{
    static ToolchainDB* s_the;
    if (!s_the)
        s_the = &ToolchainDB::construct().leak_ref();
    return *s_the;
}

void ToolchainDB::add(String name, JsonObject json_obj)
{
    toolchains.set(name, { json_obj });
}

Toolchain* ToolchainDB::get(StringView name)
{
    auto it = toolchains.find(name);
    if (it == toolchains.end())
        return nullptr;

    return &(*it).value;
}
