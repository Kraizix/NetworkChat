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

	/// <summary>
	/// Start the connection to read commands from the client.
	/// </summary>
	void Start();

	boost::asio::ip::tcp::socket& GetSocket();

	/// <summary>
	/// Add callback to be called when the connection is disconnected.
	/// </summary>
	/// <param name="callback">Callback for disconnection</param>
	void SetOnDisconnect(std::function<void(Ptr)> callback);

	/// <summary>
	/// Add a command to the write queue to be sent to the client.
	/// </summary>
	/// <param name="command">Command to send</param>
	void AddCommand(const Command& command);
private:
	TcpConnection(Server* server, boost::asio::io_context& io_context, CommandType flag);

	/// <summary>
	/// Start reading commands from the client.
	/// </summary>
	void StartRead();

	/// <summary>
	/// Start reading the header of a command from the client.
	/// </summary>
	/// <param name="error">Asio error code</param>
	void HandleReadHeader(const boost::system::error_code& error);

	/// <summary>
	/// Start reading the body of a command from the client.
	/// </summary>
	/// <param name="error">Asio error code</param>
	void HandleReadBody(const boost::system::error_code& error);

	/// <summary>
	/// Handle the command received from the client.
	/// </summary>
	void HandleCommand();

	/// <summary>
	/// Start writing commands to the client.
	/// </summary>
	void StartWrite();

	/// <summary>
	/// Handle the completion of a write operation to the client.
	/// </summary>
	/// <param name="error">Asio error code</param>
	void HandleWrite(const boost::system::error_code& error);

	/// <summary>
	/// Disconnect the clients.
	/// </summary>
	void Disconnect();

	/// <summary>
	/// Handle username command.
	/// </summary>
	void HandleUsername();

	/// <summary>
	/// Handle message command.
	/// </summary>
	void HandleMessage();

	/// <summary>
	/// Handle room join command.
	/// </summary>
	void HandleRoomJoin();

	/// <summary>
	/// Handle room create command.
	/// </summary>
	void HandleRoomCreate();

	/// <summary>
	/// Handle room list command.
	/// </summary>
	void HandleRoomList();

	/// <summary>
	/// Handle disconnection command.
	/// </summary>
	void HandleDisconnect();

	/// <summary>
	/// Handle image command.
	/// </summary>
	void HandleImage();

	boost::asio::ip::tcp::socket m_socket;
	Command m_command;
	WriteQueue m_writeQueue;
	bool m_isWriting = false;
	std::function<void(Ptr)> m_onDisconnect;
	ser::Serializer m_serializer;
	std::map<CommandType, std::function<void()>> m_commandHandlers;
	std::string m_username;
	CommandType m_acceptedTypes;
	Server* m_server = nullptr;
};

