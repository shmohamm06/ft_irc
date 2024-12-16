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

/**
 * Sends a private message to a specified user or channel.
 * Handles user-to-user and user-to-channel messaging, with appropriate error checks.
 */
void Command::privmsg(std::string receiver, const std::vector<std::string>& splitmsg, User user) {
    std::vector<Channel>::iterator channelIter;
    std::vector<User>::iterator userIter;
    unsigned long messageIndex = 2;
    std::string fullMessage;

    // Construct the full message from splitmsg
    for (size_t j = 2; j < splitmsg.size(); ++j) {
        fullMessage += splitmsg[j] + (j < splitmsg.size() - 1 ? " " : "");
    }

    // Check for profanity
    if (processMessageWithProfanityCheck(user._fd, fullMessage) == 1) {
        channelIter = channel_exist(receiver);
        if (channelIter != Server::_channels.end()) {
            // Broadcast a message to the channel about the profanity
            std::string msg = ":" + user._nickname + " :Message by user cannot be displayed due to profanity.\r\n";
            std::vector<User> channelUsers = channelIter->getUsers();
            for (std::vector<User>::iterator it = channelUsers.begin(); it != channelUsers.end(); ++it) {
                send(it->_fd, msg.c_str(), msg.length(), 0);
            }
        }
        return;
    }

    userIter = user_exist(receiver);
    if (userIter == Server::users.end()) {
        channelIter = channel_exist(receiver);
        if (channelIter != Server::_channels.end()) {
            if (channelIter->isUser(user)) {
                std::vector<User> channelUsers = channelIter->getUsers();
                for (std::vector<User>::iterator it = channelUsers.begin(); it != channelUsers.end(); ++it) {
                    if (it->_fd != user._fd) {
                        std::string msg = ":" + user._nickname + " CHANNEL-MSG " + receiver + " :";
                        send(it->_fd, msg.c_str(), msg.length(), 0);
                        while (messageIndex < splitmsg.size()) {
                            send(it->_fd, splitmsg.at(messageIndex).c_str(), strlen(splitmsg.at(messageIndex).c_str()),
                                 0);
                            if (messageIndex + 1 < splitmsg.size()) {
                                send(it->_fd, " ", 1, 0);
                            }
                            messageIndex++;
                        }
                        send(it->_fd, "\r\n", 2, 0);
                        messageIndex = 2;
                    }
                }
            } else {
                ErrorMsg(user._fd, (channelIter->getName() + " :You are not part of the channel.\r\n"), "404");
            }
        }
    } else {
        if (user._fd == userIter->_fd) {
            send(userIter->_fd, "Cannot send message to yourself.\r\n",
                 strlen("Cannot send message to yourself.\r\n"), 0);
        } else {
            std::string msg = ":" + user._nickname + " PRIVMSG " + receiver + " :";
            send(userIter->_fd, msg.c_str(), msg.length(), 0);
            while (messageIndex < splitmsg.size()) {
                send(userIter->_fd, splitmsg.at(messageIndex).c_str(), strlen(splitmsg.at(messageIndex).c_str()), 0);
                if (messageIndex + 1 < splitmsg.size()) {
                    send(userIter->_fd, " ", 1, 0);
                }
                messageIndex++;
            }
            send(userIter->_fd, "\r\n", 2, 0);
            messageIndex = 2;
        }
    }
}


/**
 * Invites a user to join a specified channel, ensuring proper permissions and channel mode settings.
 */
void Command::invite(std::string userName, std::string channelName, User inviter) {
    std::vector<Channel>::iterator channelIter = channel_exist(channelName);
    std::vector<User>::iterator inviteeIter = user_exist(userName);

    if (channelIter != Server::_channels.end()) {
        if (inviteeIter != Server::users.end()) {
            if (channelIter->isOperator(inviter) != 1) {
                ErrorMsg(inviter._fd, "You are not an operator.\r\n", "482");
            } else {
                if (channelIter->isUser(*inviteeIter)) {
                    ErrorMsg(inviter._fd, "User is already in the channel.\r\n", "443");
                } else {
                    if (channelIter->isMode('i') == 1) {
                        if (channelIter->isInvited(*inviteeIter)) {
                            send(inviter._fd, "User is already invited.\r\n", strlen("User is already invited.\r\n"), 0);
                        } else {
                            std::string inviteMessage = "You're invited to the channel " + channelName + "\r\n";
                            send(inviteeIter->_fd, inviteMessage.c_str(), inviteMessage.length(), 0);
                            channelIter->invites.push_back(*inviteeIter);

                            std::string confirmationMessage = "Invite was successfully sent to " + userName + " for the channel " + channelName + "\r\n";
                            send(inviter._fd, confirmationMessage.c_str(), confirmationMessage.length(), 0);
                        }
                    } else {
                        send(inviter._fd, "Channel is not in +i mode.\r\n", strlen("Channel is not in +i mode.\r\n"), 0);
                    }
                }
            }
        } else {
            ErrorMsg(inviter._fd, "Invalid nickname.\r\n", "401");
        }
    } else {
        ErrorMsg(inviter._fd, "Invalid channel.\r\n", "403");
    }
}

/**
 * Handles kicking a user from a channel. Verifies that the requesting user is an 
 * operator, ensures the target user is part of the channel, and removes the target 
 * user from the channel's user and operator lists. Optionally sends a reason for 
 * the kick if provided.
 */
void Command::kick(std::string channel, std::string user_kick, const std::vector<std::string>& splitmsg, User user) {
    std::vector<Channel>::iterator it_c = channel_exist(channel);

    if (it_c != Server::_channels.end()) {
        std::vector<User>::iterator it_s;
        std::vector<User>::iterator it_o;
        unsigned long i = 3;

        for (it_s = it_c->users.begin(); it_s != it_c->users.end(); ++it_s) {
            if (it_s->_nickname == user_kick) {
                if (!it_c->isOperator(user)) {
                    ErrorMsg(user._fd, "Not an operator.\r\n", "482");
                    return;
                }

                if (user._nickname == user_kick) {
                    ErrorMsg(user._fd, "You cannot kick yourself.\r\n", "404");
                    return;
                }

                send(it_s->_fd, "You have been kicked from the channel.\r\n", strlen("You have been kicked from the channel.\r\n"), 0);

                if (splitmsg.size() > 3) {
                    send(it_s->_fd, "Reason for kicking: ", strlen("Reason for kicking: "), 0);
                }

                while (i < splitmsg.size()) {
                    send(it_s->_fd, splitmsg.at(i).c_str(), strlen(splitmsg.at(i).c_str()), 0);
                    send(it_s->_fd, " ", 1, 0);
                    i++;
                }

                send(it_s->_fd, "\r\n", 2, 0);

                it_c->users.erase(it_s);

                for (it_o = it_c->operators.begin(); it_o != it_c->operators.end(); ++it_o) {
                    if (it_o->_nickname == user_kick) {
                        it_c->operators.erase(it_o);
                        break;
                    }
                }

                return;
            }
        }

        ErrorMsg(user._fd, (user_kick + " :No such nickname.\r\n"), "401");
    } else {
        ErrorMsg(user._fd, (channel + " :No such channel.\r\n"), "403");
    }
}
