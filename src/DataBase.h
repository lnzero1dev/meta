#pragma once

#include <AK/HashMap.h>
#include <AK/JsonObject.h>
#include <AK/Optional.h>
#include <LibCore/Object.h>

template<class T>
class DataBase : public Core::Object {
    C_OBJECT(DataBase)

public:
    virtual ~DataBase() {};

    template<class... Args>
    bool add(String name, Args&&... args)
    {
        if (m_entries.find(name) != m_entries.end())
            return false;
        m_entries.set(name, { name, forward<Args>(args)... });
        return true;
    }

    const T* get(StringView name) const
    {
        auto it = m_entries.find(name);
        if (it == m_entries.end())
            return nullptr;

        return &(*it).value;
    }

    template<typename Callback>
    void for_each_entry(Callback callback)
    {
        for (auto& entry : m_entries) {
            if (callback(entry.key, entry.value) == IterationDecision::Break)
                break;
        }
    };

    const HashMap<String, T>& entries() { return m_entries; }

protected:
    DataBase() {};

    HashMap<String, T> m_entries;
};
