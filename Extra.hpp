#ifndef EXTRA_HPP
#define EXTRA_HPP

#include "Server.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <sstream>



bool compareUsersByNickname(const User& u1, const User& u2);
void ErrorMsg(int fd, const std::string &message, const std::string &key);
std::vector<std::string> split(const std::string str);
std::vector<std::string> ft_split(std::string str, char delimiter);
bool detectProfanity(const std::string& message, std::string& detectedWord);
int processMessageWithProfanityCheck(int userFd, const std::string& message);


#endif
