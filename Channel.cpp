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

time_t Channel::getCreationTime() const {
    return this->_creation_time;
}

std::vector<User>::iterator Channel::inv_in_chan(int fd)
{
	for (this->it_invites = this->invites.begin(); this->it_invites != this->invites.end(); it_invites++)
	{
		if (this->it_invites->_fd == fd)
			return (it_invites);
	}
	return (it_invites);
}

int Channel::user_length(void)
{
	int len = 0;

	for (it_user = users.begin(); it_user != users.end(); it_user++)
		len++;
	return (len);
}
void Channel::addUserToChannel(User user_object) {
    // If no operators exist, add the first user as an operator
    if (operators.empty()) {
        operators.push_back(User(user_object));
    }

    // Check for user limit if the channel is in limit mode
    if (this->isMode('l')) {
        if (this->user_length() == this->_user_limit) {
            ErrorMsg(user_object._fd, (this->getName() + " :Channel is full.\r\n"), "471");
            return;
        }
    }

    // Add the user to the channel's user list
    users.push_back(User(user_object));

    // Notify the channel that the user has joined
    std::string join_message = ":" + user_object._nickname + "!" + user_object._username + "@localhost JOIN " + this->getName() + "\r\n";
    for (std::vector<User>::iterator it = users.begin(); it != users.end(); ++it) {
        send(it->_fd, join_message.c_str(), join_message.length(), 0); // Send join message
    }

    // List of users in the channel
    std::string user_list = ":irc NOTICE " + this->getName() + " :[" + this->getName() + "] Users in channel: [";
    if (!operators.empty()) {
        user_list += "@" + operators[0].getNick(); // Add operator
    }
    for (std::vector<User>::iterator it = users.begin(); it != users.end(); ++it) {
        if (it != users.begin() || !operators.empty()) {
            user_list += " " + it->getNick();
        }
    }
    user_list += "]\r\n";

    // Send the user list to all users in the channel
    for (std::vector<User>::iterator it = users.begin(); it != users.end(); ++it) {
        send(it->_fd, user_list.c_str(), user_list.length(), 0); // Send user list as NOTICE
    }

    // Send a summary of nicks and status (ops, voices, etc.)
    std::string channel_summary = ":irc NOTICE " + this->getName() + " :-!- " + this->getName() + ": Total of " + std::to_string(users.size()) + " nicks [";
    channel_summary += std::to_string(operators.size()) + " ops, 0 halfops, 0 voices, " +
                       std::to_string(users.size() - operators.size()) + " normal]\r\n";
    for (std::vector<User>::iterator it = users.begin(); it != users.end(); ++it) {
        send(it->_fd, channel_summary.c_str(), channel_summary.length(), 0); // Send summary as NOTICE
    }

    // Send the creation time of the channel
    time_t creation_time = this->getCreationTime(); // Get the creation time by value
    std::ostringstream oss;
    oss << std::ctime(&creation_time); // Convert time_t to string
    std::string creation_time_str = oss.str();
    creation_time_str.erase(creation_time_str.end() - 1); // Remove the newline character
    std::string creation_msg = ":irc NOTICE " + this->getName() + " :-!- Channel " + this->getName() + " created " + creation_time_str + "\r\n";
    for (std::vector<User>::iterator it = users.begin(); it != users.end(); ++it) {
        send(it->_fd, creation_msg.c_str(), creation_msg.length(), 0); // Send creation message as NOTICE
    }

std::string channel_welcome_msg;

// Welcome message
channel_welcome_msg = BRIGHT_WHITE "Welcome to the \"" + this->getName() + "\" channel, " 
                       + user_object._nickname + "!" RESET "\r\n";
channel_welcome_msg += BRIGHT_BLUE "Below are some commands to help you get started:" RESET "\r\n";

// Command list
channel_welcome_msg += BRIGHT_CYAN " - JOIN <channel> [key]: Enter a channel (provide key if required)" RESET "\r\n";
channel_welcome_msg += BRIGHT_CYAN " - PART <channel>: Exit the channel gracefully" RESET "\r\n";
channel_welcome_msg += BRIGHT_CYAN " - MSG <user/channel> <message>: Send a private or public message" RESET "\r\n";
channel_welcome_msg += BRIGHT_CYAN " - NICK <new_nickname>: Update your nickname" RESET "\r\n";
channel_welcome_msg += BRIGHT_CYAN " - WHO <channel>: See who’s currently in the channel" RESET "\r\n";
channel_welcome_msg += BRIGHT_CYAN " - INVITE <channel> <user>: Bring another user to the channel" RESET "\r\n";
channel_welcome_msg += BRIGHT_CYAN " - KICK <user> <channel>: Remove a user from the channel" RESET "\r\n";
channel_welcome_msg += BRIGHT_CYAN " - TOPIC <channel> <topic>: Change or set the channel topic" RESET "\r\n";
channel_welcome_msg += BRIGHT_CYAN " - MODE <channel> <+/-mode> [parameters]: Adjust channel settings" RESET "\r\n";

    // Send the welcome message to the new user
    send(user_object._fd, channel_welcome_msg.c_str(), channel_welcome_msg.length(), 0);
}
int Channel::isMode(char m)
{
    std::map<char, int>::iterator it = this->_mode.find(m);
    if (it != this->_mode.end())
    {
        if (it->second == 1) // If mode is in '+' it is set to 1 status
            return 1;
        else if (it->second == 0) // If mode is in '-' it is set to 0 status (eg; MODE #channel -i)
            return 0;
    }
    return 2;
}
int Channel::isUser(User user)
{
	std::vector<User>::const_iterator it;
	for (it = this->users.begin(); it != this->users.end(); it++)
	{
		if (it->_nickname == user._nickname)
			return (1);
	}
	return (0);
}
int Channel::isInvited(User user)
{
	std::vector<User>::iterator it;
	for (it = this->invites.begin(); it != this->invites.end(); it++)
	{
		if (it->_nickname == user._nickname)
			return (1);
	}
	return (0);
}