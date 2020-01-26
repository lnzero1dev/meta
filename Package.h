#pragma once

#include <AK/JsonObject.h>
#include <LibCore/CFile.h>
#include <LibCore/CObject.h>

class Package {

public:
    Package(JsonObject);
    ~Package();

private:
};
