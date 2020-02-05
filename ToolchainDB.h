#pragma once

#include "Toolchain.h"
#include <AK/JsonObject.h>
#include <LibCore/CFile.h>
#include <LibCore/CObject.h>

class ToolchainDB : public Core::Object {
    C_OBJECT(ToolchainDB)

public:
    static ToolchainDB& the();
    ~ToolchainDB();

    void add(String, JsonObject);
    Toolchain* get(StringView);

    template<typename Callback>
    void for_each_toolchain(Callback callback)
    {
        for (auto& toolchain : m_toolchains) {
            if (callback(toolchain.key, toolchain.value) == IterationDecision::Break)
                break;
        }
    };

    const HashMap<String, Toolchain>& toolchains() { return m_toolchains; }

private:
    ToolchainDB();

    HashMap<String, Toolchain> m_toolchains;
};
