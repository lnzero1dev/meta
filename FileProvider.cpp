#include "FileProvider.h"
#include <AK/StringBuilder.h>
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
