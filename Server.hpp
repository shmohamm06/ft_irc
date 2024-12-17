/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wyaseen <wyaseen@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/15 11:45:22 by abobylev          #+#    #+#             */
/*   Updated: 2024/12/17 11:25:28 by wyaseen          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <vector>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <fcntl.h>
#include <cerrno>
#include <iomanip>
#include <signal.h>
#include <stdint.h>
#include <unistd.h>
#include <cstdlib>
#include <algorithm>
#include <fstream>
#include "Channel.hpp"
#include "User.hpp"
#include "Commands.hpp"
#include "Extra.hpp"

// Constants
#define MAX_PORT UINT16_MAX
#define MAX_BUFFER 1024
#define MAX_HOST_NAME 512

// Foreground Colors
#define BRIGHT_YELLOW "\033[93m"
#define BRIGHT_RED "\033[91m"
#define BRIGHT_WHITE "\033[97m"
#define BRIGHT_BLUE "\033[94m"
#define BRIGHT_CYAN "\033[96m"
#define BLUE "\033[34m"
#define MAGENTA "\033[35m"
#define BOLD "\033[1m"
#define UNDERLINE "\033[4m"

// Effects of the text
#define SHINY "\033[5m"
#define SHADOW "\033[2m"
#define FRAMED "\033[51m"

// Background ColorS
#define BRIGHT_BRED "\033[101m"


// Reset
#define RESET "\033[0m"
#define BRIGHT_GREEN "\033[1;32m"
#define BRIGHT_MAGENTA "\033[1;35m"
#define RESET "\033[0m"




class Server
{
private:
	Server(void);

public:
	static const int MAX_CLIENTS = FD_SETSIZE;
	static const int BUFFER_SIZE = MAX_BUFFER;
	static std::string _password;
	static std::string _hostName;
	static std::string bufferStr;
	static char c_buffer[MAX_BUFFER];
	static char c_hostName[MAX_HOST_NAME];
	static int serverSocket;
	static int max_sd;
	static int sd;
	static int valread;
	static int _port;
	static int newSocket;
	static int curIndex;
	static int addrlen;
	static struct sockaddr_in address;
	static fd_set readfds;
	class ServerException : public std::exception
	{
		private:
			std::string _msg;
		public:
			ServerException(std::string msg) : _msg(msg) {}
			virtual ~ServerException() throw() {}
			virtual const char *what() const throw() { return _msg.c_str(); }
	};
	static std::vector<int> _fds;
	static std::vector<User> users; 
	static std::vector<Channel> _channels;
    static void handleClientMessages();
	static void openSocket(void);
	static void run(void);
	static void acceptConnection(void);
	static void handleClientDisconnection(size_t i);
	static bool isNicknameTaken(const std::string& nickname);
	static bool isUsernameTaken(const std::string& username);
	static void processCommand(int sd, const std::string& command);
};

void valid_arg(std::string a, std::string b, int c);
void Check(int ac);


#endif
