#pragma once

#include <AK/JsonObject.h>
#include <AK/JsonValue.h>
#include <AK/String.h>

class Image {

public:
    Image(String, JsonObject);
    ~Image();

    const String& filename() const { return m_filename; }
    const Vector<String>& install() const { return m_install; }
    bool install_all() const { return m_install_all; }
    const String& build_tool() { return m_build_tool; }
    const String& run_tool() { return m_run_tool; }

private:
    String m_filename;
    Vector<String> m_install;
    bool m_install_all = false;

    String m_build_tool;
    String m_run_tool;
};
