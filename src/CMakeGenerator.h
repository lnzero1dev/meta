#pragma once

#include "Image.h"
#include "Package.h"
#include "Toolchain.h"
#include <LibCore/CObject.h>

class CMakeGenerator : public Core::Object {
    C_OBJECT(CMakeGenerator)

public:
    static CMakeGenerator& the();
    ~CMakeGenerator();

    const String gen_header() const;

    void gen_image(const Image&);
    void gen_package(const Package&);
    void gen_toolchain(const Toolchain&, const Vector<Package>&);

private:
    CMakeGenerator();
};
