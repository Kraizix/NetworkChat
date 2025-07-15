#pragma once

#include <string>
#include <vector>
#include "ser/Buffer.h"

#define RoomFlags CommandType::ListRooms | CommandType::JoinRoom | CommandType::CreateRoom
#define RoomOK "OK"
#define RoomError "ERROR"

enum class CommandType : uint8_t
{
	Message = 1,
	ListRooms = 2,
	JoinRoom = 4,
	CreateRoom = 8,
	Disconnect = 16,
	Username = 32,
};


inline CommandType operator|(CommandType lhs, CommandType rhs)
{
	return static_cast<CommandType>(static_cast<uint8_t>(lhs) | static_cast<uint8_t>(rhs));
}

inline CommandType operator&(CommandType lhs, CommandType rhs)
{
	return static_cast<CommandType>(static_cast<uint8_t>(lhs) & static_cast<uint8_t>(rhs));
}

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
