#include "../../includes/Server.hpp"
#include "../../includes/Utils.hpp"

void Server::sendRPL(const int &clientFd, std::string code, const std::string &nick, const std::string &message) const {
	std::string buffer = ":server " + code + " " + nick + " :" + message + "\r\n";
	send(clientFd, buffer.c_str(), buffer.size(), 0);
}

void Server::sendERR_NEEDMOREPARAMS(const int &clientFd, const std::string &command) {
	const std::string code(ERR_NEEDMOREPARAMS);

	std::string buffer(SERV_NAME);
	buffer += code + " ";
	buffer += this->Users[clientFd].getNickname() + " ";
	buffer += command + " :";
	buffer += MSG_ERR_NEEDMOREPARAMS;
	buffer += "\r\n";

	send(clientFd, buffer.c_str(), buffer.size(), 0);
}

void Server::sendERR_NOSUCHCHANNEL(const int &clientFd, const std::string &channelName) {
	const std::string code(ERR_NOSUCHCHANNEL);

	std::string buffer(SERV_NAME);
	buffer += code + " ";
	buffer += this->Users[clientFd].getNickname() + " ";
	buffer += channelName + " :";
	buffer += MSG_ERR_NOSUCHCHANEL;
	buffer += "\r\n";

	send(clientFd, buffer.c_str(), buffer.size(), 0);
}

void Server::sendRPL_CHANNELMODEIS(const int &clientFd, const Channel &channel) {
	std::string mode;
	std::string value;
	if (channel.getInviteOnly()) {
		mode += "i";
	} if (channel.getTopicOpOnly()) {
		mode += "t";
	} if (channel.getHasKey()) {
		mode += "k";
		value += channel.getKey();
	} if (channel.getUserLimit() > 0) {
		mode += "l";
		std::ostringstream oss;
		oss << channel.getUserLimit();
		if (value.empty()) {
			value += oss.str();
		} else {
			value += " ";
			value += oss.str();
		}
		std::cout << channel.getUserLimit() << std::endl;
	}

	const std::string code = RPL_CHANNELMODEIS;

	std::string buffer(SERV_NAME);
	buffer += code + " ";
	buffer += this->Users[clientFd].getNickname() + " ";
	buffer += channel.getName();

	if (mode.empty()) {
		buffer += "\r\n";
		send(clientFd, buffer.c_str(), buffer.size(), 0);
		return ;
	}

	buffer += " +" + mode + " ";
	buffer += value + "\r\n";

	send(clientFd, buffer.c_str(), buffer.size(), 0);
}


void Server::sendRPL_NOTOPIC(const int &clientFd, const Channel &channel) {
	const std::string code = RPL_NOTOPIC;

	std::string buffer(SERV_NAME);
	buffer += code + " ";
	buffer += this->Users[clientFd].getUsername() + " ";
	buffer += channel.getName() + " :";
	buffer += MSG_NOTOPIC;
	buffer += "\r\n";

	send(clientFd, buffer.c_str(), buffer.size(), 0);
}

void Server::sendRPL_TOPIC(const int &clientFd, const Channel &channel) {
	const std::string code = RPL_TOPIC;

	std::string buffer(SERV_NAME);
	buffer += code + " ";
	buffer += this->Users[clientFd].getUsername() + " ";
	buffer += channel.getName() + " :";
	buffer += channel.getTopic() + "\r\n";

	send(clientFd, buffer.c_str(), buffer.size(), 0);
}

void Server::sendRPL_TOPICWHOTIME(const int &clientFd, const Channel &channel) {
	std::ostringstream oss;
	oss << channel.getTopicTime();
	const std::string code = RPL_TOPICWHOTIME;

	std::string buffer(SERV_NAME);
	buffer += code + " ";
	buffer += this->Users[clientFd].getNickname() + " ";
	buffer += channel.getName() + " ";
	buffer += channel.getTopicSetter() + " ";
	buffer += oss.str() + "\r\n";

	send(clientFd, buffer.c_str(), buffer.size(), 0);
}

void Server::sendERR_CHANOPRIVSNEEDED(const int &clientFd, const std::string &channelName) {
	const std::string code(ERR_CHANOPRIVSNEEDED);

	std::string buffer(SERV_NAME);
	buffer += code + " ";
	buffer += this->Users[clientFd].getNickname() + " ";
	buffer += channelName + " :";
	buffer += MSG_ERR_CHANOPRIVSNEEDED;
	buffer += "\r\n";

	send(clientFd, buffer.c_str(), buffer.size(), 0);
}


void Server::sendRPL_INVITED(const int &clientFd, const std::string &toInvite, const Channel &channel) {
	const std::string code = RPL_INVITING;

	std::string buffer(SERV_NAME);
	buffer += code + " ";
	buffer += this->Users[clientFd].getNickname() + " ";
	buffer += toInvite + " ";
	buffer += channel.getName() + "\r\n";

	broadcastToAllMember(channel, buffer);
}

void Server::sendRPL_NAMEREPLY(const int &clientFd, Channel &channel) {
	const std::string code = RPL_NAMREPLY;
	std::string channelMode;
	if (channel.getInviteOnly()) {
		channelMode += " * ";
	} else {
		channelMode += " = ";
	}


	std::string buffer(SERV_NAME);
	buffer += code + " ";
	buffer += this->Users[clientFd].getNickname();
	buffer += channelMode;
	buffer += channel.getName() + " :";

	std::vector<int> members = channel.getAllMembers();
	std::string name;
	for (std::vector<int>::const_iterator it = members.begin(); it < members.end(); it++) {
		if (!name.empty() || it == members.end()) {
			name += " ";
		}

		if (channel.isOperator(*it)) {
			name += "@";
		}

		name += this->Users[*it].getNickname();
	}

	buffer += name + "\r\n";

	send(clientFd, buffer.c_str(), buffer.size(), 0);
}

void Server::sendRPL_ENDOFNAMES(const int &clientFd, Channel &channel) {
	const std::string code = RPL_ENDOFNAMES;

	std::string buffer(SERV_NAME);
	buffer += code + " ";
	buffer += this->Users[clientFd].getNickname() + " ";
	buffer += channel.getName() + " :";
	buffer += MSG_RPL_ENDOFNAMES;
	buffer += "\r\n";

	send(clientFd, buffer.c_str(), buffer.size(), 0);
}
