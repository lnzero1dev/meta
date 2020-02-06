#include "Image.h"

Image::Image(String filename, JsonObject json_obj)
{
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
    });
}

Image::~Image()
{
}

