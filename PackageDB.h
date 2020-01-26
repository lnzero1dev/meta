#pragma once

#include "Package.h"
#include <AK/HashMap.h>
#include <AK/JsonObject.h>
#include <LibCore/CFile.h>
#include <LibCore/CObject.h>

class PackageDB : public CObject {
    C_OBJECT(PackageDB)

public:
    static PackageDB& the();
    ~PackageDB();

    void add(String, JsonObject);
    Package* get(StringView name);

private:
    PackageDB();

    HashMap<String, Package> packages;
};
