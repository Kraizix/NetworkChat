#pragma once

#include <string>
#include <vector>
struct Image
{
    int width;
    int height;
    int channels;
    std::vector<unsigned char> data;
	static std::string ParseImage(const std::string& data);
	static Image UnParseImage(const std::string& data);
};;


