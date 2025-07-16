#pragma once

#include <boost/asio.hpp>
#include <vector>
#include "TcpConnection.h"
#include "Common/Command.h"

class Server
{
public:
	/// <summary>
	/// Create a server instance that listens on the specified port and handles specified command (use bitwise operator to combinate flags).
	/// </summary>
	/// <param name="port">Port to listen</param>
	/// <param name="flag">Command flags accepted</param>
	Server(boost::asio::ip::port_type port, CommandType flag);

	~Server();

	/// <summary>
	/// Run the server to accept connections and handle commands.
	/// </summary>
	void Run();

	/// <summary>
	/// Add a new room to the server.
	/// </summary>
	/// <param name="roomConnection">Connection pointer</param>
	/// <param name="data">Room ip address</param>
	/// <returns>Whether the room was added or not</returns>
	bool AddRoom(const TcpConnection::Ptr& roomConnection, std::string& data);

	/// <summary>
	/// Get the list of rooms available on the server.
	/// </summary>
	/// <param name="rooms">Vector to use to add rooms addresses</param>
	void GetRooms(std::vector<std::string>& rooms) const;

	/// <summary>
	/// Broadcast a command to all connected clients.
	/// </summary>
	/// <param name="command">Command to Broadcast</param>
	virtual void Broadcast(const Command& command);

	/// <summary>
	/// Print a message to the console.
	/// </summary>
	/// <param name="message">Message to print.</param>
	virtual void Log(const std::string& message);
	CommandType m_flag;
private:
	/// <summary>
	/// Start accepting new connection.
	/// </summary>
	void StartAccept();

	/// <summary>
	/// Handle the accept operation of incoming connection.
	/// </summary>
	/// <param name="connection">Connection Pointer</param>
	/// <param name="error">Asio error code</param>
	void HandleAccept(TcpConnection::Ptr connection, const boost::system::error_code error);

	/// <summary>
	/// Remove a connection from the server.
	/// </summary>
	/// <param name="connection">Connection pointer to remove</param>
	void RemoveConnection(TcpConnection::Ptr connection);

	boost::asio::io_context m_ioContext;
	boost::asio::ip::tcp::acceptor m_acceptor;
	std::vector<TcpConnection::Ptr> m_connections;
	std::unordered_map<TcpConnection::Ptr, std::string> m_connectionRooms;
};
