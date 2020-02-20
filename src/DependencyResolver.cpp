#include "DependencyResolver.h"
#include "FileProvider.h"
#include "PackageDB.h"

DependencyNode::DependencyNode()
{
    children.clear();
}

DependencyNode::~DependencyNode()
{
}

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

// TODO: add infinit loop prevention of circular dependencies

NonnullOwnPtr<DependencyNode> DependencyResolver::get_dependency_tree(const Package& package) const
{
    auto m = make<DependencyNode>();
    m->package = &package;

    for (auto& dependency : package.dependencies()) {
        bool found_package = false;

        Package* dependent_package = PackageDB::the().get(dependency.key);

#ifdef DEBUG_META
        fprintf(stderr, "Package %s has dependency: %s\n", package.name().characters(), dependency.key.characters());
#endif
        if (dependent_package) {
            // Fixme: dependencies can only be of the same type! But: Allow host packages to have dependency to target, because we know what we can do...
            //if (package.machine() == dependent_package->machine() || (package.machine() == "host" && dependent_package->machine() == "target")) {
            found_package = true;
            m->children.append(get_dependency_tree(*dependent_package));
#ifdef DEBUG_META
            fprintf(stderr, "Package %s has now %i children.\n", package.name().characters(), m->children.size());
#endif
            //}
        } else {
            // search for package that provides this dependency in 'provides' attribute
            PackageDB::the().for_each_package([&](auto&, auto& package_provides) {
                if (package_provides.provides().size()) {
                    for (auto& provides : package_provides.provides()) {
                        for (auto& provide_value : provides.value) {
                            if (provide_value == dependency.key) {
                                if (package.machine() == package_provides.machine() || (package.machine() == "host" && package_provides.machine() == "target")) {
                                    found_package = true;
                                    m->children.append(get_dependency_tree(package_provides));
                                    return IterationDecision::Break;
                                }
                            }
                        }
                    }
                }
                return IterationDecision::Continue;
            });
        }

        if (!found_package && package.machine() == "host") {
            // When it's a host package, check the host machine (i.e. build machine), if the executable / library is existing
            // This could (must!) be done in the generated code, but for now, we do it here.
            // TODO: we can only check build tools for existence, move check of host tools into the host toolchain!

#ifdef DEBUG_META
            fprintf(stderr, "Checking for %s (which is a dependency of %s)\n", dependency.key.characters(), package.name().characters());
#endif

            if (dependency.key.contains("lib")) {
                if (FileProvider::the().check_host_library_available(dependency.key))
                    found_package = true;
            } else {
                if (FileProvider::the().check_host_command_available(dependency.key))
                    found_package = true;
            }
        }

        if (!found_package) {
            fprintf(stderr, "Did not find %s, which is a dependency of %s!\n", dependency.key.characters(), package.name().characters());
            m->missing_dependencies.append(dependency.key);
        }
    }

    return m;
}

const Vector<String> DependencyResolver::missing_dependencies(const DependencyNode* node) const
{
    if (!node)
        return {};

    Vector<String> missing;

    // Append node's missing dependencies
    for (auto& dependency : node->missing_dependencies)
        missing.append(dependency);

    // Append node's children's missing dependencies
    //    for (auto& child : node->children) {
    //        for (auto& dependency : missing_dependencies(child))
    //            missing.append(dependency);
    //    }

    return missing;
}
