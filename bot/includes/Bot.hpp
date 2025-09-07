#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstdlib>
#include "../../includes/Server.hpp"
#include "../../includes/Utils.hpp"

#define FILE "./srcs/message.txt"
#define BOT_NAME "ircBot"

class Bot {
private:
	int			fd;
	int			port;
	std::string	password;
	std::string	ip;

	std::vector<std::string> messages;
	size_t		nbMessages;
	std::string input;
public:
	static bool	running;
	static void	signalHandler(int signum);

	Bot();
	Bot(const Bot &src);
	Bot operator=(const Bot &src);
	~Bot();

	Bot(const int port, const std::string password, const std::string ip);
	void	loadWelcomeMessages();
	void	printMessage();

	const int &getFd() const {return(this->fd);};
	const int &getport() const {return(this->port);};
	const std::string &getpassword() const {return(this->password);};
	const std::string &getip() const {return(this->ip);};
	const size_t &getNbMessages() const {return(this->nbMessages);};

	void	connectBot();
	void	closeConnection();
	void	sendToServ(std::string msg);
	void	runBot();
	void	treatRequest();
	void	handleLine(const std::string &line);
	bool	findCommand(const std::string &line, const std::string &cmd);
	void	joinChannel(const std::string &line);
	void	welcomeUser(const std::string &line);
};

