#include "Command.hpp"
#include "Server.hpp"
#include "User.hpp"

Command::Command(void)
{
	// defualt constructor
	this->message = "";

}

Command::~Command(void)
{
	// destructor
}

void Command::who(std::string channel_s, User user) {
    // Find the channel
    std::vector<Channel>::iterator it = channel_exist(channel_s);
    if (it == Server::_channels.end()) {
        ErrorMsg(user._fd, "403 :Channel does not exist.\r\n", "403");
        return;
    }

    // Prepare the user list
    std::string userList = "NAMES " + it->getName() + " :";

    // Get the users in the channel
    std::vector<User> usersInChannel = it->getUsers();

    // Use a traditional for loop to append all user nicknames
    for (std::vector<User>::iterator it_user = usersInChannel.begin(); it_user != usersInChannel.end(); ++it_user) {
        userList += it_user->_nickname + " "; // Append each user's nickname
    }

    // Trim the trailing space
    if (!usersInChannel.empty()) {
        userList.erase(userList.length() - 1); // Remove the last space
    }

    // Add \r\n at the end of the message
    userList += "\r\n";

    // Send the user list to the user who requested it
    send(user._fd, userList.c_str(), userList.length(), 0);
}



void Command::mode(std::string channel_s, std::string mode, User user, std::string arg)
{
    std::vector<Channel>::iterator it_c;

    // Verify if mode string is valid and if mode is known
    if (mode.size() != 2 || (mode[0] != '+' && mode[0] != '-')) {
        ErrorMsg(user._fd, (mode + " :is unknown mode char to me.\r\n"), "472");
        return;
    }

    // Check if channel exists
    it_c = channel_exist(channel_s);
    if (it_c == Server::_channels.end()) {
        ErrorMsg(user._fd, (channel_s + " :No such channel.\r\n"), "403");
        return;
    }

    // Check if user is an operator
    if (!it_c->isOperator(user)) {
        ErrorMsg(user._fd, "Permission Denied - You're not an operator of the channel.\r\n", "482");
        return;
    }

    // Check if the mode character is valid
    if (it_c->isMode(mode[1]) == 2) {
        ErrorMsg(user._fd, (mode + " :is unknown mode char to me.\r\n"), "472");
        return;
    }

    // Execute mode change
    it_c->exec_mode(mode, user, arg);
}

// Pseudocode:
// Check if the channel exists.
// If the channel exists:
// Check if the channel is in 't' mode.
// If in 't' mode, check if the user is an operator.
// If the user is an operator, set or get the topic based on the provided topic string.
// If the user is not an operator, send an error message.
// If not in 't' mode, set or get the topic based on the provided topic string.
// If the channel does not exist, send an error message.
void Command::topic(std::string channel_s, std::string topic, User user)
{
    std::string serverName = "FT_IRC";  // Замените на реальное имя вашего сервера

    // Check if channel exists
    std::vector<Channel>::iterator it_c = channel_exist(channel_s);
    if (it_c != Server::_channels.end())
    {
        std::string reply;
        // Check if channel is in 't' mode
        if (it_c->isMode('t') == 1)
        {
            // Check if user is an operator
            if (it_c->isOperator(user))
            {
                if (topic.empty())
                {
                    // Get the topic
                    std::string currentTopic = it_c->getTopic();
                    // Send the topic to the user in IRC format
                    reply = ":" + serverName + " 332 " + user._nickname + " " + channel_s + " :" + currentTopic + "\r\n";
                    send(user._fd, reply.c_str(), reply.length(), 0);
                }
                else
                {
                    // Set the topic
                    it_c->setTopic(topic);
                    reply = ":" + serverName + " TOPIC " + channel_s + " :" + topic + "\r\n";
                    send(user._fd, reply.c_str(), reply.length(), 0);
                }
            }
            else
            {
                // Error: You're not an operator
                reply = ":" + serverName + " 482 " + user._nickname + " " + channel_s + " :You're not an operator of the channel.\r\n";
                send(user._fd, reply.c_str(), reply.length(), 0);
            }
        }
        else // if not in mode 't'
        {
            if (topic.empty())
            {
                // Get the topic
                std::string currentTopic = it_c->getTopic();
                // Send the topic to the user in IRC format
                reply = ":" + serverName + " 332 " + user._nickname + " " + channel_s + " :" + currentTopic + "\r\n";
                send(user._fd, reply.c_str(), reply.length(), 0);
            }
            else
            {
                // Set the topic
                it_c->setTopic(topic);
                reply = ":" + serverName + " TOPIC " + channel_s + " :" + topic + "\r\n";
                send(user._fd, reply.c_str(), reply.length(), 0);
            }
        }
    }
    else
    {
        // Error: No such channel
        std::string reply = ":" + serverName + " 403 " + user._nickname + " " + channel_s + " :No such channel.\r\n";
        send(user._fd, reply.c_str(), reply.length(), 0);
    }
}


std::vector<std::string> Command::ft_split(std::string str, char delimiter)
{
	std::vector<std::string> substrings;
	std::string substring = "";
	for (size_t i = 0; i < str.length(); i++)
	{
		if (str[i] != delimiter)
		{
			substring += str[i];
		}
		else
		{
			substrings.push_back(substring);
			while (str[i] == delimiter)
				i++;
			i--;
			substring = "";
		}
	}
	substrings.push_back(substring);
	return (substrings);
}
// Channel commands are in Channel.hpp
