#pragma once

#include "DataBase.h"
#include "Toolchain.h"

class ToolchainDB : public DataBase<Toolchain> {
public:
    static ToolchainDB& the()
    {
        static ToolchainDB* s_the;
        if (!s_the)
            s_the = static_cast<ToolchainDB*>(&ToolchainDB::construct().leak_ref());
        return *s_the;
    }
};
