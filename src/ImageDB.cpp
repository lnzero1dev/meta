#include "ImageDB.h"

ImageDB::ImageDB()
{
}

ImageDB::~ImageDB()
{
}

ImageDB& ImageDB::the()
{
    static ImageDB* s_the;
    if (!s_the)
        s_the = &ImageDB::construct().leak_ref();
    return *s_the;
}

bool ImageDB::add(String filename, String name, JsonObject json_obj)
{
    if (m_images.find(name) != m_images.end())
        return false;

    m_images.set(name, { filename, json_obj });
    return true;
}

Image* ImageDB::get(StringView name)
{
    auto it = m_images.find(name);
    if (it == m_images.end())
        return nullptr;

    return &(*it).value;
}
