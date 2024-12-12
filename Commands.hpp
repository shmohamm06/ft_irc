#ifndef COMMAND_HPP
# define COMMAND_HPP

#include <iostream>
#include <string>
#include <vector>
#include "Server.hpp"
#include "Channel.hpp"

class User;
class Channel;

class Command {
    private:
        std::string message;
        std::vector<Channel>::iterator channel_it;
        std::vector<User>::iterator user_it;

    public:
        Command(void);
        ~Command(void);
        void ajoin(std::string channelName, std::string channelKey, User user);
        void who(std::string channel_s, User user);
        void privmsg(std::string receiver, const std::vector<std::string>& splitmsg, User user);
        void invite(std::string invitee, std::string channel, User requesting_user);
        void kick(std::string channel, std::string user_kick, const std::vector<std::string>& splitmsg, User user);
        void mode(std::string channel_s, std::string mode, User user, std::string arg);
        void topic(std::string channel_s, std::string topic, User user);
        std::vector<std::string> ft_split(std::string str, char delimiter);
        std::vector<Channel>::iterator channel_exist(std::string channel);
        std::vector<User>::iterator user_exist(std::string nick);
};

#endif
