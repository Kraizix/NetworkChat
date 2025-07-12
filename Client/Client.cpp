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
            Client::SendMessage(line);
        }
    }

    m_socket.close();
    ioThread.join();
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
                self->m_serializer.Read(self->m_command.buffer, self->m_command.total_length);

                if (self->m_command.total_length > sizeof(self->m_command.buffer.m_data)) {
                    delete[] self->m_command.buffer.m_data;
                    self->m_command.buffer.m_data = new uint8_t[self->m_command.total_length];
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
	//Refactor this to handle different command types if needed
    std::string message = "";
    m_serializer.Read(m_command.buffer, message);
    std::cout << "[Server]: " << message << std::endl;
}
