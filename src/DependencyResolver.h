#pragma once

#include "Package.h"
#include <LibCore/CObject.h>

class DependencyResolver : public Core::Object {
    C_OBJECT(DependencyResolver)

public:
    static DependencyResolver& the();
    ~DependencyResolver();

    bool resolve_dependencies(const HashMap<String, Package>& packages);
    const Vector<String>& missing_dependencies() const { return m_missing_dependencies; }

private:
    DependencyResolver();

    Vector<String> m_missing_dependencies;
};
