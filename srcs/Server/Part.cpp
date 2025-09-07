#include "../../includes/Server.hpp"
#include "../../includes/Utils.hpp"

void Server::handlePart(const int &clientFd, const std::string &line) {
	const std::string channelName = getChannelName(line);

	if (channelName.empty()) {
		sendERR_NEEDMOREPARAMS(clientFd, CMD_PART);
		return ;
	}

	if (!this->channelExists(channelName)) {
		sendERR_NOSUCHCHANNEL(clientFd, channelName);
	}

	notifyPart(clientFd, channelName);
}

void Server::notifyPart(const int &clientFd, const std::string &channelName) {
	std::string buffer(":");

	buffer += this->Users[clientFd].getNickname() + "!";
	buffer += this->Users[clientFd].getUsername() + "@";
	buffer += "localhost PART ";
	buffer += channelName + " :";
	buffer += MSG_PART;
	buffer += "\r\n";

	for (std::vector<Channel>::iterator it = channelList.begin(); it != channelList.end(); ++it) {
		if (it->getName() == channelName) {
			this->broadcastToAllMember(*it, buffer);
			it->removeMember(clientFd);
			return ;
		}
	}
}
