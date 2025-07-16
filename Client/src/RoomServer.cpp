#include "RoomServer.h"

#include <iostream>

void RoomServer::Log(const std::string& message)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	std::cout << "[RoomServer] " << message << std::endl;
}

void RoomServer::Broadcast(const Command& command)
{
	if ((static_cast<uint8_t>(command.type) & static_cast<uint8_t>(m_flag)) == 0)
		return;
	if (m_onWrite)
		m_onWrite(command);
	Server::Broadcast(command);

}

void RoomServer::SetOnWrite(std::function<void(Command)> callback)
{
	m_onWrite = callback;
}
