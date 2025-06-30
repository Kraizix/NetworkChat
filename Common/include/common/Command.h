#pragma once

#include <string>
#include <vector>
#include "ser/Buffer.h"

enum class CommandType: uint8_t
{
	Message,
	Room,
};

struct Command
{
	uint16_t total_length;
	CommandType type;
	std::string data;
	ser::Buffer buffer;

	Command(CommandType _type, std::string _data): data(_data), type(_type), buffer(nullptr, 0)
	{
		total_length = static_cast<uint16_t>(data.size() + sizeof(size_t) + sizeof(type));
		buffer = ser::Buffer(new uint8_t[total_length + sizeof(total_length)], total_length + sizeof(total_length));
	}

	Command(uint16_t size): total_length(size), type(CommandType::Message), buffer(new uint8_t[size], size)
	{
	}

	~Command()
	{
	}
};