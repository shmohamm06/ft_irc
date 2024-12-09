#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <cstring>
#include <cstring>
#include <cstdlib>
#include <stdlib.h>
#include "Server.hpp"
#include <ctime>
#include <string>
#include <vector>
#include <map>

class User;

class Channel
{
	private:
		int _user_limit;
		std::string _pass;
		std::string _topic;
		std::map<char, int> _mode;
		time_t _creation_time; // Add creation time variable
	public:
		Channel();
		std::string name;
		std::vector<User> invites;
		std::vector<User> operators;
		std::vector<User> users;
		std::string message;
		std::vector<User>::iterator it_user;
		std::vector<User>::iterator it_operators;
		std::vector<User>::iterator it_invites;
		Channel(std::string str_name, std::string str_pass);
		~Channel();

		int getUserInfo();
		std::string getTopic();
		std::string getPass();
		std::map<char, int> getMode();
		std::vector<User> getUsers();
		std::vector<User> getOperators();
		std::string getName() const;
		time_t getCreationTime() const;
		void sendUserListToChannel();
		void sendChannelSummary() ;
		void sendCreationTime();
		void sendWelcomeMessage(User user_object);

		void setUserInfo(int num);
		void setTopic(std::string str);
		void setPass(std::string str);
		void setMode(char m, char sign);

		void addUserToChannel(User new_user_object);
		void exec_mode(std::string mode, User &user_object, std::string arg);
		std::vector<User>::iterator user_in_chan(int fd);
		std::vector<User>::iterator channel_operator(int fd);
		std::vector<User>::iterator inv_in_chan(int fd);
		int user_length(void);

		int isInvited(User user);
		int isMode(char m);
		int isOperator(User user);
		int isUser(User user);
		void delete_from_all(User& user);

		void removeUser(User& user);
		void removeInvite(User& user);
		void removeOperator(User& user);
		bool isEmpty();
};
// everything with channel done by Yasin Usman

#endif
