#include "DependencyResolver.h"
#include <sys/wait.h>

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
    Vector<String> needed_dependencies, needed_native_dependencies;
    m_missing_dependencies.clear();

    for (auto& package : packages) {
        if (package.value.dependencies().size()) {
            for (auto& dependency : package.value.dependencies()) {
                if (package.value.is_native())
                    needed_native_dependencies.append(dependency.key);
                else
                    needed_dependencies.append(dependency.key);
            }
        }
    }

    for (auto& dependency : find_missing_dependencies(packages, needed_dependencies)) {
        if (!m_missing_dependencies.contains_slow(dependency))
            m_missing_dependencies.append(dependency);
    }

    for (auto& dependency : find_missing_dependencies(packages, needed_native_dependencies)) {
        bool found = false;
        // check system if dependency is installed!
        fprintf(stdout, "Checking system availability for: %s\n", dependency.characters());
        if (dependency.contains("lib")) {
            // checking for lib
            StringBuilder builder;
            builder.append("ldconfig -p | grep ");
            builder.append(dependency);

            pid_t pid = fork();
            if (pid == 0) {
                int rc = execl("/bin/sh", "sh", "-c", builder.build().characters(), nullptr);
                if (rc < 0)
                    perror("execl");
                exit(1);
            }
            int status;

            waitpid(pid, &status, 0);

            if (WIFEXITED(status)) {
                int exit_status = WEXITSTATUS(status);
                if (!exit_status) {
                    found = true;
                }
            }

        } else {
            // checking for executable
            pid_t pid = fork();
            if (pid == 0) {
                int rc = execl("/usr/bin/env", "/usr/bin/env", "which", dependency.characters(), nullptr);
                if (rc < 0)
                    perror("execl");
                exit(1);
            }

            int status;

            waitpid(pid, &status, 0);

            if (WIFEXITED(status)) {
                int exit_status = WEXITSTATUS(status);
                if (!exit_status) {
                    found = true;
                }
            }
        }

        if (!found && !m_missing_dependencies.contains_slow(dependency))
            m_missing_dependencies.append(dependency);
    }
    return m_missing_dependencies.size() == 0;
}

const Vector<String> DependencyResolver::find_missing_dependencies(const HashMap<String, Package>& packages, const Vector<String>& needed_dependencies)
{
    Vector<String> missing;

    for (auto& dependency : needed_dependencies) {
        bool found = false;
        String resolved_dependency = dependency;
        for (auto& package : packages) {
            if (package.key == dependency) {
                found = true;
                break;
            }
            if (package.value.provides().size())
                for (auto& provides : package.value.provides()) {
                    for (auto& provide_value : provides.value) {
                        if (provide_value == dependency) {
                            found = true;
                            // exchange the dependency with the name of the package that provides the dependant
                            resolved_dependency = package.key;
                            break;
                        }
                    }
                    if (found) {
                        break;
                    }
                }
            if (found)
                break;
        }
        if (!found) {
            if (!missing.contains_slow(resolved_dependency))
                missing.append(resolved_dependency);
        }
    }

    return missing;
}
