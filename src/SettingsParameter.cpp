#include "SettingsParameter.h"

SettingsParameter::SettingsParameter(String filename, Type type)
    : m_type(type)
    , m_filename(filename)
{
}

SettingsParameter::SettingsParameter(const SettingsParameter& other)
{
    copy_from(other);
}

SettingsParameter& SettingsParameter::operator=(const SettingsParameter& other)
{
    if (this != &other) {
        clear();
        copy_from(other);
    }
    return *this;
}

void SettingsParameter::copy_from(const SettingsParameter& other)
{
    m_type = other.m_type;
    m_filename = other.m_filename;
    switch (m_type) {
    case Type::String:
        ASSERT(!m_value.as_string);
        m_value.as_string = other.m_value.as_string;
        m_value.as_string->ref();
        break;
    case Type::BuildGenerator:
        ASSERT(!m_value.as_buildgenerator);
        m_value.as_buildgenerator = other.m_value.as_buildgenerator;
        break;
    case Type::JsonObject:
        m_value.as_object = new JsonObject(*other.m_value.as_object);
        break;
    default:
        m_value.as_string = other.m_value.as_string;
        break;
    }
}

SettingsParameter::SettingsParameter(SettingsParameter&& other)
{
    m_type = exchange(other.m_type, Type::Undefined);
    m_filename = other.m_filename;
    m_value.as_string = exchange(other.m_value.as_string, nullptr);
}

SettingsParameter& SettingsParameter::operator=(SettingsParameter&& other)
{
    if (this != &other) {
        clear();
        m_type = exchange(other.m_type, Type::Undefined);
        m_value.as_string = exchange(other.m_value.as_string, nullptr);
        m_filename = other.m_filename;
    }
    return *this;
}

void SettingsParameter::clear()
{
    switch (m_type) {
    case Type::String:
        m_value.as_string->unref();
        break;
    case Type::JsonObject:
        delete m_value.as_object;
        break;
    default:
        break;
    }
    m_type = Type::Undefined;
    m_value.as_string = nullptr;
}

SettingsParameter::SettingsParameter(String filename, const char* cstring)
    : SettingsParameter(filename, String(cstring))
{
}

SettingsParameter::SettingsParameter(String filename, const String& value)
    : m_filename(filename)
{
    if (value.is_null()) {
        m_type = Type::Undefined;
    } else {
        m_type = Type::String;
        m_value.as_string = const_cast<StringImpl*>(value.impl());
        m_value.as_string->ref();
    }
}

SettingsParameter::SettingsParameter(String filename, const BuildGenerator value)
    : m_filename(filename)
{
    m_type = Type::BuildGenerator;
    m_value.as_buildgenerator = new BuildGenerator(value);
}

SettingsParameter::SettingsParameter(String filename, const JsonObject& value)
    : m_type(Type::JsonObject)
    , m_filename(filename)
{
    m_value.as_object = new JsonObject(value);
}
