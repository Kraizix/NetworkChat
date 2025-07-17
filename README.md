# Network Chat

A C++ project using Boost.Asio for TCP communication between clients and servers.

This project implements a **main server** responsible for managing chat rooms (create, join, list). Each **client** connects to the main server to discover or join chat rooms. A client can also **host a chat room** itself, becoming a local server where other clients can connect to chat.

---

## Features

- Centralized **main server** for room management:
  - Create rooms
  - Join rooms
  - List available rooms
- Clients can:
  - Join existing rooms to chat with others
  - Create and host their own rooms (acts as a server)
  - Send messages and images
  - Set a username
  - Disconnect(work in progress)

---

## Requirements

- C++23
- [vcpkg](https://github.com/microsoft/vcpkg)
- Visual Studio 2022

---

## Build Instructions

1. Open the solution (`.sln`) file in Visual Studio 2022.
2. Build both the **Client** and **Server** projects.

---

## Usage

1. Start the **Main Server** by running `Server.exe`.
2. Start one or more **Clients** by running `Client.exe`.

Each client connects to the main server to manage room activities.

Main Server only handles room commands and disconnect. Rooms only handle message, username, image and disconnect commands

---

## Commands

| Command                 | Description                                                                                                                 |
| ----------------------- | --------------------------------------------------------------------------------------------------------------------------- |
| `CreateRoom port`       | Creates a new room on the specified port. The actual address is deduced automatically.                                      |
| `JoinRoom address:port` | Connects to a specified room if available.                                                                                  |
| `ListRooms`             | Lists all currently available rooms from the main server.                                                                   |
| `Message your_message`  | Sends a message to all connected users in the current room.                                                                 |
| `Username your_name`    | Sets your username.                                                                                                         |
| `Image path_to_image`   | Sends an image to other clients. Images must be RGB (3 channels) and under 60,000 total size (`width * height * channels`), 100x100 works. |
| `Disconnect`            | Disconnects from the current server (work in progress).                                                                     |

---

## Known Limitations

- Only basic disconnection handling is implemented (`/Disconnect` is a work-in-progress).
- Image size is limited to avoid memory issues.
- No port checking before opening Server/Clients

![](.\resources\Networkchat.gif)
