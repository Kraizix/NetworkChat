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
	Log("Server stopped.");
}

void Server::Run()
{
	Log("Server started. Waiting for connections...");
	m_ioContext.run();
 }

bool Server::AddRoom(const TcpConnection::Ptr& roomConnection, std::string& data)
{
	if (m_connectionRooms.contains(roomConnection))
	{
		Log("Connection already has a room.");
		return false;
	}
	m_connectionRooms.insert({roomConnection, roomConnection->GetSocket().remote_endpoint().address().to_string() + ':' + data});
	Log("Room added: " + roomConnection->GetSocket().remote_endpoint().address().to_string() + data);
	return true;
}

void Server::GetRooms(std::vector<std::string>& rooms) const
{
	for (auto& room : m_connectionRooms)
	{
		rooms.push_back(room.second);
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
		Log("New connection accepted.");
		m_connections.push_back(connection);
		connection->Start();
	}
	else
	{
		Log("Error accepting connection: " + error.message());
		std::erase(m_connections, connection);
	}
	StartAccept();
}

void Server::RemoveConnection(TcpConnection::Ptr connection)
{
	m_connectionRooms.erase(connection);

	std::erase(m_connections, connection);
}

void Server::Log(const std::string& message)
{
	std::cout << message << '\n';
}
