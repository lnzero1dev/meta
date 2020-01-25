#include "FileProvider.h"
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

    if (!s_the)
        s_the = &FileProvider::construct(cwd).leak_ref();
    return *s_the;
}

CFile& FileProvider::find_in_working_directory_and_parents(StringView filename)
{
    CFile& file = CFile::construct(filename); // FIXME: search for file

    return file;
}
