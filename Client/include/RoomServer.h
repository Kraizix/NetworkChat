#pragma once
#include "Server/Server.h"
#include <mutex>
class RoomServer : public Server
{
public:
	RoomServer(std::mutex& mutex, boost::asio::ip::port_type port) : Server(port, CommandType::Disconnect | CommandType::Message | CommandType::Username | CommandType::Image), m_mutex(mutex)
	{

	}
	void Log(const std::string& message) override;
	void Broadcast(const Command& command) override;
	void SetOnWrite(std::function<void(Command)> callback);
private:
	std::function<void(Command)> m_onWrite;
	std::mutex& m_mutex;
};

