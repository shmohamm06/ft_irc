/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Mode.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shmohamm <shmohamm@student.42abudhabi.a    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/18 13:49:39 by shmohamm          #+#    #+#             */
/*   Updated: 2024/12/18 13:49:40 by shmohamm         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Channel.hpp"

/**
 * Checks if the user is an operator of the channel.
 * @param user The User object to check against the list of operators.
 * @return 1 if the user is an operator, 0 otherwise.
 */
int Channel::isOperator(User user)
{
    std::vector<User>::const_iterator operatorIt;
    operatorIt = this->operators.begin();
    while (operatorIt != this->operators.end())
    {
        if (operatorIt->_nickname == user._nickname)
            return (1);
        operatorIt++;
    }
    return (0);
}

/**
 * Finds an operator by their file descriptor.
 * @param userFd The file descriptor of the user.
 * @return An iterator to the operator if found, or the end iterator if not.
 */
std::vector<User>::iterator Channel::channel_operator(int userFd)
{
    this->it_operators = this->operators.begin();
    while (this->it_operators != this->operators.end())
    {
        if (this->it_operators->_fd == userFd)
            return (it_operators);
        it_operators++;
    }
    return (it_operators);
}

/**
 * Sets a mode for the channel, updating the mode based on the action (+ or -).
 * @param modeFlag The mode character (e.g., 'k', 'o', 'l').
 * @param action The action to perform ('+' to enable, '-' to disable the mode).
 */
void Channel::setMode(char modeFlag, char action)
{
    std::map<char, int>::iterator modeIt;
    modeIt = this->_mode.find(modeFlag);
    if (modeIt != this->_mode.end())
    {
        if (action == '+')
            modeIt->second = 1;
        else
            modeIt->second = 0;
    }
}

/**
 * Executes a mode change on the channel based on the provided mode and argument.
 * The function handles specific modes like key setting, operator assignment, and user limit changes.
 * @param modeSetting The mode to be applied, including the action (+ or -) and mode type (e.g., 'k', 'o', 'l').
 * @param user The User object requesting the mode change.
 * @param argument The argument for the mode (e.g., password, nickname, or limit).
 */
void Channel::exec_mode(std::string modeSetting, User &user, std::string argument) {
    switch (modeSetting[1]) {
        case 'k': // Key setting for channel
            if (modeSetting[0] == '+') {
                if (argument.empty()) {
                    ErrorMsg(user._fd, "No password provided for the channel.\r\n", "461");
                } else {
                    this->_pass = argument;
                }
            } else {
                this->_pass.clear();
            }
            break;

        case 'o': { // Operator assignment or removal
            std::vector<User>::iterator userIt = this->users.begin();
            while (userIt != this->users.end()) {
                if (userIt->_nickname == argument)
                    break;
                ++userIt;
            }
            if (userIt != this->users.end()) {
                if (modeSetting[0] == '+') {
                    if (this->isOperator(*userIt)) {
                        send(user._fd, "User is already an operator.\r\n", strlen("User is already an operator.\r\n"), 0);
                    } else {
                        this->operators.push_back(*userIt);
                        send(userIt->_fd, BRIGHT_WHITE "You have been granted operator status.\r\n" RESET, strlen("You have been granted operator status.\r\n"), 0);
                    }
                } else {
                    userIt = this->channel_operator(userIt->_fd);
                    if (userIt != this->operators.end()) {
                        if (userIt->_nickname != user._nickname) {
                            send(userIt->_fd, BRIGHT_YELLOW "You have been removed from operator status.\r\n" RESET, strlen("You have been removed from operator status.\r\n"), 0);
                            this->operators.erase(userIt);
                        } else {
                            send(userIt->_fd, BRIGHT_RED "You cannot remove yourself as an operator.\r\n", strlen("You cannot remove yourself as an operator.\r\n"), 0);
                        }
                    } else {
                        send(user._fd, "User is not an operator.\r\n", strlen("User is not an operator.\r\n"), 0);
                    }
                }
            } else {
                ErrorMsg(user._fd, (argument + " :No such user found.\r\n"), "401");
            }
            break;
        }
        case 'l': // User limit setting
            if (modeSetting[0] == '+') {
                int newLimit = std::atoi(argument.c_str());
                if (newLimit <= 0) {
                    send(user._fd, "Invalid limit number.\r\n", strlen("Invalid limit number.\r\n"), 0);
                } else {
                    if (this->user_length() > newLimit) {
                        send(user._fd, "Cannot set limit. Current user count exceeds the new limit.\r\n", strlen("Cannot set limit. Current user count exceeds the new limit.\r\n"), 0);
                    } else {
                        this->_user_limit = newLimit;
                        std::string limitMsg = "Channel " + this->getName() + " now has a user limit of " + std::to_string(this->_user_limit) + ".\r\n";
                        send(user._fd, limitMsg.c_str(), limitMsg.length(), 0);
                    }
                }
            }
            break;
    }
    this->setMode(modeSetting[1], modeSetting[0]);
    std::cout << modeSetting[0] << std::endl;
    std::cout << _pass << std::endl;
}
