#pragma once
#include <string>
#include <deque>
#include <ser/Serializer.h>
#include <boost/asio.hpp>
#include "Server/Server.h"
#include "Common/Command.h"

class Client : public std::enable_shared_from_this<Client>
{
public:
    typedef std::shared_ptr<Client> Ptr;

	static Ptr Create(std::string host, std::string port)
	{
		return Ptr(new Client(host, port));
	}

	void Run();

    void SendMessage(const std::string& message);
	void SendCommand(Command& command);

    Command ParseCommand(const std::string& line);

private:
    void DoConnect();
    void DoReadHeader();
    void DoReadBody();
    void DoWrite();
    void HandleCommand();

    Client(std::string host, std::string port);

    boost::asio::io_context m_ioContext;
    boost::asio::ip::tcp::socket m_socket;
    boost::asio::ip::tcp::resolver m_resolver;

    std::string m_host;
    std::string m_port;

    Command m_command;
    ser::Serializer m_serializer;

    std::deque<Command> m_writeQueue;
    bool m_isWriting = false;
    bool m_isRoom = false;
    std::thread* m_serverThread = nullptr;
    Server* m_server = nullptr;
};

