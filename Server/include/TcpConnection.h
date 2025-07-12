#pragma once
#include <boost/asio.hpp>
#include <memory>
#include <deque>
#include <map>
#include "common/Command.h"
#include "ser/Serializer.h"

class Server;

typedef std::deque<Command> WriteQueue;

class TcpConnection : public std::enable_shared_from_this<TcpConnection>
{
public:
	typedef std::shared_ptr<TcpConnection> Ptr;

	static Ptr Create(Server* server, boost::asio::io_context& io_context, CommandType flag)
	{
		return Ptr(new TcpConnection(server, io_context, flag));
	}

	void Start();
	
	boost::asio::ip::tcp::socket& GetSocket();

	void SetOnDisconnect(std::function<void(Ptr)> callback);

	void AddCommand(const Command& command);
private:
	TcpConnection(Server* server, boost::asio::io_context& io_context, CommandType flag);

	void StartRead();
	void HandleReadHeader(const boost::system::error_code& error);
	void HandleReadBody(const boost::system::error_code& error);
	void HandleCommand();
	void StartWrite();
	void HandleWrite(const boost::system::error_code& error);
	void Disconnect();

	void HandleMessage();
	void HandleRoomJoin();
	void HandleRoomCreate();
	void HandleRoomList();
	void HandleDisconnect();
	boost::asio::ip::tcp::socket m_socket;
	Command m_command;
	WriteQueue m_writeQueue;
	bool m_isWriting = false;

	std::function<void(Ptr)> m_onDisconnect;

	ser::Serializer m_serializer;
	
	std::map<CommandType, std::function<void()>> m_commandHandlers;

	CommandType m_acceptedTypes;

	Server* m_server = nullptr;
};

