#include "Image.h"
#include "StringUtils.h"

InstallDir Image::string_to_install_dir(String type)
{
    if (type.matches("BinDir"))
        return InstallDir::BinDir;
    else if (type.matches("SbinDir"))
        return InstallDir::SbinDir;
    else if (type.matches("LibexecDir"))
        return InstallDir::LibexecDir;
    else if (type.matches("SysconfDir"))
        return InstallDir::SysconfDir;
    else if (type.matches("SharedstateDir"))
        return InstallDir::SharedstateDir;
    else if (type.matches("LocalstateDir"))
        return InstallDir::LocalstateDir;
    else if (type.matches("IncludeDir"))
        return InstallDir::IncludeDir;
    else if (type.matches("DatarootDir"))
        return InstallDir::DatarootDir;
    else if (type.matches("DataDir"))
        return InstallDir::DataDir;
    else if (type.matches("InfoDir"))
        return InstallDir::InfoDir;
    else if (type.matches("LocaleDir"))
        return InstallDir::LocaleDir;
    else if (type.matches("ManDir"))
        return InstallDir::ManDir;
    else
        return InstallDir::Undefined;
}

const String Image::install_dir_to_string(InstallDir installDir)
{
    if (installDir == InstallDir::BinDir)
        return "BinDir";
    else if (installDir == InstallDir::SbinDir)
        return "SbinDir";
    else if (installDir == InstallDir::LibexecDir)
        return "LibexecDir";
    else if (installDir == InstallDir::SysconfDir)
        return "SysconfDir";
    else if (installDir == InstallDir::SharedstateDir)
        return "SharedstateDir";
    else if (installDir == InstallDir::LocalstateDir)
        return "LocalstateDir";
    else if (installDir == InstallDir::IncludeDir)
        return "IncludeDir";
    else if (installDir == InstallDir::DatarootDir)
        return "DatarootDir";
    else if (installDir == InstallDir::DataDir)
        return "DataDir";
    else if (installDir == InstallDir::InfoDir)
        return "InfoDir";
    else if (installDir == InstallDir::LocaleDir)
        return "LocaleDir";
    else if (installDir == InstallDir::ManDir)
        return "ManDir";

    return "Undefined";
}

Image::Image(const String& filename, const String& name, JsonObject json_obj)
{
    set_default_install_dirs();
    m_filename = filename;
    m_name = name;

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
                    m_install_dirs.set(installDir, value.as_string());
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
    m_install_dirs.set(InstallDir::BinDir, "bin");
    m_install_dirs.set(InstallDir::SbinDir, "sbin");
    m_install_dirs.set(InstallDir::LibexecDir, "libexec");
    m_install_dirs.set(InstallDir::SysconfDir, "etc");
    m_install_dirs.set(InstallDir::SharedstateDir, "shared");
    m_install_dirs.set(InstallDir::LocalstateDir, "var");
    m_install_dirs.set(InstallDir::IncludeDir, "include");
    m_install_dirs.set(InstallDir::DatarootDir, "share");
    m_install_dirs.set(InstallDir::DataDir, "share");
    m_install_dirs.set(InstallDir::InfoDir, "share/info");
    m_install_dirs.set(InstallDir::LocaleDir, "share/locale");
    m_install_dirs.set(InstallDir::ManDir, "share/man");
}
