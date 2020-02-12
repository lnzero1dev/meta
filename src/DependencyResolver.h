#pragma once

#include "Package.h"
#include <LibCore/CObject.h>

class DependencyNode {
public:
    DependencyNode();
    ~DependencyNode();
    Vector<NonnullOwnPtr<DependencyNode>> children {};
    Vector<String> missing_dependencies {};
    Package const* package { nullptr };
    //DependencyNode const* parent { nullptr };

    template<typename Callback>
    static void start_by_leave(const DependencyNode* node, Callback callback)
    {
        ASSERT(node);
        for (auto& child : node->children) {
            start_by_leave(child, callback);
        }

        if (node->package) {
            callback(*node->package);
        }
    }
};

class DependencyResolver : public Core::Object {
    C_OBJECT(DependencyResolver)

public:
    static DependencyResolver& the();
    ~DependencyResolver();

    NonnullOwnPtr<DependencyNode> get_dependency_tree(const Package& package) const;
    const Vector<String> missing_dependencies(const DependencyNode* node) const;

private:
    DependencyResolver();
};
