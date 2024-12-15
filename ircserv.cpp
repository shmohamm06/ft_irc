/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ircserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abobylev <abobylev@student.42abudhabi.a    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/15 11:45:18 by abobylev          #+#    #+#             */
/*   Updated: 2024/12/15 11:46:15 by abobylev         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Channel.hpp"

int main(int ac, char **av)
{

    Check(ac);
    int port_num = std::atoi(av[1]);
	std::string	port(av[1]), password(av[2]);
	valid_arg(port, password, port_num);
	try
	{
		Server::_port = port_num;
		Server::_password = password;
		Server::openSocket();
		Server::run();
	} catch(const std::exception& e) {
		std::cerr << "Exception: " << e.what() << RESET << '\n';

	}
}