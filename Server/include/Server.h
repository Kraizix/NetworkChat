#pragma once

#include <boost/asio.hpp>
#include <vector>
#include "TcpConnection.h"
#include "Common/Command.h"

class Server
{
public:
	Server(boost::asio::ip::port_type port, CommandType flag);
	~Server();

	void Run();

	bool AddRoom(const TcpConnection::Ptr& roomConnection);
	void GetRooms(std::vector<std::string>& rooms) const;
	void Broadcast(const Command& command);
private:
	void StartAccept();
	void HandleAccept(TcpConnection::Ptr connection, const boost::system::error_code error);
	void RemoveConnection(TcpConnection::Ptr connection);
	
	boost::asio::io_context m_ioContext;
	boost::asio::ip::tcp::acceptor m_acceptor;
	std::vector<TcpConnection::Ptr> m_connections;
	
	std::vector<TcpConnection::Ptr> m_rooms;
	CommandType m_flag;
};
