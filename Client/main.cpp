#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include "common/Command.h"
#include "ser/Serializer.h"
#include "Client.h"

int main() {
	std::shared_ptr<Client> client = Client::Create("127.0.0.1", "12345");
	client->Run();
}
