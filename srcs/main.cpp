#include <iostream>
#include <cstdlib>
#include <signal.h>
#include "../includes/Server.hpp"
#include "../includes/Utils.hpp"

void setupSignal() {
	signal(SIGINT, Server::signalHandler);
	signal(SIGQUIT, Server::signalHandler);
}

bool checkArgs(const int port, const std::string password) {
	if (port < 1024 || port > 65535) {
		std::cerr << "Port needs to be between 1024 and 65535" << std::endl;
		return (false);
	}
	if (password.length() > 18) {
		std::cerr << "Password too long" << std::endl;
		return (false);
	}
	return (true);
}


int main(int ac, char **av) {

	if (ac != 3 || !checkArgs(std::atoi(av[PORT]), av[PASSWORD])) {
		std::cerr << "usage: ./ircserv <port> <password>" << std::endl;
		return (EXIT_FAILURE);
	}
	try {
		Server serv;
		setupSignal();
		serv.initServer(std::atoi(av[PORT]), av[PASSWORD]);
		serv.runServer();
	}
	catch(const std::exception& e) {
		std::cerr << e.what() << '\n';
		Server::running = false;
	}
	return (EXIT_SUCCESS);
}
