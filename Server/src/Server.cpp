#include "Server.h"

#include <iostream>
#include "TcpConnection.h"

Server::Server(boost::asio::ip::port_type port, CommandType flag) : m_acceptor(m_ioContext, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)), m_flag(flag)
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

bool Server::AddRoom(TcpConnection::Ptr roomConnection)
{
	if (std::find(m_connections.begin(), m_connections.end(), roomConnection) != m_connections.end())
	{
		std::cerr << "Connection already exists." << std::endl;
		return false;
	}
	m_connections.push_back(roomConnection);
	std::cout << "Room added successfully." << std::endl;
	return true;
}

void Server::GetRooms(std::vector<std::string>& rooms) const
{
	for (auto& room : m_rooms)
	{
		rooms.push_back(room->GetSocket().remote_endpoint().address().to_string());
	}
}

void Server::Broadcast(const Command& command)
{
	for(auto& connection : m_connections)
	{
		connection->AddCommand(command);
	}
}

void Server::StartAccept()
{
	TcpConnection::Ptr connection = TcpConnection::Create(this, m_ioContext, m_flag);
	connection->SetOnDisconnect([this](auto ptr) { this->RemoveConnection(ptr); });
	m_acceptor.async_accept(connection->GetSocket(),
							std::bind(&Server::HandleAccept, this, connection,
								boost::asio::placeholders::error));
}

void Server::HandleAccept(TcpConnection::Ptr connection, const boost::system::error_code error)
{
	if (!error)
	{
		std::cout << "New connection accepted." << std::endl;
		m_connections.push_back(connection);
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
	if (std::find(m_rooms.begin(), m_rooms.end(), connection) != m_rooms.end())
		m_rooms.erase(std::remove(m_rooms.begin(), m_rooms.end(), connection), m_rooms.end());

	m_connections.erase(std::remove(m_connections.begin(), m_connections.end(), connection), m_connections.end());
}
