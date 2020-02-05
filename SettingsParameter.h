#pragma once

#include <AK/JsonObject.h>
#include <AK/String.h>

class SettingsParameter {
public:
    enum class Type {
        Undefined,
        String,
        JsonObject,
    };

    explicit SettingsParameter(String filename, Type type);

    SettingsParameter(const SettingsParameter&);
    SettingsParameter(SettingsParameter&&);

    SettingsParameter& operator=(const SettingsParameter&);
    SettingsParameter& operator=(SettingsParameter&&);

    ~SettingsParameter() { clear(); }

    SettingsParameter(String filename, const char*);
    SettingsParameter(String filename, const String&);
    SettingsParameter(String filename, const JsonObject&);
    SettingsParameter(String filename, JsonObject&&);

    String as_string() const
    {
        ASSERT(is_string());
        return *m_value.as_string;
    }

    String as_string_or(const String& alternative)
    {
        if (is_string())
            return as_string();
        return alternative;
    }

    Type type() const
    {
        return m_type;
    }

    const String& filename() const
    {
        return m_filename;
    }

    bool is_undefined() const { return m_type == Type::Undefined; }
    bool is_string() const { return m_type == Type::String; }
    bool is_json_object() const { return m_type == Type::JsonObject; }

private:
    void clear();
    void copy_from(const SettingsParameter&);

    Type m_type { Type::Undefined };
    String m_filename;

    union {
        StringImpl* as_string { nullptr };
        JsonObject* as_object;
    } m_value;
};
