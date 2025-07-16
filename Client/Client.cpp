#include "Client.h"
#include <iostream>
#include <thread>
#include <common/Command.h>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/connect.hpp>

#include "common/Image.h"
using boost::asio::ip::tcp;

Client::Client(std::string host, std::string port)
    : m_socket(m_ioContext), m_resolver(m_ioContext), m_host(host), m_port(port), m_command(512) {
}

void Client::Run()
{
    DoConnect();
    std::thread ioThread([this]() { m_ioContext.run(); });

    std::string line;
    while (std::getline(std::cin, line) && !m_isStopped) {
        if (!line.empty()) {
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                std::cout
                    << "\x1b[1A" // Move cursor up one
                    << "\x1b[2K"; // Delete the entire line
                std::cout << "\r";
            }
			Command c = ParseCommand(line);
            if (!m_server)
				SendCommand(c);
            else
            {
                if (c.type == CommandType::Username)
                    m_username = c.data;
                else
                {
                    m_server->Broadcast(c);
                }
            }
            
        }
    }
    ioThread.join();
}

void Client::DoConnect()
{
    auto endpoints = m_resolver.resolve(m_host, m_port);
    boost::asio::async_connect(m_socket, endpoints,
        [self = shared_from_this()](boost::system::error_code ec, auto) {
            if (!ec) {
                self->Log("Connected to server.");
                self->m_isConnected = true;
                self->DoReadHeader();
            }
            else {
                self->Log("Connect error: " +ec.message());
            }
        });
}

void Client::SendCommand(Command& cmd)
{
    m_serializer.Write(cmd.buffer, cmd);
    cmd.buffer.Reset();
    bool writeInProgress = !m_writeQueue.empty();
    m_writeQueue.push_back(cmd);
    if (!writeInProgress) {
        DoWrite();
    }
}


Command Client::ParseCommand(const std::string& line)
{
    if (line.empty())
        return {CommandType::Message, line};

    size_t spacePos = line.find(' ');
    std::string cmdStr = (spacePos == std::string::npos) ? line : line.substr(0, spacePos);
    std::string data = (spacePos == std::string::npos) ? "" : line.substr(spacePos + 1);
    if (data.empty())
    {
        data = " ";
    }

    CommandType type = CommandType::Message;
  
    if (cmdStr == "CreateRoom") type = CommandType::CreateRoom;
    else if (cmdStr == "JoinRoom") type = CommandType::JoinRoom;
    else if (cmdStr == "ListRooms") type = CommandType::ListRooms;
    else if (cmdStr == "Disconnect") type = CommandType::Disconnect;
    else if (cmdStr == "Username") type = CommandType::Username;
    else if (cmdStr == "Image")
    {
        type = CommandType::Image;
        data = Image::ParseImage(data);
    }
    else
    {
        type = CommandType::Message;
        if (m_isRoom)
        {
            data = m_username + ": " + data;
        }
    }
    return {type, data};
}

Client::~Client()
{
    m_isConnected = false;
    boost::system::error_code ec;
    m_socket.cancel(ec);
    m_socket.shutdown(tcp::socket::shutdown_both, ec);
    m_socket.close(ec);
    m_writeQueue.clear();
    m_isWriting = false;
    if (m_serverThread) {
        m_serverThread->join();
        delete m_serverThread;
        delete m_server;
	}
}

void Client::DoWrite()
{
    if (m_writeQueue.empty()) return;

    Command& cmd = m_writeQueue.front();
    m_isWriting = true;

    boost::asio::async_write(m_socket,
        boost::asio::buffer(cmd.buffer.m_data, cmd.total_length ),
        [self = shared_from_this()](boost::system::error_code ec, std::size_t) {
            if (!ec) {
                self->m_writeQueue.pop_front();
                self->m_isWriting = false;
                if (!self->m_writeQueue.empty()) {
                    self->DoWrite();
                }
            }
            else {
                self->Log("Write error: " + ec.message());
            }
        });
}

void Client::DoReadHeader()
{
    if(!m_isConnected) {
        Log("Not connected to server.");
        return;
	}
    boost::asio::async_read(m_socket,
        boost::asio::buffer(m_command.buffer.m_data, sizeof(m_command.total_length)),
        [self = shared_from_this()](boost::system::error_code ec, std::size_t) {
            if (!ec) {
                self->m_serializer.Read(self->m_command.buffer, self->m_command.total_length);
                self->m_command.buffer.Reset();

                if (self->m_command.total_length > self->m_command.buffer.m_size) {
					self->m_command = Command(self->m_command.total_length - sizeof(m_command.total_length));
                }

                self->DoReadBody();
            }
            else {
                self->Log("Header read error: " + ec.message());
            }
        });
}

void Client::DoReadBody()
{
    if (!m_isConnected) {
    	Log("Not connected to server.");
        return;
    }
    boost::asio::async_read(m_socket,
        boost::asio::buffer(m_command.buffer.m_data, m_command.total_length - sizeof(m_command.total_length)),
        [self = shared_from_this()](boost::system::error_code ec, std::size_t) {
            if (!ec) {
                self->m_serializer.Read(self->m_command.buffer, self->m_command);
                self->HandleCommand(self->m_command);
                self->DoReadHeader();  // Loop read
            }
            else {
                self->Log("Body read error: " + ec.message());
            }
        });
}

void Client::HandleCommand(Command command)
{
    if (command.type == CommandType::CreateRoom)
    {
        if (command.data != RoomError)
        {
			 Log("[Server]: Room created successfully.");
            m_isRoom = true;
            m_server = new RoomServer(m_mutex, (unsigned short)atoi(command.data.data()));
            m_server->SetOnWrite([this](Command c) {this->HandleCommand(c); });
            m_serverThread = new std::thread([this]() { m_server->Run(); });
        }
    }
    else if (command.type == CommandType::JoinRoom)
    {
        if (command.data != RoomError)
        {
            Log("[Server]: Joined room successfully.");
            m_isConnected = false;
            boost::system::error_code ec;
            m_socket.cancel(ec);
            m_socket.shutdown(tcp::socket::shutdown_both, ec);
            m_socket.close(ec);
            m_writeQueue.clear();
            m_isWriting = false;
            size_t pos = command.data.find(':');
            if (pos != std::string::npos) {
                m_host = command.data.substr(0, pos);
                m_port = command.data.substr(pos + 1);
            }
            DoConnect();
        }
    }
    else if (command.type == CommandType::ListRooms)
    {
        Log("[Server]: Available rooms: " + command.data);
	}
    else if (command.type == CommandType::Message)
    {
	     Log(command.data);
    }
    else if (command.type == CommandType::Disconnect)
    {
         Log("[Server]: Disconnected.");
         m_isStopped = true;
    }
    else if (command.type == CommandType::Image)
    {
        Image img = Image::UnParseImage(command.data);
        const char* grayscale_chars = "@%#*+=-:. ";
        size_t shades = strlen(grayscale_chars) - 1;

        for (int y = 0; y < img.height; y++) {
            for (int x = 0; x < img.width; x++) {
                int idx = (y * img.width + x) * 3;
                unsigned char r = img.data[idx + 0];
                unsigned char g = img.data[idx + 1];
                unsigned char b = img.data[idx + 2];

                unsigned char pixel = static_cast<unsigned char>(0.299 * r + 0.587 * g + 0.114 * b);

                size_t index = pixel * shades / 255;
                printf("%c", grayscale_chars[index]);
            }
            printf("\n");
        }

        
    }
}

void Client::Log(const std::string& message)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    std::cout << message << std::endl;
}

