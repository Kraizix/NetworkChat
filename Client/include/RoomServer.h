#pragma once
#include "Server/Server.h"
#include <mutex>
class RoomServer : public Server
{
public:
	RoomServer(std::mutex& mutex, boost::asio::ip::port_type port) : Server(port, CommandType::Disconnect | CommandType::Message | CommandType::Username | CommandType::Image), m_mutex(mutex)
	{

	}
	/// <summary>
	/// Log a message to the server console.
	/// </summary>
	/// <param name="message">Message to log</param>
	void Log(const std::string& message) override;

	/// <summary>
	/// Broadcast a command to all connected clients.
	/// </summary>
	/// <param name="command">Command to broadcast</param>
	void Broadcast(const Command& command) override;

	/// <summary>
	/// Set a callback to be called when a command is written to the server.
	/// </summary>
	/// <param name="callback">Callback</param>
	void SetOnWrite(std::function<void(Command)> callback);
private:
	/// <summary>
	/// Callback to be called when a command is written to the server.
	/// </summary>
	std::function<void(Command)> m_onWrite;

	/// <summary>
	/// Mutex for console access;
	/// </summary>
	std::mutex& m_mutex;
};

