#pragma once

#include "DataBase.h"
#include "Package.h"

class PackageDB : public DataBase<Package> {
public:
    Package* find_package_that_provides(const String& executable);
};

class BuildPackageDB : public PackageDB {
public:
    static BuildPackageDB& the()
    {
        static BuildPackageDB* s_the;
        if (!s_the)
            s_the = static_cast<BuildPackageDB*>(&BuildPackageDB::construct().leak_ref());
        return *s_the;
    }
};

class HostPackageDB : public PackageDB {
public:
    static HostPackageDB& the()
    {
        static HostPackageDB* s_the;
        if (!s_the)
            s_the = static_cast<HostPackageDB*>(&HostPackageDB::construct().leak_ref());
        return *s_the;
    }
};

class TargetPackageDB : public PackageDB {
public:
    static TargetPackageDB& the()
    {
        static TargetPackageDB* s_the;
        if (!s_the)
            s_the = static_cast<TargetPackageDB*>(&TargetPackageDB::construct().leak_ref());
        return *s_the;
    }
};

DataBase<Package>& package_db_for_machine(MachineType);

bool add_package(const String&, const String&, const JsonObject&);
