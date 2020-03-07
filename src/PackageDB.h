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

    bool add(const String&, const String&, const JsonObject&);
    Package* get(MachineType machine, StringView name);

    template<typename Callback>
    void for_each_package(MachineType machine, Callback callback)
    {
        auto packages = (machine == MachineType::Build) ? m_packages_build : ((machine == MachineType::Host) ? m_packages_host : packages_target());

        for (auto& package : packages) {
            if (callback(package.key, package.value) == IterationDecision::Break)
                break;
        }
    };

    template<typename Callback>
    void for_each_build_package(Callback callback)
    {
        for (auto& package : m_packages_build) {
            if (callback(package.key, package.value) == IterationDecision::Break)
                break;
        }
    };

    template<typename Callback>
    void for_each_host_package(Callback callback)
    {
        for (auto& package : m_packages_host) {
            if (callback(package.key, package.value) == IterationDecision::Break)
                break;
        }
    };

    template<typename Callback>
    void for_each_target_package(Callback callback)
    {
        for (auto& package : m_packages_target) {
            if (callback(package.key, package.value) == IterationDecision::Break)
                break;
        }
    };

    const HashMap<String, Package>& packages_build() { return m_packages_build; }
    const HashMap<String, Package>& packages_host() { return m_packages_host; }
    const HashMap<String, Package>& packages_target() { return m_packages_target; }

    Package* find_host_package_that_provides(const String& executable);

private:
    PackageDB();

    HashMap<String, Package> m_packages_build;
    HashMap<String, Package> m_packages_host;
    HashMap<String, Package> m_packages_target;
};
