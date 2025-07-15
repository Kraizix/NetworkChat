#include "Client.h"
#include <iostream>
#include <thread>
#include <common/Command.h>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/connect.hpp>
using boost::asio::ip::tcp;

Client::Client(std::string host, std::string port)
    : m_socket(m_ioContext), m_resolver(m_ioContext), m_host(host), m_port(port), m_command(512) {
}

void Client::Run()
{
    DoConnect();
    std::thread ioThread([this]() { m_ioContext.run(); });

    std::string line;
    while (std::getline(std::cin, line)) {
        if (!line.empty()) {
			Command c = ParseCommand(line);
            if (!m_server)
				SendCommand(c);
            else
				m_server->Broadcast(c);
            std::cout
                << "\x1b[1A" // Move cursor up one
                << "\x1b[2K"; // Delete the entire line
            std::cout << "\r";
        }
    }

    m_socket.close();
    ioThread.join();
    if (m_serverThread) {
        m_serverThread->join();
        delete m_serverThread;
        delete m_server;
    };
}

void Client::DoConnect()
{
    auto endpoints = m_resolver.resolve(m_host, m_port);
    boost::asio::async_connect(m_socket, endpoints,
        [self = shared_from_this()](boost::system::error_code ec, auto) {
            if (!ec) {
                std::cout << "Connected to server.\n";
                self->DoReadHeader();
            }
            else {
                std::cerr << "Connect error: " << ec.message() << std::endl;
            }
        });
}

void Client::SendMessage(const std::string& message)
{
    Command cmd(CommandType::Message, message);
    m_serializer.Write(cmd.buffer, cmd);
    cmd.buffer.m_remaining = cmd.total_length + sizeof(cmd.total_length);
    cmd.buffer.m_data -= cmd.buffer.m_remaining;

    bool writeInProgress = !m_writeQueue.empty();
    m_writeQueue.push_back(std::move(cmd));
    if (!writeInProgress) {
        DoWrite();
    }
}

void Client::SendCommand(Command& cmd)
{
    m_serializer.Write(cmd.buffer, cmd);
    cmd.buffer.m_remaining = cmd.total_length + sizeof(cmd.total_length);
    cmd.buffer.m_data -= cmd.buffer.m_remaining;
    bool writeInProgress = !m_writeQueue.empty();
    m_writeQueue.push_back(cmd);
    if (!writeInProgress) {
        DoWrite();
    }
}

Command Client::ParseCommand(const std::string& line)
{
    if (line.empty() || line[0] != '/')
        return {CommandType::Message, line};

    size_t spacePos = line.find(' ');
    std::string cmdStr = (spacePos == std::string::npos) ? line.substr(1) : line.substr(1, spacePos - 1);
    std::string data = (spacePos == std::string::npos) ? "" : line.substr(spacePos + 1);

    CommandType type = CommandType::Message;
    if (cmdStr == "Message") type = CommandType::Message;
    else if (cmdStr == "CreateRoom") type = CommandType::CreateRoom;
    else if (cmdStr == "JoinRoom") type = CommandType::JoinRoom;
    else if (cmdStr == "ListRooms") type = CommandType::ListRooms;
    else if (cmdStr == "Disconnect") type = CommandType::Disconnect;
    else if (cmdStr == "Username") type = CommandType::Username;

    return {type, data};
}

void Client::DoWrite()
{
    if (m_writeQueue.empty()) return;

    Command& cmd = m_writeQueue.front();
    m_isWriting = true;

    boost::asio::async_write(m_socket,
        boost::asio::buffer(cmd.buffer.m_data, cmd.total_length + sizeof(cmd.total_length)),
        [self = shared_from_this()](boost::system::error_code ec, std::size_t) {
            if (!ec) {
                self->m_writeQueue.pop_front();
                self->m_isWriting = false;
                if (!self->m_writeQueue.empty()) {
                    self->DoWrite();
                }
            }
            else {
                std::cerr << "Write error: " << ec.message() << std::endl;
            }
        });
}

void Client::DoReadHeader()
{
    boost::asio::async_read(m_socket,
        boost::asio::buffer(m_command.buffer.m_data, sizeof(m_command.total_length)),
        [self = shared_from_this()](boost::system::error_code ec, std::size_t) {
            if (!ec) {
				std::cout << "Header read successfully." << std::endl;
                self->m_serializer.Read(self->m_command.buffer, self->m_command.total_length);
                self->m_command.buffer.Reset();

                if (self->m_command.total_length > self->m_command.buffer.m_size) {
					self->m_command = Command(self->m_command.total_length);
                }

                self->DoReadBody();
            }
            else {
                std::cerr << "Header read error: " << ec.message() << std::endl;
            }
        });
}

void Client::DoReadBody()
{
    boost::asio::async_read(m_socket,
        boost::asio::buffer(m_command.buffer.m_data, m_command.total_length),
        [self = shared_from_this()](boost::system::error_code ec, std::size_t) {
            if (!ec) {
				std::cout << "Body read successfully." << std::endl;
                self->m_serializer.Read(self->m_command.buffer, self->m_command);
                self->HandleCommand();
                self->DoReadHeader();  // Loop read
            }
            else {
                std::cerr << "Body read error: " << ec.message() << std::endl;
            }
        });
}

void Client::HandleCommand()
{
    std::cout << "[Server]: " << m_command.data << std::endl;
    if (m_command.type == CommandType::CreateRoom)
    {
        if (m_command.data == RoomOK)
        {
			std::cout << "[Server]: Room created successfully." << std::endl;
            m_isRoom = true;
            m_server = new Server(static_cast<boost::asio::ip::port_type>(12344), CommandType::Disconnect | CommandType::Message | CommandType::Username);
            m_serverThread = new std::thread([this]() { m_server->Run(); });
        }
    }
    else if (m_command.type == CommandType::JoinRoom)
    {
        if (m_command.data != RoomError)
        {
			std::cout << "[Server]: Joined room successfully." << std::endl;
            m_ioContext.stop();
            m_socket.close();

            size_t pos = m_command.data.find(':');
            if (pos != std::string::npos) {
                m_host = m_command.data.substr(0, pos);
                m_port = m_command.data.substr(pos + 1);
            }
            m_ioContext.restart();
            DoConnect();
        }
    }
    else if (m_command.type == CommandType::ListRooms)
    {
        std::cout << "[Server]: Available rooms: " << m_command.data << std::endl;
	}
    else if (m_command.type == CommandType::Message)
    {
	    std::cout << "[Message]: " << m_command.data << std::endl;
    }
    else if (m_command.type == CommandType::Disconnect)
    {
        std::cout << "[Server]: Disconnected." << std::endl;
       // TODO: Handle disconnection.
    }
	m_command = Command(512);
}
