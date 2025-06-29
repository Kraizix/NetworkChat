#include "Server.h"

#include <iostream>
#include "TcpConnection.h"

Server::Server(boost::asio::ip::port_type port) :m_acceptor(m_ioContext, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
{
	StartAccept();
}

Server::~Server()
{
	for (auto& connection : m_connections)
	{
		connection->GetSocket().close();
	}
	m_connections.clear();
	std::cout << "Server stopped." << std::endl;
}

void Server::Run()
{
	std::cout << "Server is running." << std::endl;
	m_ioContext.run();
}

void Server::StartAccept()
{
	TcpConnection::Ptr connection = TcpConnection::Create(m_ioContext);
	connection->SetOnDisconnect([this](auto ptr) { this->RemoveConnection(ptr); });
	m_connections.push_back(connection);
	m_acceptor.async_accept(connection->GetSocket(),
							std::bind(&Server::HandleAccept, this, connection,
								boost::asio::placeholders::error));
}

void Server::HandleAccept(TcpConnection::Ptr connection, const boost::system::error_code error)
{
	if (!error)
	{
		std::cout << "New connection accepted." << std::endl;
		connection->Start();
	}
	else
	{
		std::cerr << "Error accepting connection: " << error.message() << std::endl;
		m_connections.erase(std::remove(m_connections.begin(), m_connections.end(), connection), m_connections.end());
	}
	StartAccept();
}

void Server::RemoveConnection(TcpConnection::Ptr connection)
{
	m_connections.erase(std::remove(m_connections.begin(), m_connections.end(), connection), m_connections.end());
}
