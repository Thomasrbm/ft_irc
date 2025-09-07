#include "../includes/Bot.hpp"

bool Bot::running = true;

Bot::Bot(): fd(-1), port(-1), password(""), ip(""), nbMessages(0) {}
Bot::Bot(const Bot &src): fd(src.fd), port(src.port), password(src.password), ip(src.ip), nbMessages(0) {}
Bot Bot::operator=(const Bot &src) {
	this->fd = src.fd;
	this->port = src.port;
	this->password = src.password;
	this->ip = src.ip;
	this->nbMessages = src.nbMessages;

	return (*this);
}
Bot::~Bot() {this->closeConnection();}

Bot::Bot(const int port, const std::string password, const std::string ip):
fd(-1), port(port), password(password), ip(ip), nbMessages(0) {
	loadWelcomeMessages();
}

void Bot::signalHandler(int signum) {
	(void) signum;
	std::cout << std::endl << "Closing Bot..." << std::endl;
	running = false;
}

void Bot::closeConnection() {
	const std::string msg("QUIT :leaving");
	sendToServ(msg);
	close(this->fd);
}

void Bot::loadWelcomeMessages() {
	std::ifstream	msgFile(FILE);
	if (!msgFile.is_open()) {
		throw(std::runtime_error("Can't open message file"));
	}

	std::string line;
	while (std::getline(msgFile, line)) {
		this->messages.push_back(line);
	}

	msgFile.close();
	this->nbMessages = this->messages.size();
}

void Bot::printMessage() {
	for (size_t i = 0; i < this->messages.size(); i++) {
		std::cout << this->messages[i] << std::endl;
	}

}

void Bot::connectBot() {
	this->fd = socket(AF_INET, SOCK_STREAM, 0);
	if (this->fd < 0) {
		throw(std::runtime_error("Error while opening the socket"));
	}

	struct sockaddr_in servAddress;
	servAddress.sin_family = AF_INET;
	servAddress.sin_addr.s_addr = inet_addr(this->ip.c_str());
	servAddress.sin_port = htons(this->port);

	if (connect(this->fd, (struct sockaddr *) &servAddress, sizeof(servAddress)) < 0) {
		throw(std::runtime_error("Error while connecting to server"));
	}

	fcntl(this->fd, F_SETFL, O_NONBLOCK);

	std::string bot(BOT_NAME);
	this->sendToServ("PASS " + this->password);
	this->sendToServ("NICK " + bot);
	this->sendToServ("USER " + bot);
}

void Bot::sendToServ(std::string msg) {
	msg += "\r\n";
	send(this->fd, msg.c_str(), msg.size(), 0);
}

void Bot::runBot() {
	while (running) {
		treatRequest();
	}
}

void Bot::treatRequest() {
	char input[BUFFER_SIZE];

	int inputLength = read(this->fd, input, sizeof(input) - 1);
	if (inputLength < 0) {
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

		handleLine(line);
	}
}

bool Bot::findCommand(const std::string &line, const std::string &cmd) {
	size_t pos = 0;
	size_t lastPos = 0;
	std::string buffer;

	while (pos != std::string::npos) {
		pos = line.find(' ', lastPos);
		if (pos == std::string::npos) {
			break ;
		}

		buffer = line.substr(lastPos, pos - lastPos);
		if (buffer == cmd) {
			return (true);
		}
		lastPos = pos + 1;
	}
	return(false);
}

void Bot::handleLine(const std::string &line) {
	if (findCommand(line, CMD_INVITE)) {
		joinChannel(line);
	} else if (findCommand(line, CMD_JOIN)) {
		welcomeUser(line);
	}

}

void Bot::joinChannel(const std::string &line) {
	const std::string channelName = getChannelName(line);

	if (channelName.empty()) {
		return ;
	}

	std::string msg(CMD_JOIN);
	msg += " " + channelName;
	sendToServ(msg);
}

void Bot::welcomeUser(const std::string &line) {
	const size_t pos = line.find('!');
	if (pos == std::string::npos) {
		return ;
	}

	const std::string nick = line.substr(1, pos - 1);

	if (nick == BOT_NAME) {
		return ;
	}

	std::srand(std::time(NULL));
	int num = std::rand() % this->nbMessages + 1;


	const std::string welcomeMsg = this->messages[num];

	std::string msg(CMD_PRIVMSG);
	msg += " " + getChannelName(line);
	msg += " :Bienvenue " + nick + ": " + welcomeMsg;

	sendToServ(msg);
}
