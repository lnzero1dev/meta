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
    m_toolchains.set(name, { json_obj });
}

Toolchain* ToolchainDB::get(StringView name)
{
    auto it = m_toolchains.find(name);
    if (it == m_toolchains.end())
        return nullptr;

    return &(*it).value;
}
