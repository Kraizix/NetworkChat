#pragma once
#include <string>
#include <deque>
#include <ser/Serializer.h>
#include <boost/asio.hpp>
#include "Common/Command.h"
#include "include/RoomServer.h"

class Client : public std::enable_shared_from_this<Client>
{
public:
    typedef std::shared_ptr<Client> Ptr;

	static Ptr Create(std::string host, std::string port)
	{
		return Ptr(new Client(host, port));
	}

	void Run();
	void SendCommand(Command& command);
    Command ParseCommand(const std::string& line);

    ~Client();

private:
    /// <summary>
	/// Handles the connection to the server.
    /// </summary>
    void DoConnect();

    /// <summary>
	/// Handles reading the header of the command.
    /// </summary>
    void DoReadHeader();

    /// <summary>
	/// Handles reading the body of the command.
    /// </summary>
    void DoReadBody();

    /// <summary>
	/// Handles writing the command to the server.
    /// </summary>
    void DoWrite();

    /// <summary>
	/// Handles the command received from the server.
    /// </summary>
    /// <param name="command">Command received</param>
    void HandleCommand(Command command);

	/// <summary>
	/// Log a message to the console.
	/// </summary>
	/// <param name="message">Message to log</param>
	void Log(const std::string& message);

    Client(std::string host, std::string port);

    boost::asio::io_context m_ioContext;
    boost::asio::ip::tcp::socket m_socket;
    boost::asio::ip::tcp::resolver m_resolver;
    std::string m_host;
    std::string m_port;
    Command m_command;
    ser::Serializer m_serializer;
   
    

    std::deque<Command> m_writeQueue;
    bool m_isConnected = false;
    bool m_isWriting = false;
    bool m_isRoom = false;
    bool m_isStopped = false;
    std::thread* m_serverThread = nullptr;
    RoomServer* m_server = nullptr;
	std::mutex m_mutex;
    std::string m_username = "Guest";
};

