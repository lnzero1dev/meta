#include "Image.h"
#include "StringUtils.h"

InstallDir string_to_install_dir(String type)
{
    if (type.matches("bindir"))
        return InstallDir::BinDir;
    else if (type.matches("bindir"))
        return InstallDir::SbinDir;
    else if (type.matches("sbindir"))
        return InstallDir::LibexecDir;
    else if (type.matches("libexecdir"))
        return InstallDir::SysconfDir;
    else if (type.matches("sysconfdir"))
        return InstallDir::SharedstateDir;
    else if (type.matches("sharedstatedir"))
        return InstallDir::LocalstateDir;
    else if (type.matches("localstatedir"))
        return InstallDir::IncludeDir;
    else if (type.matches("includedir"))
        return InstallDir::DatarootDir;
    else if (type.matches("datarootdir"))
        return InstallDir::DataDir;
    else if (type.matches("datadir"))
        return InstallDir::InfoDir;
    else if (type.matches("infodir"))
        return InstallDir::LocaleDir;
    else if (type.matches("mandir"))
        return InstallDir::ManDir;
    else
        return InstallDir::Undefined;
}

Image::Image(String filename, JsonObject json_obj)
{
    set_default_install_dirs();
    m_filename = filename;

    json_obj.for_each_member([&](auto& key, auto& value) {
        if (key == "install") {
            if (value.is_array()) {
                value.as_array().for_each([&](auto& arr_value) {
                    m_install.append(arr_value.as_string());
                });
            } else if (value.is_string()) {
                if (value.as_string() == "*")
                    m_install_all = true;
                else
                    m_install.append(value.as_string());
            }
            return;
        }
        if (key == "build_tool") {
            m_build_tool = value.as_string();
            return;
        }
        if (key == "run_tool") {
            m_run_tool = value.as_string();
            return;
        }
        if (key == "install_dirs") {
            value.as_object().for_each_member([&](auto& key, auto& value) {
                auto installDir = string_to_install_dir(key);
                if (installDir != InstallDir::Undefined) {
                    String replaced = value.as_string();
                    if (potentially_contains_variable(replaced))
                        replaced = replace_variables(replaced, "datarootdir", imageInstallDirs.ensure(InstallDir::DatarootDir));

                    imageInstallDirs.set(installDir, replaced);
                }
            });
        }
    });
}

Image::~Image()
{
}

void Image::set_default_install_dirs()
{
    imageInstallDirs.set(InstallDir::BinDir, "bin");
    imageInstallDirs.set(InstallDir::SbinDir, "sbin");
    imageInstallDirs.set(InstallDir::LibexecDir, "libexec");
    imageInstallDirs.set(InstallDir::SysconfDir, "etc");
    imageInstallDirs.set(InstallDir::SharedstateDir, "shared");
    imageInstallDirs.set(InstallDir::LocalstateDir, "var");
    imageInstallDirs.set(InstallDir::IncludeDir, "include");
    imageInstallDirs.set(InstallDir::DatarootDir, "share");
    imageInstallDirs.set(InstallDir::DataDir, "share");
    imageInstallDirs.set(InstallDir::InfoDir, "share/info");
    imageInstallDirs.set(InstallDir::LocaleDir, "share/locale");
    imageInstallDirs.set(InstallDir::ManDir, "share/man");
}
