#pragma once

#include "Settings.h"
#include <AK/HashMap.h>
#include <AK/JsonObject.h>
#include <AK/Types.h>
#include <LibCore/Object.h>

enum class SettingsPriority : u8 {
    Undefined = 0,
    User,   // highes priority
    Project // lowest priority
};

namespace AK {
template<>
struct Traits<SettingsPriority> : public GenericTraits<SettingsPriority> {
    static constexpr bool is_trivial() { return true; }
    static unsigned hash(SettingsPriority i) { return int_hash((int)i); }
    static void dump(SettingsPriority i) { kprintf("%d", (int)i); }
};
}

class SettingsProvider : public Core::Object {
    C_OBJECT(SettingsProvider)

public:
    static SettingsProvider& the();
    ~SettingsProvider();

    bool add(const String& filename, SettingsPriority priority, const JsonObject& settings_object);
    Optional<SettingsParameter> get(const String& parameter);
    Optional<String> get_string(const String& parameter);

    void list_all();

private:
    SettingsProvider();

    HashMap<SettingsPriority, Settings*> m_settings;
    Vector<SettingsPriority> m_settings_sorted_keys;
};
