#include "../../includes/Server.hpp"
#include "../../includes/Utils.hpp"

bool Server::nickAlreadyInUse(const std::string &nick) {
	for (std::map<int, User>::iterator it = this->Users.begin(); it != this->Users.end(); ++it) {
		if (it->second.getNickname() == nick) {
			return (true);
		}
	}
	return (false);
}

void Server::welcomeUser(const int &clientFd, const std::string &name) const {
	sendRPL(clientFd, RPL_WELCOME, name, "Welcome to the IRC server " + name + "!");
	sendRPL(clientFd, RPL_YOURHOST, name, "Your host is ircserv");
	sendRPL(clientFd, RPL_CREATED, name, "This server was created today");
}

void Server::handleNick(const int &clientFd, const std::string &line) {
	std::string nick = getParam(NICK_CMD_LENGTH, line);

	if (nick.empty()) {
		sendERR_NEEDMOREPARAMS(clientFd, CMD_NICK);
		return ;
	}

	if (this->nickAlreadyInUse(nick)) {
		sendRPL(clientFd, ERR_NICKNAMEINUSE, this->findNameById(clientFd), MSG_ERR_NICKNAMEINUSE);
		return ;
	}

	if (nick.find('#') != std::string::npos) {
		this->Users[clientFd].setHasNickname(false);
		this->Users[clientFd].setHasRegister(false);
		sendRPL(clientFd, ERR_NICKCOLLISION, this->findNameById(clientFd), MSG_ERR_INVALIDNICK);
		return ;
	}

	this->Users[clientFd].setNickname(nick);
	this->Users[clientFd].setHasNickname(true);
	this->Users[clientFd].tryRegisterUser();

	// Envoyer les messages de bienvenue seulement après enregistrement complet
	if (this->Users[clientFd].getIsRegister() && !this->Users[clientFd].getWelcomeMessage()) {
		welcomeUser(clientFd, nick);
		this->Users[clientFd].hasWelcomeMessage();
	}
}

void Server::handleUsername(const int &clientFd, const std::string &line) {
	std::string username = getParam(USER_CMD_LENGTH, line);

	if (username.empty()) {
		sendERR_NEEDMOREPARAMS(clientFd, CMD_USER);
		return ;
	}

	this->Users[clientFd].setHasUsername();
	this->Users[clientFd].setUsername(username);
	this->Users[clientFd].tryRegisterUser();

	// Envoyer les messages de bienvenue seulement après enregistrement complet
	if (this->Users[clientFd].getIsRegister() && !this->Users[clientFd].getWelcomeMessage()) {
		welcomeUser(clientFd, this->Users[clientFd].getNickname());
		this->Users[clientFd].hasWelcomeMessage();
	}
}

int Server::findIdByName(const std::string &name) const {
	for (std::map<int, User>::const_iterator it = this->Users.begin(); it != this->Users.end(); ++it) {
		if (it->second.getNickname() == name) {
			return (it->first);
		}
	}
	return (-1);
}

std::string Server::findNameById(const int &clientFd) const {
	for (std::map<int, User>::const_iterator it = this->Users.begin(); it != this->Users.end(); ++it) {
		if (it->second.getFd() == clientFd) {
			return (it->second.getNickname());
		}
	}
	return (" ");
}

