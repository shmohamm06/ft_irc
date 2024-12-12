#include "Extra.hpp"

void ErrorMsg(int fd, const std::string &error_msg, const std::string &code)
{
	std::string errormsg = code + " ERROR: " + error_msg;
	send(fd, errormsg.c_str(), strlen(errormsg.c_str()), 0);
}

std::vector<std::string> split(const std::string str)
{
	std::vector<std::string> vector;
	std::istringstream input_str_stream(str);
	std::string cmd;
	while (input_str_stream >> std::skipws >> cmd)
		vector.push_back(cmd);
	return vector;
}

std::vector<std::string> ft_split(std::string str, char delimiter)
{
	std::vector<std::string> substrings;
	std::string substring = "";
	size_t i = 0;
	while ( i < str.length())
	{
		if (str[i] != delimiter)
		{
			substring = substring + str[i];
		}
		else
		{
			substrings.push_back(substring);
			while (str[i] == delimiter)
				i++;
			i--;
			substring = "";
		}
		i++;
	}
	substrings.push_back(substring);
	return (substrings);
}

static std::vector<std::string> createProfanityList() {
    std::vector<std::string> list;
    list.push_back("badword1");
    list.push_back("badword2");
    list.push_back("badword3");
    return list;
}

static std::vector<std::string> profanityList = createProfanityList();

bool compareUsersByNickname(const User& u1, const User& u2) {
    return u1._nickname == u2._nickname;
}

// Function to detect profanity in a message
bool detectProfanity(const std::string& message, std::string& detectedWord) {
    // Convert the message to lowercase for case-insensitive comparison
    std::string lowerMessage = message;
    std::transform(lowerMessage.begin(), lowerMessage.end(), lowerMessage.begin(), ::tolower);

    // Tokenize the message and check each word against the profanity list
    std::istringstream stream(lowerMessage);
    std::string word;
    while (stream >> word) {
        // Check if the word matches any profane word in the list
        if (std::find(profanityList.begin(), profanityList.end(), word) != profanityList.end()) {
            detectedWord = word;
            return true; // Profanity detected
        }
    }

    return false; // No profanity found
}

void processMessageWithProfanityCheck(int userFd, const std::string& message) {
    static std::map<std::string, int> profanityCounts; // Track profanity counts by user nickname
    std::string detectedWord;

    // Retrieve the user by their FD
    User* user = nullptr;
    for (std::vector<User>::iterator it = Server::users.begin(); it != Server::users.end(); ++it) {
        if (it->_fd == userFd) {
            user = &(*it);
            break;
        }
    }

    // Check if the user was found
    if (user == nullptr) {
        std::cerr << "Error: User with FD " << userFd << " not found." << std::endl;
        return;
    }

    // Check for profanity
    if (detectProfanity(message, detectedWord)) {
        std::string nickname = user->_nickname;

        // Increment the count for this user
        profanityCounts[nickname]++;
        int count = profanityCounts[nickname];

        // Handle warnings and actions
        if (count == 1) {
            std::string warning = "Warning: Your message contains inappropriate language (" + detectedWord +
                                  "). Continued violations will result in removal.\r\n";
            send(userFd, warning.c_str(), warning.length(), 0);
        } else if (count == 4) {
            std::string finalWarning = "Final Warning: One more violation will result in removal.\r\n";
            send(userFd, finalWarning.c_str(), finalWarning.length(), 0);
        } else if (count >= 5) {
            // Remove the user from all channels they are in
            for (std::vector<Channel>::iterator it = Server::_channels.begin(); it != Server::_channels.end(); ++it) {
                for (std::vector<User>::iterator userIt = it->users.begin(); userIt != it->users.end(); ++userIt) {
                    if (compareUsersByNickname(*userIt, *user)) {
                        it->users.erase(userIt); // Remove user from the channel
                        std::cout << "User " << nickname << " has been removed from channel " << it->name << " due to repeated profanity." << std::endl;
                        break; // Exit loop after removal
                    }
                }
            }

            // Notify the user about being kicked
            std::string kickMessage = "You have been removed from the channel(s) due to repeated violations of the rules.\r\n";
            send(userFd, kickMessage.c_str(), kickMessage.length(), 0);

            profanityCounts.erase(nickname); // Reset the count after kicking
            return;
        }

        // Log the infraction
        std::ofstream logFile("log.txt", std::ios::app);
        if (logFile.is_open()) {
            logFile << "User " << nickname << " sent profanity: " << detectedWord << "\n";
            logFile.close();
        } else {
            std::cerr << "Failed to open log file for profanity logging.\n";
        }
    }
}
