/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shmohamm <shmohamm@student.42abudhabi.a    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/15 11:45:12 by abobylev          #+#    #+#             */
/*   Updated: 2024/12/17 11:16:16 by shmohamm         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"
#include "Server.hpp"
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <unordered_map>

// Static Member Initialization: Initializes server variables for configuration
std::string Server::_password = ""; 
std::string Server::bufferStr = "";
std::string Server::_hostName = ""; 
char Server::c_buffer[BUFFER_SIZE] = {0};
char Server::c_hostName[MAX_HOST_NAME] = {0};
int Server::serverSocket = -1; 
int Server::max_sd = -1;
int Server::sd = -1; 
int Server::valread = -1; 
int Server::_port = -1; 
int Server::newSocket = -1;
int Server::curIndex = -1;
int Server::addrlen = sizeof(struct sockaddr_in);
std::vector<int> Server::_fds;
std::vector<User> Server::users;
struct sockaddr_in Server::address; 
fd_set Server::readfds;
std::vector<Channel> Server::_channels;
std::unordered_map<int, std::string> partialCommands;

// Error checks
void	Check(int ac)
{
	if (ac != 3)
	{
		std::cerr << RED << "Usage: ./ircserv [port] [PASS]" << RESET << std::endl;
		exit(0);
	}
}
void	valid_arg(std::string a, std::string b, int c)
{
	if (a.empty() || b.empty() || c > MAX_PORT || a.length() > 5 || a.find_first_not_of("0123456789") != std::string::npos)
	{
		std::cerr << RED << "Error: invalid arguments !" << RESET << std::endl;
		exit(0);
	}
}

// Socket Setup: Creates and configures the server socket
void Server::openSocket()
{
	int	opt;

	if ((Server::serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
		throw ServerException(RED "Failed to create socket" RESET);
	}
	opt = 1;
	if (setsockopt(Server::serverSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,
			sizeof(opt)) < 0)
	{
		throw ServerException(RED "setsockopt failed" RESET);
	}
	Server::address.sin_family = AF_INET;
	Server::address.sin_addr.s_addr = INADDR_ANY;
	Server::address.sin_port = htons(_port);
	if (bind(Server::serverSocket, (struct sockaddr *)&Server::address,
			sizeof(Server::address)) < 0)
	{
		throw ServerException(RED "Bind failed" RESET);
	}
	if (listen(Server::serverSocket, MAX_CLIENTS) < 0)
	{
		throw ServerException(RED "Listen failed" RESET);
	}
	addrlen = sizeof(Server::address);
	gethostname(c_hostName, MAX_HOST_NAME);
	Server::_hostName = c_hostName;
	std::cout << UNDERLINE << GREEN << BOLD << "IRC Server started on port " << _port << " : " << _hostName << RESET << std::endl;
	std::cout << BLUE << BOLD << "Waiting for incoming connections..." << RESET << std::endl;
}

// Accepting Connections: Handles new client connections.
// This function call will block until a client connects, at which point it
// returns a new socket descriptor for the connection, allowing you to communicate with that client.
void Server::acceptConnection()
{
    int newSocket = accept(serverSocket, (struct sockaddr*)&address, (socklen_t*)&addrlen);

    if (newSocket < 0) {
        std::cerr << "Failed to accept connection" << std::endl;
        return;
    }

    User newUser;
    newUser._fd = newSocket; 
    newUser._nickname = ""; 

    users.push_back(newUser);
    _fds.push_back(newSocket); 
}

// Client Disconnection: Safely removes disconnected clients
void Server::handleClientDisconnection(size_t i)
{
	getpeername(sd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
	std::cout << YELLOW << "Client disconnected" << RESET << std::endl;
	close(sd);
	_fds.erase(_fds.begin() + i);   
	users.erase(users.begin() + i); 
}


// User Commands Execution: Processes IRC commands like JOIN, WHO, etc
void User::execute(std::string mes, User *user) {
    std::vector<std::string> splitmsg = split(mes);
    if (splitmsg.empty()) {
        return;
    }

    Command cmd;
    std::string cmdType = splitmsg.at(0);
    if (cmdType == "JOIN") {
        if (splitmsg.size() == 2) {
            cmd.ajoin(splitmsg.at(1), "", *user);
        } else if (splitmsg.size() == 3) {
            cmd.ajoin(splitmsg.at(1), splitmsg.at(2), *user);
        } else {
            return;
        }
    } else if (cmdType == "WHO") {
        if (splitmsg.size() == 2) {
            cmd.who(splitmsg.at(1), *user);
        } else {
            return;
        }
    } else if (cmdType == "PRIVMSG" || cmdType == "MSG") {
        if (splitmsg.size() >= 3) {
            cmd.privmsg(splitmsg.at(1), splitmsg, *user);
        } else if (splitmsg.size() == 2) {
            ErrorMsg(user->_fd, "411 :No recipient given\r\n", "411");
        } else if (splitmsg.size() == 1) {
            ErrorMsg(user->_fd, "411 :No recipient given\r\n", "411");
        } else {
            std::string S = "461 :Not enough parameters\r\n";
            send(user->_fd, S.c_str(), strlen(S.c_str()), 0);
            return;
        }
    } else if (cmdType == "PING") {
        std::string pong = "PONG\r\n";
        send(user->_fd, pong.c_str(), pong.length(), 0);
    } else if (cmdType == "INVITE") {
        if (splitmsg.size() == 3) {
            cmd.invite(splitmsg.at(1), splitmsg.at(2), *user);
        } else {
            ErrorMsg(user->_fd, "461 :INVITE command requires 3 arguments\r\n", "461");
        }
    } else if (cmdType == "KICK") {
        if (splitmsg.size() >= 3) {
            cmd.kick(splitmsg.at(1), splitmsg.at(2), splitmsg, *user);
        } else {
            ErrorMsg(user->_fd, "461 :KICK command requires 3 or 4 arguments\r\n", "461");
        }
    } else if (cmdType == "MODE") {
        if (splitmsg.size() == 4) {
            cmd.mode(splitmsg.at(1), splitmsg.at(2), *user, splitmsg.at(3));
        } else if (splitmsg.size() == 3) {
            cmd.mode(splitmsg.at(1), splitmsg.at(2), *user, "");
        } else {
            ErrorMsg(user->_fd, "461 :Not enough parameters\r\n", "461");
        }
    }
    else if (cmdType == "TOPIC")
	{
		if (splitmsg.size() == 3)
		{
			cmd.topic(splitmsg.at(1), splitmsg.at(2), *user);
		}
		else if (splitmsg.size() == 2)
		{
			cmd.topic(splitmsg.at(1), "", *user);
		}
		else
		{
			ErrorMsg(user->_fd, "TOPIC command requires 1 or 2 arguments\n", "461");
		}
	}
    else if(cmdType == "CAP")
    {
        std::string firstServerMsg = ":irc CAP * ACK multi-prefix\r\n";
        send(user->_fd, firstServerMsg.c_str(), firstServerMsg.length(), 0);
    }
    else if (cmdType != "NICK" && cmdType != "PASS" && cmdType != "USER" && cmdType != "CAP") {
        ErrorMsg(user->_fd, "421 :Unknown command\r\n", "421");
    }
    else if(cmdType == "NICK" || cmdType == "PASS" || cmdType == "USER")
    {
        if (user->cap == 0)
        {
        std::string wlcmMsg = ":irc 001 " + user->_nickname + " :Welcome to FT_IRC, " + user->_username + "@" + Server::_hostName + "\r\n"
                      ":irc 002 " + user->_nickname + " :Your host is " + Server::_hostName + ", running version 1.0\r\n"
                      ":irc 003 " + user->_nickname + " :This server was created by Wahab, Shah & Alex\r\n"
                      ":irc 004 " + user->_nickname + " " + Server::_hostName + " 1.0 o OIRSv o oirwkl bkl\r\n"  // Sample modes
                      ":irc 005 " + user->_nickname + " MODES=20 CHANLIMIT=#&:100 NICKLEN=9 TOPICLEN=390 CHANTYPES=#& :are supported by this server\r\n";
        send(user->_fd, wlcmMsg.c_str(), wlcmMsg.length(), 0);
            user->cap = 1;
        };
    }
}

void printSplitMsg(const std::vector<std::string>& splitmsg) {
    std::cout << "Split Message:" << std::endl;
    for (size_t i = 0; i < splitmsg.size(); ++i) {
        std::cout << splitmsg[i] << "," << std::endl;
    }
}

void Server::handleClientMessages() {
    for (size_t i = 0; i < _fds.size(); i++) {
        sd = _fds[i];
        if (FD_ISSET(sd, &readfds)) {
            if ((valread = read(sd, c_buffer, BUFFER_SIZE)) == 0) {
                handleClientDisconnection(i);
            } else {
                c_buffer[valread] = '\0'; 

                partialCommands[sd] += c_buffer;

                size_t pos;
                while ((pos = partialCommands[sd].find('\n')) != std::string::npos) {
                    std::string command = partialCommands[sd].substr(0, pos);
                    partialCommands[sd] = partialCommands[sd].substr(pos + 1);

                    if (!command.empty() && command.back() == '\r') {
                        command.pop_back();
                    }

                    processCommand(sd, command);
                }
            }
        }
    }
}

// Processes a client's command and handles IRC-specific logic for NICK, USER, and PASS
void Server::processCommand(int sd, const std::string& command) {
    std::vector<std::string> splitmsg = split(command);

    if (splitmsg.empty()) {
        return;
    }

    size_t userIndex = std::distance(_fds.begin(), std::find(_fds.begin(), _fds.end(), sd));

    if (splitmsg[0] == "NICK" && splitmsg.size() > 1) {
        std::string nick = splitmsg[1];
        if (isNicknameTaken(nick)) {
            std::string errorMsg = ":irc 433 " + nick + " :Nickname is already in use\r\n";
            send(sd, errorMsg.c_str(), errorMsg.length(), 0);
            handleClientDisconnection(userIndex);
            return;
        }
        users[userIndex]._nickname = nick;
        users[userIndex].nick_flag = 1;
    } else if (splitmsg[0] == "USER" && splitmsg.size() > 1) {
        std::string user = splitmsg[1];
        if (isUsernameTaken(user)) {
            std::string errorMsg = ":irc 433 " + user + " :Username is already in use\r\n";
            send(sd, errorMsg.c_str(), errorMsg.length(), 0);
            handleClientDisconnection(userIndex);
            return;
        }
        users[userIndex].setUser(user);
        users[userIndex].user_flag = 1;
    } else if (splitmsg[0] == "PASS" && splitmsg.size() > 1) {
        std::string pass = splitmsg[1];
        users[userIndex]._password = pass;
        users[userIndex].pass_flag = 1;
        if (users[userIndex]._password != Server::_password) {
            std::string errorMsg = ":irc 464 " + users[userIndex]._nickname + " :Wrong password\r\n";
            send(sd, errorMsg.c_str(), errorMsg.length(), 0);
            handleClientDisconnection(userIndex);
            return;
        }
    }

    if (users[userIndex].pass_flag == 1 && users[userIndex].nick_flag == 1 && users[userIndex].user_flag == 1) {
        users[userIndex].execute(command, &users[userIndex]);
    }
}

bool Server::isNicknameTaken(const std::string &nick) {
    for (size_t i = 0; i < users.size(); ++i) {
        if (users[i]._nickname == nick) {
            return true;
        }
    }
    return false;
}

bool Server::isUsernameTaken(const std::string &user) {
    for (size_t i = 0; i < users.size(); ++i) {
        if (users[i]._username == user) {
            return true;
        }
    }
    return false;
}

// Main server loop that monitors connections, accepts new clients, and handles messages
void Server::run()
{
    while (true) {
        FD_ZERO(&readfds);
        FD_SET(serverSocket, &readfds);
        max_sd = serverSocket;
        for (size_t i = 0; i < _fds.size(); i++) {
            sd = _fds[i];
            if (sd > 0) {
                FD_SET(sd, &readfds);
            }
            if (sd > max_sd) {
                max_sd = sd;
            }
        }
        int activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
        if ((activity < 0) && (errno != EINTR)) {
            std::cerr << "Select error" << std::endl;
        }
        if (FD_ISSET(serverSocket, &readfds)) {
            acceptConnection();
            std::cout << GREEN << "Client connected" << RESET << std::endl;
        }
        handleClientMessages();
    }
}
