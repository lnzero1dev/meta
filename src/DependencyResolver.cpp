#include "DependencyResolver.h"

DependencyResolver::DependencyResolver()
{
}

DependencyResolver::~DependencyResolver()
{
}

DependencyResolver& DependencyResolver::the()
{
    static DependencyResolver* s_the;
    if (!s_the)
        s_the = &DependencyResolver::construct().leak_ref();
    return *s_the;
}

bool DependencyResolver::resolve_dependencies(const HashMap<String, Package>& packages)
{
    Vector<String> needed_dependencies;
    m_missing_dependencies.clear();

    for (auto& package : packages) {
        if (package.value.dependencies().size()) {
            for (auto& dependency : package.value.dependencies()) {
                needed_dependencies.append(dependency.key);
            }
        }
    }

    for (auto& dependency : needed_dependencies) {
        bool found = false;
        for (auto& package : packages) {
            if (package.key == dependency) {
                found = true;
                break;
            }
        }
        if (!found) {
            if (!m_missing_dependencies.contains_slow(dependency))
                m_missing_dependencies.append(dependency);
        }
    }

    return m_missing_dependencies.size() == 0;
}
