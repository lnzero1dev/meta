#pragma once

#include "Image.h"
#include <AK/HashMap.h>
#include <AK/JsonObject.h>
#include <LibCore/Object.h>

class ImageDB : public Core::Object {
    C_OBJECT(ImageDB)

public:
    static ImageDB& the();
    ~ImageDB();

    bool add(String, String, JsonObject);
    Image* get(StringView name);

    template<typename Callback>
    void for_each_image(Callback callback)
    {
        for (auto& images : m_images) {
            if (callback(images.key, images.value) == IterationDecision::Break)
                break;
        }
    };

    const HashMap<String, Image>& images() { return m_images; }

private:
    ImageDB();

    HashMap<String, Image> m_images;
};
