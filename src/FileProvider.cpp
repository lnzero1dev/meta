#include "FileProvider.h"
#include "SettingsProvider.h"
#include <AK/StringBuilder.h>
#include <LibCore/CDirIterator.h>
#include <sys/stat.h>
#include <sys/wait.h>

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

#ifdef DEBUG_META
    fprintf(stderr, "Current working dir: %s\n", cwd);
#endif

    if (!s_the)
        s_the = &FileProvider::construct(cwd).leak_ref();
    return *s_the;
}

Vector<String> FileProvider::glob_all_meta_json_files(String root_directory)
{
    Vector<String> skip_paths;
    auto opt_build_dir = SettingsProvider::the().get("build_directory");
    if (opt_build_dir.has_value()) {
        skip_paths.append(opt_build_dir.value().as_string());
    }

    skip_paths.append("/home/ema/checkout/serenity/Base");
    skip_paths.append("/home/ema/checkout/serenity/Ports");
    skip_paths.append("/home/ema/checkout/serenity/Root");
    skip_paths.append("/home/ema/checkout/serenity/DevTools/meta/serenity/build");
    skip_paths.append("/home/ema/checkout/serenity/Toolchain");

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
#ifdef DEBUG_META
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
            pattern_builder.append(".");
        } else if (pattern[i] == '/') {
            pattern_builder.append("\\/");
        } else if (pattern[i] == '*' && i < pattern.length() - 1 && pattern[i + 1] == '*') {
            pattern_builder.append("(.*\\/)?");
            ++i;
            if (i < pattern.length() - 1 && pattern[i + 1] == '/') {
                ++i;
            }
        } else if (pattern[i] == '*') {
            pattern_builder.append("[^\\/]*");

        } else {
            pattern_builder.append(pattern[i]);
        }
    }

    const char* regexp = pattern_builder.build().characters();

#ifdef DEBUG_META
    fprintf(stderr, "compile_regexp: %s\n", regexp);
#endif

    regex_t regex;
    if (regcomp(&regex, regexp, REG_EXTENDED)) {
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
    Core::DirIterator di(current_dir, Core::DirIterator::SkipDots);
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
            //perror("stat");
            //fprintf(stderr, "file: %s\n", new_path.characters());
        }
    }

#ifdef DEBUG_META
    fprintf(stdout, "recursive_glob found:\n");
    for (auto& file : vec) {
        fprintf(stdout, "file: %s\n", file.characters());
    }
#endif
    return vec;
}

bool FileProvider::update_if_relative(String& path, String base)
{
    if (!path.starts_with("/")) {
        StringBuilder builder;
        builder.append(base);
        builder.append("/");
        builder.append(path);

        String path2 = builder.build().characters();

        char buf[PATH_MAX];
        char* res = realpath(path2.characters(), buf);
        if (res) {
            path = buf;
            return true;
        } else {
            // path its not existing, create it!
            // FIXME: This likely needs mkdir_p!
            int rc = mkdir(path2.characters(), 0755);
            if (rc < 0) {
                perror("mkdir");
                return false;
            }
            // after creating the path, we have to call realpath again to fully resolve the path
            char* res = realpath(path2.characters(), buf);
            if (res) {
                path = buf;
                return true;
            }
            return false;
        }
    }
    return true;
}

bool FileProvider::check_host_library_available(const String& library)
{
    bool found = false;
    StringBuilder builder;
    builder.append("ldconfig -p | grep ");
    builder.append(library);

    pid_t pid = fork();
    if (pid == 0) {
        int rc = execl("/bin/sh", "sh", "-c", builder.build().characters(), nullptr);
        if (rc < 0)
            perror("execl");
        exit(1);
    }
    int status;

    waitpid(pid, &status, 0);

    if (WIFEXITED(status)) {
        int exit_status = WEXITSTATUS(status);
        if (!exit_status) {
            found = true;
        }
    }
    return found;
}

bool FileProvider::check_host_command_available(const String& command)
{
    bool found = false;
    pid_t pid = fork();
    if (pid == 0) {
        int rc = execl("/usr/bin/env", "/usr/bin/env", "which", command.characters(), nullptr);
        if (rc < 0)
            perror("execl");
        exit(1);
    }

    int status;

    waitpid(pid, &status, 0);

    if (WIFEXITED(status)) {
        int exit_status = WEXITSTATUS(status);
        if (!exit_status) {
            found = true;
        }
    }
    return found;
}
