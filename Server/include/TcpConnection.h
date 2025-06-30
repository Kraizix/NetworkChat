#pragma once
#include <boost/asio.hpp>
#include <memory>
#include <deque>
#include "common/Command.h"
#include "ser/Serializer.h"

typedef std::deque<Command> WriteQueue;

class TcpConnection : public std::enable_shared_from_this<TcpConnection>
{
public:
	typedef std::shared_ptr<TcpConnection> Ptr;

	static Ptr Create(boost::asio::io_context& io_context)
	{
		return Ptr(new TcpConnection(io_context));
	}

	void Start();
	
	boost::asio::ip::tcp::socket& GetSocket();

	void SetOnDisconnect(std::function<void(Ptr)> callback);

private:
	TcpConnection(boost::asio::io_context& io_context);

	void StartRead();
	void HandleReadHeader(const boost::system::error_code& error);
	void HandleReadBody(const boost::system::error_code& error);
	void HandleCommand();
	void StartWrite();
	void HandleWrite(const boost::system::error_code& error);
	void Disconnect();

	boost::asio::ip::tcp::socket m_socket;
	Command m_command;
	WriteQueue m_writeQueue;
	bool m_isWriting = false;

	std::function<void(Ptr)> m_onDisconnect;

	ser::Serializer m_serializer;
};

