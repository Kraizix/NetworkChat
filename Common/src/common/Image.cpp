#include "common/Image.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>

std::string Image::ParseImage(const std::string& data)
{
    int x, y, n;
    unsigned char* d = stbi_load(data.c_str(), &x, &y, &n, 0);
    std::string res;
    res.resize(3 * sizeof(int) + x * y * n);
    memcpy(&res[0], (char*)&x, sizeof(int));
    memcpy(&res[sizeof(int)], (char*)&y, sizeof(int));
    memcpy(&res[2 * sizeof(int)], (char*)&n, sizeof(int));
    memcpy(&res[3 * sizeof(int)], d, x * y * n);
    return res;
}

Image Image::UnParseImage(const std::string& data)
{
    Image img;
    memcpy(&img, &data[0], 3 * sizeof(int));
    img.data.resize(img.width * img.height * img.channels);
    memcpy(img.data.data(), &data[3 * sizeof(int)], img.data.size());
    return img;
}
