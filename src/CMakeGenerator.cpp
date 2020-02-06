#include "CMakeGenerator.h"

CMakeGenerator::CMakeGenerator()
{
}

CMakeGenerator::~CMakeGenerator()
{
}

CMakeGenerator& CMakeGenerator::the()
{
    static CMakeGenerator* s_the;
    if (!s_the)
        s_the = &CMakeGenerator::construct().leak_ref();
    return *s_the;
}

void CMakeGenerator::gen_image(const String&)
{
}

void CMakeGenerator::gen_package(const String&)
{
}
