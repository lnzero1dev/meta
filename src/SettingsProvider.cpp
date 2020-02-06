#include "SettingsProvider.h"
#include <AK/QuickSort.h>

SettingsProvider::SettingsProvider()
{
}

SettingsProvider::~SettingsProvider()
{
    for (auto& settings : m_settings) {
        delete settings.value;
    }
}

SettingsProvider& SettingsProvider::the()
{
    static SettingsProvider* s_the;
    if (!s_the)
        s_the = &SettingsProvider::construct().leak_ref();
    return *s_the;
}

bool SettingsProvider::add(const String& filename, SettingsPriority priority, const JsonObject& settings_object)
{
    if (!m_settings.contains(priority)) {
        m_settings.set(priority, new Settings());
        m_settings_sorted_keys.append(priority);
        quick_sort(m_settings_sorted_keys.begin(), m_settings_sorted_keys.end(), [](auto& a, auto& b) { return (int)a <= (int)b; });
    }

    auto& settings = *m_settings.ensure(priority);
    if (!settings.load(filename, settings_object)) {
        fprintf(stdout, "Error loading settings file: %s\n", filename.characters());
        return false;
    }
    return true;
}

Optional<SettingsParameter> SettingsProvider::get(const String& parameter)
{
    for (auto& key : m_settings_sorted_keys) {
        auto optionalParameter = (*m_settings.ensure(key)).get({}, parameter);
        if (optionalParameter.has_value())
            return optionalParameter.value();
    }
    return {};
}

Optional<String> SettingsProvider::get_string(const String& parameter)
{
    auto param = get(parameter);
    if (param.has_value() && param.value().is_string()) {
        return param.value().as_string();
    }
    return {};
}

void SettingsProvider::list_all()
{
    Vector<String> string_params = { "root", "toolchain", "build_directory", "gendata_directory", "build_generator" };
    for (auto& param : string_params) {
        auto value = get_string(param);
        fprintf(stdout, "%s: %s\n", param.characters(), value.value_or("").characters());
    }
}
