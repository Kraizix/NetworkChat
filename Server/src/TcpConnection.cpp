#include "TcpConnection.h"

#include <iostream>
#include "ser/Buffer.h"
#include "ser/Serializer.h"

TcpConnection::TcpConnection(boost::asio::io_context& io_context) : m_socket(io_context), m_command(512)
{
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


void TcpConnection::StartRead()
{
	boost::asio::async_read(m_socket, boost::asio::buffer(m_command.buffer.m_data, sizeof(m_command.total_length)),
		std::bind(&TcpConnection::HandleReadHeader, shared_from_this(),
			boost::asio::placeholders::error));
}

void TcpConnection::HandleReadHeader(const boost::system::error_code& error)
{
	if (error)
	{
		std::cerr << "Error on header read: " << error.message() << std::endl;
		Disconnect();
		return;
	}

	m_serializer.Read(m_command.buffer, m_command.total_length);


	if (m_command.total_length > 0)
	{
		if(m_command.total_length < sizeof(m_command.buffer.m_data))
		{
			delete[] m_command.buffer.m_data;
			m_command.buffer.m_data = new uint8_t[m_command.total_length];
		}

		// Read the body of the command
		boost::asio::async_read(m_socket, boost::asio::buffer(m_command.buffer.m_data, m_command.total_length),
			std::bind(&TcpConnection::HandleReadBody, shared_from_this(),
				boost::asio::placeholders::error));
	}
	//TODO: Handle the case where total_length is 0 (no body to read).
}

void TcpConnection::HandleReadBody(const boost::system::error_code& error)
{
	if (error)
	{
		std::cerr << "Error on body read: " << error.message() << std::endl;
		Disconnect();
		return;
	}

	m_serializer.Read(m_command.buffer, m_command);

	HandleCommand();
	StartRead();
}

void TcpConnection::HandleCommand()
{
	// TODO: Implement command handling logic here.
	// StartWrite()
}

void TcpConnection::StartWrite()
{
	if(m_isWriting || m_writeQueue.empty())
		return;

	m_isWriting = true;
	auto& data = m_writeQueue.front();
	boost::asio::async_write(m_socket, boost::asio::buffer(data.buffer.m_data, data.total_length + sizeof(data.total_length)),
		std::bind(&TcpConnection::HandleWrite, shared_from_this(),
			boost::asio::placeholders::error));
}

void TcpConnection::HandleWrite(const boost::system::error_code& error)
{
	if (!error)
	{
		m_writeQueue.pop_front();
		m_isWriting = false;
		if(m_writeQueue.size() >= 1)
			StartWrite();
	}
	else
	{
		//TODO: Handle error properly.
		std::cerr << "Error on write: " << error.message() << std::endl;
		m_isWriting = false;
		Disconnect();
	}
}

void TcpConnection::Disconnect()
{
	if (m_onDisconnect)
		m_onDisconnect(shared_from_this());
	m_socket.close();
}