#include "Commands.hpp"
#include "Server.hpp"
#include "User.hpp"

Command::Command(void)
{
	this->message = "";
}

Command::~Command(void)
{
}

// Retrieves and sends a list of users present in the specified channel to the requesting user.
void Command::who(std::string channelName, User requestingUser) {
    std::vector<Channel>::iterator channelIt = channel_exist(channelName);
    if (channelIt == Server::_channels.end()) {
        ErrorMsg(requestingUser._fd, "403 :The specified channel does not exist.\r\n", "403");
        return;
    }

    std::string userList = "NAMES " + channelIt->getName() + " :";
    std::vector<User> channelUsers = channelIt->getUsers();

    for (std::vector<User>::iterator userIt = channelUsers.begin(); userIt != channelUsers.end(); ++userIt) {
        userList += userIt->_nickname + " ";
    }

    if (!channelUsers.empty()) {
        userList.erase(userList.length() - 1);
    }

    userList += "\r\n";
    send(requestingUser._fd, userList.c_str(), userList.length(), 0);
}

//  Handles changing channel modes based on the provided mode flag and argument.
void Command::mode(std::string channelName, std::string modeFlag, User requestingUser, std::string modeArgument)
{
    std::vector<Channel>::iterator channelIt;

    if (modeFlag.size() != 2 || (modeFlag[0] != '+' && modeFlag[0] != '-')) {
        ErrorMsg(requestingUser._fd, (modeFlag + " :Invalid mode format. For eg: use '+x' or '-x'.\r\n"), "472");
        return;
    }

    channelIt = channel_exist(channelName);
    if (channelIt == Server::_channels.end()) {
        ErrorMsg(requestingUser._fd, (channelName + " :Channel not found.\r\n"), "403");
        return;
    }

    if (!channelIt->isOperator(requestingUser)) {
        ErrorMsg(requestingUser._fd, "482 :Access denied. You must be a channel operator.\r\n", "482");
        return;
    }

    if (channelIt->isMode(modeFlag[1]) == 2) {
        ErrorMsg(requestingUser._fd, (modeFlag + " :Unknown mode character.\r\n"), "472");
        return;
    }

    channelIt->exec_mode(modeFlag, requestingUser, modeArgument);
}

// Retrieves or updates the topic of a channel based on its mode and the user's privileges.
void Command::topic(std::string channelName, std::string newTopic, User requestingUser)
{
    std::string serverName = "FT_IRC";
    std::vector<Channel>::iterator channelIt = channel_exist(channelName);

    if (channelIt != Server::_channels.end()) {
        std::string responseMessage;

        if (channelIt->isMode('t') == 1) {
            if (channelIt->isOperator(requestingUser)) {
                if (newTopic.empty()) {
                    std::string currentTopic = channelIt->getTopic();
                    responseMessage = ":" + serverName + " 332 " + requestingUser._nickname + " " + channelName + " :" + currentTopic + "\r\n";
                    send(requestingUser._fd, responseMessage.c_str(), responseMessage.length(), 0);
                } else {
                    channelIt->setTopic(newTopic);
                    responseMessage = ":" + serverName + " TOPIC " + channelName + " :" + newTopic + "\r\n";
                    send(requestingUser._fd, responseMessage.c_str(), responseMessage.length(), 0);
                }
            } else {
                responseMessage = ":" + serverName + " 482 " + requestingUser._nickname + " " + channelName + " :You lack operator privileges for this channel.\r\n";
                send(requestingUser._fd, responseMessage.c_str(), responseMessage.length(), 0);
            }
        } else {
            if (newTopic.empty()) {
                std::string currentTopic = channelIt->getTopic();
                responseMessage = ":" + serverName + " 332 " + requestingUser._nickname + " " + channelName + " :" + currentTopic + "\r\n";
                send(requestingUser._fd, responseMessage.c_str(), responseMessage.length(), 0);
            } else {
                channelIt->setTopic(newTopic);
                responseMessage = ":" + serverName + " TOPIC " + channelName + " :" + newTopic + "\r\n";
                send(requestingUser._fd, responseMessage.c_str(), responseMessage.length(), 0);
            }
        }
    } else {
        std::string responseMessage = ":" + serverName + " 403 " + requestingUser._nickname + " " + channelName + " :The specified channel does not exist.\r\n";
        send(requestingUser._fd, responseMessage.c_str(), responseMessage.length(), 0);
    }
}

/**
 * Handles a user attempting to join a channel. If the channel doesn't exist,
 * it is created. This function also enforces key (password) and invite-only
 * restrictions, if applicable.
 */
void Command::ajoin(std::string channelName, std::string channelKey, User user) {
    std::vector<Channel>::iterator channelIt;
    std::vector<User>::iterator userIt;

    // Validate the channel name
    if (channelName.at(0) != '#' && channelName.at(0) != '&') {
        send(user._fd, "401 :Invalid channel name. Channels must start with '#' or '&'.\r\n", 
             strlen("401 :Invalid channel name. Channels must start with '#' or '&'.\r\n"), 0);
        return;
    } else if (channelName.size() <= 1) {
        send(user._fd, "401 :Channel name must contain more than one character.\r\n", 
             strlen("401 :Channel name must contain more than one character.\r\n"), 0);
        return;
    }

    // Check if the channel exists
    channelIt = channel_exist(channelName);
    if (channelIt != Server::_channels.end()) {
        // Check if the user is already in the channel
        if (channelIt->isUser(user)) {
            ErrorMsg(user._fd, "443 :" + user._nickname + " " + channelIt->getName() + " :You are already in the channel.\r\n", "443");
            return;
        }

        // Handle channels with a key (password)
        if (channelIt->isMode('k') == 1) {
            if (!channelKey.empty()) { // Key provided
                if (channelKey == channelIt->getPass()) { // Correct key
                    if (channelIt->isMode('i') == 1) { // Check invite-only mode
                        if (channelIt->isInvited(user)) {
                            userIt = channelIt->inv_in_chan(user._fd);
                            if (userIt != channelIt->invites.end())
                                channelIt->invites.erase(userIt);
                            channelIt->addUserToChannel(user);
                        } else {
                            ErrorMsg(user._fd, "473 :" + channelIt->getName() + " :Channel is invite-only.\r\n", "473");
                            return;
                        }
                    } else {
                        channelIt->addUserToChannel(user);
                    }
                } else {
                    ErrorMsg(user._fd, "475 :" + channelIt->getName() + " :Incorrect channel key.\r\n", "475");
                    return;
                }
            } else {
                ErrorMsg(user._fd, "475 :" + channelIt->getName() + " :This channel requires a key to join.\r\n", "475");
                return;
            }
        } else { // No key required
            if (channelIt->isMode('i') == 1) { // Check invite-only mode
                if (channelIt->isInvited(user)) {
                    userIt = channelIt->inv_in_chan(user._fd);
                    if (userIt != channelIt->invites.end())
                        channelIt->invites.erase(userIt);
                    channelIt->addUserToChannel(user);
                } else {
                    ErrorMsg(user._fd, "473 :" + channelIt->getName() + " :Channel is invite-only.\r\n", "473");
                    return;
                }
            } else {
                channelIt->addUserToChannel(user);
            }
        }
    } else {
        // If the channel does not exist, create a new one and add the user
        Channel newChannel(channelName, channelKey);
        newChannel.addUserToChannel(user);
        Server::_channels.push_back(newChannel);
    }
}

std::vector<std::string> Command::ft_split(std::string inputStr, char delimiter)
{
	std::vector<std::string> splitResult;
	std::string currentSubstring = "";

	for (size_t charIndex = 0; charIndex < inputStr.length(); charIndex++)
	{
		if (inputStr[charIndex] != delimiter)
		{
			currentSubstring += inputStr[charIndex];
		}
		else
		{
			splitResult.push_back(currentSubstring);
			while (inputStr[charIndex] == delimiter)
				charIndex++;
			charIndex--;
			currentSubstring = "";
		}
	}

	splitResult.push_back(currentSubstring);
	return (splitResult);
}
