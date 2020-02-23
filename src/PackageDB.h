#pragma once

#include "Package.h"
#include <AK/HashMap.h>
#include <AK/JsonObject.h>
#include <LibCore/Object.h>

class PackageDB : public Core::Object {
    C_OBJECT(PackageDB)

public:
    static PackageDB& the();
    ~PackageDB();

    bool add(String, String, JsonObject);
    Package* get(StringView name);

    template<typename Callback>
    void for_each_package(Callback callback)
    {
        for (auto& package : m_packages) {
            if (callback(package.key, package.value) == IterationDecision::Break)
                break;
        }
    };

    const HashMap<String, Package>& packages() { return m_packages; }

    Package* find_package_that_provides(const String& executable);

private:
    PackageDB();

    HashMap<String, Package> m_packages;
};
