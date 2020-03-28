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
    void gen_toolchain(const Toolchain&, const Vector<String>& json_input_files);
    void gen_root(const Toolchain&, int argc, char** argv);

private:
    CMakeGenerator();

    const String gen_cmake_toolchain_content(const HashMap<String, Tool>&, Optional<const HashMap<String, HashMap<String, ToolConfiguration>>>);
    String gen_toolchain_package(const Package&);
    StringBuilder gen_toolchain_cmakelists_txt();
    String gen_package_collection(const Package&);

    String make_path_with_cmake_variables(const String& path);

    const String gen_header() const;
    const String cmake_minimum_version() const;
    const String project_root_dir() const;
    const String colorful_message() const;
    const String make_command_workaround() const;
    const String includes() const;
    const String find_tools_not_in_toolchain(const HashMap<String, Tool>& tools) const;
};
