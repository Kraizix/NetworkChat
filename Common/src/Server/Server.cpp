#include "Server/Server.h"

#include <iostream>
#include "Server/TcpConnection.h"

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

bool Server::AddRoom(const TcpConnection::Ptr& roomConnection, std::string& data)
{
	/*if (std::ranges::find(m_connections, roomConnection) != m_connections.end())
	{
		std::cerr << "Connection already exists." << std::endl;
		return false;
	}*/
	m_rooms.push_back(data);
	std::cout << "Room added successfully." << std::endl;
	return true;
}

void Server::GetRooms(std::vector<std::string>& rooms) const
{
	for (auto& room : m_rooms)
	{
		rooms.push_back(room);
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
								std::placeholders::_1));
}

void Server::HandleAccept(TcpConnection::Ptr connection, const boost::system::error_code error)
{
	if (!error)
	{
		std::cout << "New connection accepted." << '\n';
		m_connections.push_back(connection);
		connection->Start();
	}
	else
	{
		std::cerr << "Error accepting connection: " << error.message() << '\n';
		std::erase(m_connections, connection);
	}
	StartAccept();
}

void Server::RemoveConnection(TcpConnection::Ptr connection)
{
	/*if (std::ranges::find(m_rooms, connection) != m_rooms.end())
		std::erase(m_rooms, connection);*/

	std::erase(m_connections, connection);
}
