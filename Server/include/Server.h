#pragma once

#include <boost/asio.hpp>
#include <vector>
#include "ser/Serializer.h"
#include "TcpConnection.h"

class Server
{
public:
	Server(boost::asio::ip::port_type port);
	~Server();

	void Run();
private:
	void StartAccept();
	void HandleAccept(TcpConnection::Ptr connection, const boost::system::error_code error);
	void RemoveConnection(TcpConnection::Ptr connection);
	
	boost::asio::io_context m_ioContext;
	boost::asio::ip::tcp::acceptor m_acceptor;
	std::vector<TcpConnection::Ptr> m_connections;
};
