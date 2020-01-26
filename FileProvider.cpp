#include "FileProvider.h"
#include "Settings.h"
#include <AK/FileSystemPath.h>
#include <AK/StringBuilder.h>
#include <LibCore/CDirIterator.h>
#include <sys/stat.h>
#include <unistd.h>

FileProvider::FileProvider(StringView current_dir)
    : m_current_dir { current_dir }
{
}

FileProvider::~FileProvider()
{
}

FileProvider& FileProvider::the()
{
    static FileProvider* s_the;
    auto* cwd = getcwd(nullptr, 0);

#ifdef META_DEBUG
    fprintf(stderr, "Current working dir: %s\n", cwd);
#endif

    if (!s_the)
        s_the = &FileProvider::construct(cwd).leak_ref();
    return *s_the;
}

String FileProvider::find_in_working_directory_and_parents(StringView filename)
{
    auto file = CFile::construct();

    for_each_parent_directory([&](auto directory) {
        StringBuilder builder;
        builder.append(directory);
        builder.append("/");
        builder.append(filename);
        file->set_filename(builder.build());
        if (file->exists())
            return IterationDecision::Break;
        return IterationDecision::Continue;
    });

    return file->filename();
}

Vector<String> FileProvider::glob_all_meta_json_files(String root_directory)
{
    Vector<String> skip_paths;
    String build_dir;
    if (Settings::the().get("build_directory", &build_dir)) {
        skip_paths.append(build_dir);
    }
    return recursive_glob("**/*.m.json", root_directory, skip_paths);
}

String without_base(StringView path, StringView base)
{
    if (path.starts_with(base)) {
        return path.substring_view(base.length() + 1, path.length() - base.length() - 1); // + 1 to remove trailing / from path
    }
    return path;
}

bool FileProvider::match(const GlobState& state, String path)
{
    if (state.relative_regex)
        path = without_base(path, state.base_dir);

    int reti = regexec(&state.compiled_regex, path.characters(), 0, NULL, 0);
    if (!reti) {
#ifdef META_DEBUG
        fprintf(stderr, "Regex match: %s - %s\n", state.pattern.characters(), path.characters());
#endif
        return true;
    } else if (reti == REG_NOMATCH) {
    } else {
        char buf[100];
        regerror(reti, &state.compiled_regex, buf, sizeof(buf));
        fprintf(stderr, "Regex match failed: %s\n", buf);
    }

    return false;
}

static regex_t compile_regex(const StringView& pattern)
{
    StringBuilder pattern_builder;

    for (size_t i = 0; i < pattern.length(); ++i) {
        if (pattern[i] == '.') {
            pattern_builder.append("\\.");
        } else if (pattern[i] == '?') {
            pattern_builder.append("\\(.\\)");
        } else if (pattern[i] == '/') {
            pattern_builder.append("\\/");
        } else if (pattern[i] == '*' && i < pattern.length() - 1 && pattern[i + 1] == '*') {
            pattern_builder.append("\\(.*\\/\\)\\?");
            ++i;
            if (i < pattern.length() - 1 && pattern[i + 1] == '/') {
                ++i;
            }
        } else if (pattern[i] == '*') {
            pattern_builder.append("\\(.*\\)");
        } else {
            pattern_builder.append(pattern[i]);
        }
    }

    const char* regexp = pattern_builder.build().characters();

#ifdef META_DEBUG
    fprintf(stderr, "compile_regexp: %s\n", regexp);
#endif

    regex_t regex;
    if (regcomp(&regex, regexp, 0)) {
        perror("regcomp");
    }

    return regex;
}

Vector<String> FileProvider::recursive_glob(const StringView& pattern, const StringView& base)
{
    struct GlobState state;
    state.base_dir = base;
    state.skip_paths = {};
    state.compiled_regex = compile_regex(pattern);
    state.relative_regex = !pattern.starts_with("/");
    state.pattern = pattern;
    return recursive_glob(state, base);
}

Vector<String> FileProvider::recursive_glob(const StringView& pattern, const StringView& base, Vector<String> skip_paths)
{
    struct GlobState state;
    state.base_dir = base;
    state.skip_paths = skip_paths;
    state.compiled_regex = compile_regex(pattern);
    state.relative_regex = !pattern.starts_with("/");
    state.pattern = pattern;
    return recursive_glob(state, base);
}

Vector<String> FileProvider::recursive_glob(GlobState state, const StringView& current_dir)
{
    CDirIterator di(current_dir, CDirIterator::SkipDots);
    Vector<String> vec;

    if (di.has_error()) {
        return vec;
    }
    while (di.has_next()) {
        String next = di.next_path();

        StringBuilder builder;
        builder.append(current_dir);
        builder.append("/");
        builder.append(next);
        String new_path = builder.build();

        struct stat st;

        if (stat(new_path.characters(), &st) == 0) {

            if (S_ISDIR(st.st_mode)) {
                bool skip = false;

                if (state.skip_paths.size()) {
                    for (auto& skip_path_it : state.skip_paths) {
                        if (new_path.starts_with(skip_path_it)) {
                            skip = true;
                            break;
                        }
                    }
                }

                if (!skip) {
                    vec.append(recursive_glob(state, new_path));
                }

            } else {
                // Files, sockets and other iterated items that aren't directories
                if (match(state, new_path))
                    vec.append(new_path);
            }
        } else {
            perror("stat");
            fprintf(stderr, "file: %s\n", new_path.characters());
        }
    }

    return vec;
}
