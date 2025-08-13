# ft_irc

## Overview
ft_irc is a custom Internet Relay Chat (IRC) server implementation that allows multiple clients to connect and communicate in real-time. This project demonstrates network programming, multi-client handling, protocol implementation, and server architecture design.

## Features
- **Multi-Client Support**: Handles multiple simultaneous client connections
- **Real-time Communication**: Instant message delivery between clients
- **Channel Management**: Create, join, and manage chat channels
- **User Authentication**: Basic user registration and identification
- **Message Broadcasting**: Send messages to channels or individual users
- **Network Protocol**: Implements IRC protocol standards
- **Server Administration**: Basic server management commands

## IRC Protocol Features
- **User Registration**: NICK, USER, and PASS commands
- **Channel Operations**: JOIN, PART, MODE, and TOPIC commands
- **Messaging**: PRIVMSG and NOTICE commands
- **Server Information**: VERSION, TIME, and INFO commands
- **User Management**: WHO, WHOIS, and WHOWAS commands

## How It Works
1. **Server Initialization**: Sets up network socket and listens for connections
2. **Client Connection**: Accepts new client connections
3. **Command Parsing**: Parses incoming IRC commands
4. **Message Routing**: Routes messages to appropriate recipients
5. **Channel Management**: Manages channel membership and operations
6. **Client Communication**: Maintains persistent connections

## Project Structure
```
ft_irc/
├── ft_irc.h                 # Header file with structures and declarations
├── main.c                   # Main server entry point
├── server.c                 # Server initialization and main loop
├── client.c                 # Client connection management
├── commands.c               # IRC command implementations
├── channels.c               # Channel management functions
├── messages.c               # Message handling and routing
├── network.c                # Network socket operations
├── utils.c                  # Utility functions
├── Makefile                 # Build configuration
└── .git/                   # Git repository
```

## Supported Commands
- **Connection**: PASS, NICK, USER, QUIT
- **Channel**: JOIN, PART, MODE, TOPIC, INVITE
- **Messaging**: PRIVMSG, NOTICE
- **Information**: WHO, WHOIS, WHOWAS, VERSION, TIME
- **Server**: PING, PONG, ERROR

## Usage
```bash
# Compile the server
make

# Run the IRC server
./ft_irc <port>

# Example: Run on port 6667
./ft_irc 6667

# Clean build files
make clean
```



## Requirements
- GCC compiler
- Make utility
- Standard C libraries
- Network socket libraries
- Libft library (dependency)

## Dependencies
This project depends on the Libft library for basic string and memory functions.

## Network Features
- **Socket Programming**: TCP socket implementation
- **Multi-Client Handling**: Select/poll for non-blocking I/O
- **Protocol Compliance**: IRC protocol standard implementation
- **Error Handling**: Robust network error management
- **Connection Management**: Persistent client connections

## Client Features
- **User Registration**: Nickname and username setup
- **Channel Participation**: Join and leave channels
- **Private Messaging**: Direct user-to-user communication
- **Channel Messaging**: Broadcast messages to channel members
- **User Information**: Query user details and status

## Security Features
- **Input Validation**: Sanitizes user input
- **Access Control**: Channel permissions and modes
- **Flood Protection**: Prevents message flooding
- **Connection Limits**: Maximum client connections

## Notes
- Follows 42 coding style (norminette)
- Implements robust network programming
- Handles multiple concurrent connections
- Memory-safe implementation
- Compliant with IRC protocol standards

## Author
shmohamm - 42 Abu Dhabi
