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

void Server::handleCapReq(const int &userFd) const {
    std::string msg = ":server CAP * LS :multi-prefix sasl\r\n";
    send(userFd, msg.c_str(), msg.length(), 0);
}

void Server::acceptUser() {
	std::cout << "TENTATIVE DE CONNECTION" << std::endl;
	int userFd = accept(this->socketfd, NULL, NULL);
	if (userFd < 0) {
		std::cerr << "Failed to accept User" << std::endl;
		return ;
	}

	fcntl(userFd, F_SETFL, O_NONBLOCK);

	// // Récupérer l'adresse IP du client
    // struct sockaddr_in clientAddr;
    // socklen_t addrLen = sizeof(clientAddr);
    // if (getpeername(userFd, (struct sockaddr*)&clientAddr, &addrLen) == -1) {
    //     std::cerr << "Failed to get client IP" << std::endl;
    //     return;
    // }
    // std::cout << "New User connected from IP: "
    //           << inet_ntoa(clientAddr.sin_addr)
    //           << " Port: "
    //           << ntohs(clientAddr.sin_port)
    //           << std::endl;


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
	
	handleCapReq(userFd);
	this->Users[userFd].tryRegisterUser();
	std::cout << "New User on fd : " << userFd << std::endl;
}

void Server::parseInput(int clientFd) {
	if (this->Users.size() <= 0) {
		return ;
	}

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

	std::string tmp;
	tmp.append(input, inputLength);

	size_t pos;
	while ((pos = tmp.find("\r\n")) != std::string::npos) {
		std::string line = tmp.substr(0, pos);
		tmp.erase(0, pos + 2);
		std::cout << "LINE: " << line << std::endl;

		handleLine(clientFd, line);
	}
}

void Server::handleLine(const int &clientFd, const std::string &line) {
	if (line.find(CMD_CAP, 0) == 0) {
		handleCapReq(clientFd);
		return ;
	} else if (line.find(CMD_PING, 0) == 0) {
		handlePing(clientFd, line);
		return ;
	} else if (line.find(CMD_NICK, 0) == 0) {
		handleNick(clientFd, line);
		return ;
	} else if (line.find(CMD_USER, 0) == 0) {
		handleUsername(clientFd, line);
		return ;
	} else if (line.find(CMD_PASS, 0) == 0) {
		handlePass(clientFd, line);
		return ;
	}

	if (!this->Users[clientFd].getIsRegister()) {
		if (!this->Users[clientFd].getHasNickname() && this->Users[clientFd].getWelcomeMessage()) {
			sendRPL(clientFd, ERR_NOTREGISTERED, "guest", MSG_NEED_NICK);
		}

		if (!this->Users[clientFd].getHasUsername() && this->Users[clientFd].getWelcomeMessage()) {
			sendRPL(clientFd, ERR_NOTREGISTERED, "guest", MSG_NEED_USER);
		}
		return ;
	}

	if (line.find(CMD_JOIN, 0) == 0) {
		handleJoin(clientFd, line);
	} else if (line.find(CMD_PART, 0) == 0) {
		handlePart(clientFd, line);
	} else if (line.find(CMD_INVITE, 0) == 0) {
		handleInvite(clientFd, line);
	} else if (line.find(CMD_KICK, 0) == 0) {
		handleKick(clientFd, line);
	} else if (line.find(CMD_PRIVMSG, 0) == 0) {
		// Vérifier si c'est un DCC dans un PRIVMSG
		if (line.find("DCC SEND") != std::string::npos) {
			handleDccSend(clientFd, line);
		} else if (line.find("DCC ACCEPT") != std::string::npos) {
			handleDccAccept(clientFd, line);
		} else {
			handlePrivateMessage(clientFd, line);
		}
	} else if (line.find(CMD_DCC, 0) == 0) {
		if (line.find("DCC SEND") != std::string::npos) {
			handleDccSend(clientFd, line);
		} else if (line.find("DCC ACCEPT") != std::string::npos) {
			handleDccAccept(clientFd, line);
		}
	} else if (line.find(CMD_MODE, 0) == 0) {
		handleMode(clientFd, line);
	} else if (line.find(CMD_TOPIC, 0) == 0) {
		handleTopic(clientFd, line);
	} else {
		sendRPL(clientFd, ERR_UNKNOWNCOMMAND, this->findNameById(clientFd), line + " :" + MSG_ERR_UNKNOWNCOMMAND);
	}
}

void Server::handlePass(const int &clientFd, const std::string &line) {

	if (this->password.empty()) {
		return ;
	}

	std::string pass = getParam(PASS_CMD_LENGTH, line);

	if (pass.empty() || pass != this->password) {
		sendRPL(clientFd, ERR_PASSWDMISMATCH, this->findNameById(clientFd), MSG_ERR_PASSWDMISMATCH);
		sendRPL(clientFd, ERR_YOUWILLBEBANNED, this->findNameById(clientFd), MSG_ERR_YOUWILLBEBANNED);
		this->Users[clientFd].closeConnection();
		return ;
	}

	this->Users[clientFd].setHasPass();
}

bool Server::hasPassword() const {
	if (this->password.empty()) {
		return (false);
	}
	return (true);
}

void Server::handlePing(const int &clientFd, const std::string &line) {
	const std::string param = getParam(PING_CMD_LENGTH, line);

	if (param.empty()) {
		sendRPL(clientFd, ERR_NOORIGIN, this->Users[clientFd].getNickname(), MSG_ERR_NOORIGIN);
		return ;
	}

	const std::string pong = "PONG :" + param + "\r\n";

	send(clientFd, pong.c_str(), pong.size(), 0);
}

void Server::broadcastToAllMember(Channel chanel, const std::string message) {
	std::vector<int> members = chanel.getAllMembers();

	for (std::vector<int>::iterator it = members.begin(); it != members.end(); ++it) {
		send(*it, message.c_str(), message.size(), 0);
	}
}
