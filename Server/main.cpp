#include "Server.h"
#include "common/Command.h"

int main() {
	Server server(12345, RoomFlags | CommandType::Disconnect | CommandType::Message);
	server.Run();
	return 0;
}