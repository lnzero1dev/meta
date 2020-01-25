#pragma once

#include <AK/FileSystemPath.h>
#include <AK/Optional.h>
#include <LibCore/CFile.h>
#include <LibCore/CObject.h>

class FileProvider : public CObject {
    C_OBJECT(FileProvider)
public:
    static FileProvider& the();

    CFile& find_in_working_directory_and_parents(StringView filename);

    ~FileProvider();

private:
    FileProvider(StringView current_dir);

    FileSystemPath m_current_dir;
};
