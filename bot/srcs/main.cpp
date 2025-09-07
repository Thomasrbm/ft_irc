#include "../includes/Bot.hpp"
#include <signal.h>
#include <cstdlib>

void setupSignal() {
	signal(SIGINT, Bot::signalHandler);
	signal(SIGQUIT, Bot::signalHandler);
}

bool checkIp(const std::string ip) {
	if (ip == "127.0.0.1") {
		return (true);
	}

	std::vector<std::string> nums;
	size_t	pos = 0;
	size_t	lastPos = 0;
	std::string num;

	while (pos != std::string::npos) {
		pos = ip.find('.', lastPos);

		num = ip.substr(lastPos, pos - lastPos);
		if (std::atoi(num.c_str()) < 0 || std::atoi(num.c_str()) > 255 || num.empty()) {
			return (false);
		}
		nums.push_back(num);
		lastPos = pos + 1;
	}

	if (nums.size() != 4) {
		return (false);
	} else {
		return (true);
	}
}

bool checkArgs(const int port, const std::string password, const std::string ip) {
	if (port < 1024 || port > 65535) {
		std::cerr << "Port needs to be between 1024 and 65535" << std::endl;
		return (false);
	}
	if (password.length() > 18) {
		std::cerr << "Password too long" << std::endl;
		return (false);
	}
	if (!checkIp(ip)) {
		std::cerr << "Invalid IPv4" << std::endl;
		return (false);
	}

	return (true);
}

int main(int ac, char **av) {
	std::string ip;
	if (ac == 4) {
		ip = av[IP];
	} else {
		ip = "127.0.0.1";
	}

	if (ac < 3 || ac > 4 ||  !checkArgs(std::atoi(av[PORT]), av[PASSWORD], ip)) {
		std::cout << "usage: ./ircserv <port> <password> optional: <ip>(set to localhost by default)" << std::endl;
		return (EXIT_FAILURE);
	}
	try {
		Bot ircBot(std::atoi(av[PORT]), av[PASSWORD], ip);
		setupSignal();
		ircBot.connectBot();
		ircBot.runBot();
	}
	catch(const std::exception& e) {
		std::cerr << e.what() << '\n';
		Bot::running = false;
	}
	return (EXIT_SUCCESS);
}
