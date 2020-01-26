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

    template<typename Callback>
    void for_each_package(Callback callback)
    {
        for (auto& package : m_packages) {
            if (callback(package.key, package.value) == IterationDecision::Break)
                break;
        }
    };

private:
    PackageDB();

    HashMap<String, Package> m_packages;
};
