#pragma once

#include "DataBase.h"
#include "Image.h"

class ImageDB : public DataBase<Image> {
public:
    static ImageDB& the()
    {
        static ImageDB* s_the;
        if (!s_the)
            s_the = static_cast<ImageDB*>(&ImageDB::construct().leak_ref());
        return *s_the;
    }
};
