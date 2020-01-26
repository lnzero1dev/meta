#pragma once

#include <AK/FileSystemPath.h>
#include <AK/Optional.h>
#include <LibCore/CFile.h>
#include <LibCore/CObject.h>
#include <regex.h>

struct GlobState {
public:
    regex_t compiled_regex;
    String base_dir;
    Vector<String> skip_paths;
    bool already_matched;
    bool relative_regex;
    String pattern;
};

class FileProvider : public CObject {
    C_OBJECT(FileProvider)

public:
    static FileProvider& the();

    String find_in_working_directory_and_parents(StringView filename);

    ~FileProvider();

    template<typename Callback>
    void for_each_parent_directory(Callback callback)
    {
        String dir = m_current_dir;

        while (dir != "/") {
            FileSystemPath dirname { dir };
            dir = dirname.dirname();
            if (callback(dir) == IterationDecision::Break)
                break;
        }
    }

    String current_dir() { return m_current_dir; }

    Vector<String> glob_all_meta_json_files(String root_directory);

    Vector<String> recursive_glob(const StringView& pattern, const StringView& base);
    Vector<String> recursive_glob(const StringView& pattern, const StringView& base, Vector<String> skip_paths);

private:
    FileProvider(StringView current_dir);

    bool match(const GlobState& state, String path);

    String m_current_dir;

    Vector<String> recursive_glob(GlobState state, const StringView& path);
};
