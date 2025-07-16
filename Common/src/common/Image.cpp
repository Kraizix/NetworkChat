#include "common/Image.h"
#include <filesystem>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>

std::string Image::ParseImage(const std::string& path)
{
    int x, y, n;
    std::string res;
    if (!std::filesystem::exists(path)) {
        return res; // File doesn't exist
    }
    unsigned char* d = stbi_load(path.c_str(), &x, &y, &n, 0);
    if (!d)
    {
		return res; // failed to load image
    }
    if (x * y * n >= std::numeric_limits<std::uint16_t>::max() ||  n != 3) // Check if image is too large or not using RGB channels
    {
		return res; // Image too large
		stbi_image_free(d);
    }
    res.resize(3 * sizeof(int) + x * y * n);
    memcpy(&res[0], (char*)&x, sizeof(int));
    memcpy(&res[sizeof(int)], (char*)&y, sizeof(int));
    memcpy(&res[2 * sizeof(int)], (char*)&n, sizeof(int));
    memcpy(&res[3 * sizeof(int)], d, x * y * n);
    stbi_image_free(d);
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
