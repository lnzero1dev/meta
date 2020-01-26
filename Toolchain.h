#pragma once

#include <AK/JsonObject.h>
#include <LibCore/CFile.h>
#include <LibCore/CObject.h>

class Toolchain {

public:
    Toolchain(JsonObject json_obj);
    //    Toolchain(Toolchain&) = default;
    //    Toolchain(Toolchain&&) = default;

    //    Toolchain& operator=(Toolchain&) = default;
    //    Toolchain& operator=(Toolchain&&) = default;

    ~Toolchain();

private:
};
