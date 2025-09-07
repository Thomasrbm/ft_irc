#include "../includes/User.hpp"

User::User() : nickname(""), username(""), fd(-1),
hasNickname(false), hasUsername(false), hasPass(false), isRegister(false), welcomeMessage(false) {}

User::User(const User &src) {
	*this = src;
}
User &User::operator=(const User &src) {
	if (this == &src)
		return (*this);
	this->nickname = src.nickname;
	this->username = src.username;
	this->fd = src.fd;
	this->hasNickname = src.hasNickname;
	this->hasUsername = src.hasUsername;
	this->hasPass = src.hasPass;
	this->isRegister = src.isRegister;
	this->welcomeMessage = src.welcomeMessage;
	return (*this);
}
User::~User() {}

User::User(const std::string &nickname, const std::string &username) :
nickname(nickname), username(username), fd(-1),
hasNickname(false), hasUsername(false), hasPass(false), isRegister(false), welcomeMessage(false) {}

void User::setFd(const int &fd) {
	this->fd = fd;
}

void User::setNickname(const std::string &nickname) {
	this->nickname = nickname;
}

void User::setUsername(const std::string &username) {
	this->username = username;
}

void User::closeConnection() {
	if (this->fd > 0) {
		close(this->fd);
	}
}

void User::tryRegisterUser() {
	if (getHasNickname() && getHasUsername() && getHasPass()) {
		this->isRegister = true;
	}
}
