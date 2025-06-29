#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include "common/Command.h"
#include "ser/Serializer.h"
using boost::asio::ip::tcp;

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: client <host> <port>" << std::endl;
        return 1;
    }

    try {
        boost::asio::io_context io_context;
        tcp::resolver resolver(io_context);
        auto endpoints = resolver.resolve(argv[1], argv[2]);
        tcp::socket socket(io_context);
        boost::asio::connect(socket, endpoints);

        std::cout << "Connected to server. Type messages to send.\n";

        std::string line;
        while (std::getline(std::cin, line)) {
            if (line.empty())
                continue;

            Command cmd(CommandType::Message,line);
            ser::Serializer s;
			s.Write(cmd.buffer, cmd);
			cmd.buffer.m_remaining = cmd.total_length + sizeof(cmd.total_length);
            cmd.buffer.m_data -= cmd.buffer.m_remaining;
            boost::asio::write(socket, boost::asio::buffer(cmd.buffer.m_data, cmd.total_length + sizeof(cmd.total_length)));
        }

        socket.close();
    }
    catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

}
