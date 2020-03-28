#pragma once

#include "SettingsProvider.h"
#include "StringUtils.h"
#include <AK/FileSystemPath.h>
#include <AK/Function.h>
#include <AK/Optional.h>
#include <LibCore/File.h>
#include <LibCore/Object.h>
#include <regex.h>

bool create_dir(const String& path, const String& sub_dir = "");

struct GlobState {
public:
    regex_t compiled_regex;
    String base_dir;
    Vector<String> skip_paths;
    bool already_matched;
    bool relative_regex;
    String pattern;
};

class FileProvider : public Core::Object {
    C_OBJECT(FileProvider)

public:
    static FileProvider& the();
    ~FileProvider();

    template<typename Callback>
    void for_each_parent_directory(Callback callback)
    {
        String dir = m_current_dir;

        while (dir != "/" && !dir.is_empty()) {
            FileSystemPath dirname { dir };
            dir = dirname.dirname();
            if (dir.is_empty())
                dir = "/";
            if (callback(dir) == IterationDecision::Break)
                break;
        }
    }

    template<typename Callback>
    void for_each_parent_directory_including_current_dir(Callback callback)
    {
        String dir = m_current_dir;
        if (callback(dir) == IterationDecision::Break)
            return;

        for_each_parent_directory(callback);
    }

    String current_dir() { return m_current_dir; }

    Vector<String> glob_all_meta_json_files(String root_directory);

    Vector<String> recursive_glob(const StringView& pattern, const StringView& base);
    Vector<String> recursive_glob(const StringView& pattern, const StringView& base, Vector<String> skip_paths);
    Vector<String> glob(const StringView& pattern, const String& base);


    bool check_host_library_available(const String&);
    bool check_host_command_available(const String&);

    String replace_path_variables(
        const String& path, Function<Optional<String>(const String&)>* callback = nullptr) const;
    String make_absolute_path(const String& path, const String& base);
    String full_path_update(const String& path, const String& base, Function<Optional<String>(const String&)>* callback = nullptr);

private:
    FileProvider(StringView current_dir);

    bool match(const GlobState& state, String path);
    bool match(const String& haystack, const regex_t& needle);

    String m_current_dir;

    Vector<String> recursive_glob(GlobState state, const StringView& path);
};
