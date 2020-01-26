#pragma once

#include "Toolchain.h"
#include <AK/JsonObject.h>
#include <LibCore/CFile.h>
#include <LibCore/CObject.h>

class ToolchainDB : public CObject {
    C_OBJECT(ToolchainDB)

public:
    static ToolchainDB& the();
    ~ToolchainDB();

    void add(String, JsonObject);
    Toolchain* get(StringView);

private:
    ToolchainDB();

    HashMap<String, Toolchain> toolchains;
};
