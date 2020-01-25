#pragma once

#include <AK/FileSystemPath.h>
#include <AK/Optional.h>
#include <LibCore/CFile.h>
#include <LibCore/CObject.h>

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

private:
    FileProvider(StringView current_dir);

    String m_current_dir;
};
