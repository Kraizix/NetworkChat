#include "Server/Server.h"
#include "common/Command.h"

int main() {
	Server server(12345, RoomFlags | CommandType::Disconnect);
	server.Run();
	return 0;
}