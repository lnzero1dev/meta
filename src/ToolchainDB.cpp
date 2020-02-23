#include "ToolchainDB.h"

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

bool ToolchainDB::add(String filename, String name, JsonObject json_obj)
{
    if (m_toolchains.find(name) != m_toolchains.end())
        return false;

    m_toolchains.set(name, { filename, json_obj });
    return true;
}

Toolchain* ToolchainDB::get(StringView name)
{
    auto it = m_toolchains.find(name);
    if (it == m_toolchains.end())
        return nullptr;

    return &(*it).value;
}
