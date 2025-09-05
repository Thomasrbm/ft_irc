#include "../../includes/Server.hpp"
#include "../../includes/Utils.hpp"

bool Server::running = true;

Server::Server() : socketfd(0), epollFd(0) {}
Server::Server(const Server &src) {*this = src;}
Server	Server::operator=(const Server &src) {
	this->port = src.port;
	this->password = src.password;
	return (*this);
}

Server::~Server() {
	if (this->socketfd > 0) {
		close(this->socketfd);
	}
	if (this->epollFd > 0) {
		close(this->epollFd);
	}
	for (std::map<int, User>::iterator it = Users.begin(); it != Users.end(); ++it) {
		it->second.closeConnection();
	}
}

void Server::signalHandler(int signum) {
	(void) signum;
	std::cout << std::endl << "Closing server..." << std::endl;
	running = false;
}

void Server::initSocket() {
	int opt = 1;

	this->socketfd = socket(AF_INET, SOCK_STREAM, 0);
	if (this->socketfd < 0) {
		throw(std::runtime_error("Error while opening the socket"));
	}

	struct sockaddr_in servAddress;
	servAddress.sin_family = AF_INET;
	servAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddress.sin_port = htons(this->port);

	if (setsockopt(this->socketfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
		throw(std::runtime_error("Error while setting option for socket"));
	}

	if (bind(this->socketfd, (struct sockaddr *) &servAddress, sizeof(servAddress)) < 0) {
		throw(std::runtime_error("Error while binding address"));
	}

	if (listen(this->socketfd, MAX_USER) < 0) {
		throw(std::runtime_error("Error while opening the server"));
	}

}

void Server::initEpoll() {
	this->epollFd = epoll_create1(0);
	if (this->epollFd < 0) {
		throw(std::runtime_error("Error while creating epoll fd"));
	}

	this->event.events = EPOLLIN;
	this->event.data.fd = this->socketfd;
	if (epoll_ctl(this->epollFd, EPOLL_CTL_ADD, this->socketfd, &this->event)) {
		throw(std::runtime_error("Error while adding server to epoll instance"));
	}

	std::cout << "Server listening on port : " << this->port << std::endl;
}

void Server::initServer(const int &port, const std::string &password) {
	this->port = port;
	this->password = password;
	this->initSocket();
	this->initEpoll();
}

void Server::runServer() {
	while (running) {
		int eventNumber = epoll_wait(this->epollFd, this->events, MAX_EVENTS, -1);
		if (eventNumber == -1) {
			std::cerr << "Failed to get events" << std::endl;
			continue ;
		}

		for (int i = 0; i < eventNumber; i++) {
			if (events[i].data.fd == this->socketfd) {
				this->acceptUser();
			} else {
				this->parseInput(events[i].data.fd);
			}
		}
	}
}

void Server::acceptUser() {
	int userFd = accept(this->socketfd, NULL, NULL);
	if (userFd < 0) {
		std::cerr << "Failed to accept User" << std::endl;
		return ;
	}

	fcntl(userFd, F_SETFL, O_NONBLOCK);

	epoll_event UserEvent;
	UserEvent.events = EPOLLIN;
	UserEvent.data.fd = userFd;
	if (epoll_ctl(this->epollFd, EPOLL_CTL_ADD, userFd, &UserEvent)) {
		std::cerr << "Error while adding User to epoll instance";
		return ;
	}

	User newUser;
	newUser.setFd(userFd);
	this->Users.insert(std::pair<int, User>(userFd, newUser));

	if (!this->hasPassword()) {
		this->Users[userFd].setHasPass();
	}

	this->Users[userFd].tryRegisterUser();
	std::cout << "New User on fd : " << userFd << std::endl;
}

void Server::parseInput(int clientFd) {
	char input[BUFFER_SIZE];

	int inputLength = read(clientFd, input, sizeof(input) - 1);
	if (inputLength < 0) {
		std::cerr << "Error while reading client input from fd : " << clientFd << std::endl;
		return;
	} else if (inputLength == 0) {
		close(clientFd);
		this->Users.erase(clientFd);
		std::cout << "Closing connection on fd : " << clientFd << std::endl;
		return;
	}

	input[inputLength] = '\0';

	std::string &tmp = this->Users[clientFd].getInput();
	tmp.append(input, inputLength);

	size_t pos;
	while ((pos = tmp.find("\r\n")) != std::string::npos) {
		std::string line = tmp.substr(0, pos);
		tmp.erase(0, pos + 2);
		std::cout << "LINE: " << line << std::endl;

		handleLine(clientFd, line);
	}
}

void Server::handleLine(int clientFd, const std::string &line) {
	if (line.find("NICK", 0) == 0) {
		handleNick(clientFd, line);
	} else if (line.find("USER", 0) == 0) {
		handleUsername(clientFd, line);
	} else if (line.find("PASS", 0) == 0) {
		handlePass(clientFd, line);
	}

	if (!this->Users[clientFd].getIsRegister()) {
		sendRPL(clientFd, ERR_NOTREGISTERED, "guest", ":please enter a nickname, username and pass if needer");
		return ;
	}

	if (line.find("JOIN", 0) == 0) {
		handleJoin(clientFd, line);
	} else if (line.find("KICK", 0) == 0) {
		handleKick(clientFd, line);
	}
	else if (line.find("PRIVMSG", 0) == 0) {
		handleChannelMessage(clientFd, line);
	}
	else if (line.find("MODE", 0) == 0) {
		handleMode(clientFd, line);
	} else if (line.find("PASS", 0) == 0) {
		handlePass(clientFd, line);
	} else {
		sendRPL(clientFd, ERR_UNKNOWNCOMMAND, this->findNameById(clientFd), line + " this command is unknown" );
	}
}

void Server::sendRPL(const int &clientFd, std::string code, const std::string &nick, const std::string &message) const {
	std::string buffer = ":server " + code + " " + nick + " :" + message + "\r\n";
	send(clientFd, buffer.c_str(), buffer.size(), 0);
}

void Server::handlePass(const int clientFd, const std::string &line) {
	if (this->password.empty()) {
		return ;
	}

	std::string pass = getParam(PASS_CMD, line);
	if (pass.empty() || pass != this->password) {
		sendRPL(clientFd, ERR_PASSWDMISMATCH, this->findNameById(clientFd), "Wrong password");
		this->Users[clientFd].closeConnection();
		return ;
	}
}

bool Server::hasPassword() const {
	if (this->password.empty()) {
		return (false);
	}
	return (true);
}

