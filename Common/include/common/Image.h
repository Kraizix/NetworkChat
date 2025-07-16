#pragma once

#include <string>
#include <vector>
struct Image
{
    int width;
    int height;
    int channels;
    std::vector<unsigned char> data;

	/// <summary>
	/// Load an image from specified file path and serialize it's data into a string.
	/// </summary>
	/// <param name="path">Path of the image</param>
	/// <returns>Serialized data of the image</returns>
	static std::string ParseImage(const std::string& path);

	/// <summary>
	/// Unserialize the image data from a string and create an Image object.
	/// </summary>
	/// <param name="data">Serialized data of the image</param>
	/// <returns>Image object</returns>
	static Image UnParseImage(const std::string& data);
};;


