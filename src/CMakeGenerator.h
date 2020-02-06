#pragma once

#include <LibCore/CObject.h>

class CMakeGenerator : public Core::Object {
    C_OBJECT(CMakeGenerator)

public:
    static CMakeGenerator& the();
    ~CMakeGenerator();

    void gen_image(const String&);
    void gen_package(const String&);

private:
    CMakeGenerator();
};
