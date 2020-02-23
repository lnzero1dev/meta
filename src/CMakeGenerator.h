#pragma once

#include "Image.h"
#include "Package.h"
#include "Toolchain.h"
#include <LibCore/Object.h>

class CMakeGenerator : public Core::Object {
    C_OBJECT(CMakeGenerator)

public:
    static CMakeGenerator& the();
    ~CMakeGenerator();

    void gen_image(const Image&, const Vector<const Package*>);
    bool gen_package(const Package&);
    void gen_toolchain(const Toolchain&, const Vector<Package>&);
    void gen_root(const Toolchain&);

private:
    CMakeGenerator();

    const String gen_toolchain_content(const HashMap<String, Tool>&, Optional<const HashMap<String, HashMap<String, ToolConfiguration>>>);

    const String gen_header() const;
    const String cmake_minimum_version() const;
    const String project_root_dir() const;
};
