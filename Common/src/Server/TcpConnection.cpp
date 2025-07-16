#include "Server/TcpConnection.h"

#include <iostream>
#include <ranges>
#include "ser/Buffer.h"
#include "ser/Serializer.h"
#include "Server/Server.h"

TcpConnection::TcpConnection(Server* server, boost::asio::io_context& io_context, CommandType flag) : m_server(server), m_socket(io_context), m_command(512), m_acceptedTypes(flag), m_username("Guest")
{
	m_commandHandlers.emplace(CommandType::Message, [this] { HandleMessage(); });
	m_commandHandlers.emplace(CommandType::ListRooms, [this] { HandleRoomList(); });
	m_commandHandlers.emplace(CommandType::CreateRoom, [this] { HandleRoomCreate(); });
	m_commandHandlers.emplace(CommandType::JoinRoom, [this] { HandleRoomJoin(); });
	m_commandHandlers.emplace(CommandType::Disconnect, [this] { HandleDisconnect(); });
	m_commandHandlers.emplace(CommandType::Username, [this] { HandleUsername();  });
	m_commandHandlers.emplace(CommandType::Image, [this] { HandleImage(); });
}

void TcpConnection::Start()
{
	StartRead();
}

boost::asio::ip::tcp::socket& TcpConnection::GetSocket()
{
	return m_socket;
}


void TcpConnection::SetOnDisconnect(std::function<void(Ptr)> callback)
{
	m_onDisconnect = callback;
}

void TcpConnection::AddCommand(const Command& command)
{
	m_writeQueue.push_back(command);
	if (!m_isWriting)
		StartWrite();
}


void TcpConnection::StartRead()
{
	boost::asio::async_read(m_socket, boost::asio::buffer(m_command.buffer.m_data, sizeof(m_command.total_length)),
		std::bind(&TcpConnection::HandleReadHeader, shared_from_this(),
			std::placeholders::_1));
}

void TcpConnection::HandleReadHeader(const boost::system::error_code& error)
{
	if (error)
	{
		m_server->Log("Error on header read: " + error.message());
		Disconnect();
		return;
	}
	m_serializer.Read(m_command.buffer, m_command.total_length);
	m_command.buffer.Reset();
	

	if (m_command.total_length > 0)
	{
		if(m_command.total_length > m_command.buffer.m_size)
		{
			m_command = Command(m_command.total_length - sizeof(m_command.total_length));
		}

		// Read the body of the command
		boost::asio::async_read(m_socket, boost::asio::buffer(m_command.buffer.m_data, m_command.total_length - sizeof(m_command.total_length)),
			std::bind(&TcpConnection::HandleReadBody, shared_from_this(),
				std::placeholders::_1));
		return;
	}
	m_server->Log("Error while receiving body length is 0 or less");
	Disconnect();
}

void TcpConnection::HandleReadBody(const boost::system::error_code& error)
{
	if (error)
	{
		m_server->Log("Error on body read: " + error.message());
		Disconnect();
		return;
	}

	m_serializer.Read(m_command.buffer, m_command);
	m_command.buffer.Reset();
	HandleCommand();
	
	StartRead();
}

void TcpConnection::HandleCommand()
{
	m_server->Log("Handling command of type: " +std::to_string(static_cast<uint8_t>(m_command.type)));
	if((static_cast<uint8_t>(m_command.type) & static_cast<uint8_t>(m_acceptedTypes)) != 0)
		m_commandHandlers[m_command.type]();
	StartWrite();
}

void TcpConnection::StartWrite()
{
	if(m_isWriting || m_writeQueue.empty())
		return;

	m_isWriting = true;
	auto& data = m_writeQueue.front();
	m_serializer.Write(data.buffer, data);
	data.buffer.Reset();
	boost::asio::async_write(m_socket, boost::asio::buffer(data.buffer.m_data, data.total_length),
		std::bind(&TcpConnection::HandleWrite, shared_from_this(),
			std::placeholders::_1));
}

void TcpConnection::HandleWrite(const boost::system::error_code& error)
{
	if (!error)
	{
		m_writeQueue.pop_front();
		m_isWriting = false;
		if(!m_writeQueue.empty())
			StartWrite();
	}
	else
	{
		m_server->Log("Error on write: " + error.message());
		m_isWriting = false;
		Disconnect();
	}
}

void TcpConnection::Disconnect()
{
	m_server->Log("Disconnecting client");
	if (m_onDisconnect)
		m_onDisconnect(shared_from_this());
	m_socket.close();
}

void TcpConnection::HandleUsername()
{
	m_server->Log("Setting username: " + m_command.data);
	m_username = m_command.data;
}

void TcpConnection::HandleMessage()
{
	m_server->Log("Received message: " + m_command.data);
	m_command.data = m_username + ": " + m_command.data;
	Command c(m_command.type, m_command.data); 
	m_server->Broadcast(c);
	m_command = Command(512);
}

void TcpConnection::HandleRoomJoin()
{
	m_server->Log("Received request to join room: " + m_command.data);
	std::vector<std::string> rooms;
	m_server->GetRooms(rooms);
	if(std::ranges::find(rooms, m_command.data) != rooms.end())
	{
		Command response(CommandType::JoinRoom, m_command.data);
		m_writeQueue.push_back(std::move(response));
	}
	else
	{
		Command response(CommandType::JoinRoom, RoomError);
		m_writeQueue.push_back(std::move(response));
	}
}

void TcpConnection::HandleRoomCreate()
{
	m_server->Log("Received request to create room: " + m_command.data);
	if (!m_server->AddRoom(shared_from_this(), m_command.data))
	{
		Command response(CommandType::CreateRoom, RoomError);
		m_writeQueue.push_back(std::move(response));
		return;
	}
	Command response(CommandType::CreateRoom, m_command.data);
	m_writeQueue.push_back(std::move(response));
}

void TcpConnection::HandleRoomList()
{
	m_server->Log("Received request for room list.");
	std::vector<std::string> rooms;
	m_server->GetRooms(rooms);
	auto roomList = rooms | std::views::join_with(std::string(";")) | std::ranges::to<std::string>();
	if(roomList.empty())
		roomList = "No rooms available.";
	Command response(CommandType::ListRooms, roomList);
	m_writeQueue.push_back(std::move(response));
}


void TcpConnection::HandleDisconnect()
{
	Disconnect();
}

void TcpConnection::HandleImage()
{
	m_server->Broadcast(m_command);
	m_command = Command(512);
}

