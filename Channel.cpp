#include "Server.hpp"
#include <cstring>
#define JOIN "JOIN"

Channel::Channel(std::string str_name, std::string str_pass)
{
	this->name = str_name;
	this->_pass = str_pass;
	this->_user_limit = 0;
	this->_topic = "";
	this->message = "";
	this->_mode['i'] = 0;
	this->_mode['t'] = 0;
	this->_mode['k'] = 0;
	this->_mode['o'] = 0;
	this->_mode['l'] = 0;
    this->_user_limit = 0;
}

Channel::~Channel(void){}

int	Channel::getUserInfo(void){return (_user_limit);}

std::string	Channel::getTopic(void)
{
	return (_topic);
}

std::string	Channel::getPass(void)
{
	return (_pass);
}

std::map<char, int> Channel::getMode(void)
{
	return (_mode);
}


std::vector<User> Channel::getUsers(void)
{
	return (users);
}

std::vector<User> Channel::getOperators(void)
{
	return (operators);
}

std::string Channel::getName(void) const
{
	return (name);
}


void Channel::setUserInfo(int num)
{
	_user_limit = num;
}

void Channel::setTopic(std::string str)
{
	_topic = str;
}

void Channel::setPass(std::string str)
{
	_pass = str;
}

std::vector<Channel>::iterator Command::channel_exist(std::string channel)
{
	this->channel_it = Server::_channels.begin();
	while (this->channel_it != Server::_channels.end())
	{
		if (channel_it->getName() == channel)
			return channel_it;
		channel_it++;
	}
	return channel_it;
}

std::vector<User>::iterator Command::user_exist(std::string nick)
{
	this->user_it = Server::users.begin();
	while (this->user_it != Server::users.end())
	{
		if (user_it->_nickname == nick)
			return user_it;
		user_it++;
	}
	return user_it;
}
