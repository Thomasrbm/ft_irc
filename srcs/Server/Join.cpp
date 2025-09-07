#include "../../includes/Server.hpp"
#include "../../includes/Utils.hpp"


void Server::handleJoin(const int &clientFd, const std::string &line) {
	std::vector<std::string> channelName_andkey = parseJoinChannelName(line);

	if (channelName_andkey[0].empty()) {
		std::cout << "Invalid JOIN command received from user " << clientFd << std::endl;
		return;
	}

	if (channelExists(channelName_andkey[0])) {
		if (joinExistingChannel(channelName_andkey[0], channelName_andkey[1], clientFd)) {
			std::cout << "User " << clientFd << " joined existing channel: " << channelName_andkey[0] << std::endl;
			notifyJoin(channelName_andkey[0], clientFd);
			sendRPL_NAMEREPLY(clientFd, this->findChannelByName(channelName_andkey[0]));
			sendRPL_ENDOFNAMES(clientFd, this->findChannelByName(channelName_andkey[0]));
		}
	}
	else {
		createChannel(channelName_andkey[0], clientFd);
		std::cout << "New channel created: " << channelName_andkey[0] << " by user " << clientFd << std::endl;
		notifyJoin(channelName_andkey[0], clientFd);
		sendRPL_NAMEREPLY(clientFd, this->findChannelByName(channelName_andkey[0]));
		sendRPL_ENDOFNAMES(clientFd, this->findChannelByName(channelName_andkey[0]));
	}
}


std::vector<std::string> Server::parseJoinChannelName(const std::string &line)
{
	std::vector<std::string> channelName_andkey;

	// Format attendu: JOIN #channelname ou avec &
	size_t spacePos = line.find(' ');

	if (spacePos == std::string::npos)
		return channelName_andkey;

	// Resize sinon segfault
	channelName_andkey.resize(2);

	channelName_andkey[0] = line.substr(spacePos + 1);

	size_t space_after_channelName = line.find(' ', spacePos + 1);
	if (space_after_channelName == std::string::npos)
		return channelName_andkey;

	std::string key;

	if (space_after_channelName != std::string::npos)
		key = line.substr(space_after_channelName + 1);

	// Le nom du canal est tout ce qui est avant l'espace
	channelName_andkey[0] = line.substr(spacePos + 1, space_after_channelName - spacePos - 1);

	// Si une clé est présente, on la stocke
	channelName_andkey[1] = key;

	return channelName_andkey;
}



bool Server::channelExists(const std::string &channelName) {
	for (std::vector<Channel>::iterator it = channelList.begin(); it != channelList.end(); ++it) {
		if (it->getName() == channelName)
			return (true);
	}
	return (false);
}

void Server::createChannel(const std::string &channelName, int creatorFd) {
	Channel newChannel(channelName, creatorFd);
	this->channelList.push_back(newChannel);
}

void Server::sendChannelError(const int &clientFd, const std::string &code, const std::string &nick, const std::string &channel, const std::string &message) const {
	std::string buffer = ":server " + code + " " + nick + " " + channel + " :" + message + "\r\n";
	send(clientFd, buffer.c_str(), buffer.size(), 0);
}

bool Server::joinExistingChannel(const std::string &channelName,  const std::string &key, int userFd) {
	for (std::vector<Channel>::iterator it = channelList.begin(); it != channelList.end(); ++it) {
		if (it->getName() == channelName) {
			std::string reason;
			if (!it->canJoin(userFd, key, reason))
			{
				if (reason.find("limit") != std::string::npos) {
					sendChannelError(userFd, ERR_CHANNELISFULL, findNameById(userFd), channelName, "Cannot join channel (+l)");
				}
				else if (reason.find("invited") != std::string::npos) {
					sendChannelError(userFd, ERR_INVITEONLYCHAN, findNameById(userFd), channelName, "Cannot join channel (+i)");
				}
				else if (reason.find("password") != std::string::npos) {
					sendChannelError(userFd, ERR_BADCHANNELKEY, findNameById(userFd), channelName, "Cannot join channel (+k)");
				}
				return false;
			}
			it->addMember(userFd);
			this->sendTopic(userFd, *it);
			this->sendRPL_CHANNELMODEIS(userFd, *it);
			return true;
		}
	}
	return false;
}


void Server::notifyJoin(const std::string &channelName, int clientFd)
{
	std::string buffer(":");
	buffer += this->Users[clientFd].getNickname() + "!";
	buffer += this->Users[clientFd].getUsername() + "@";
	buffer += "localhost JOIN ";
	buffer += channelName + "\r\n";

	Channel &channel = this->findChannelByName(channelName);

	broadcastToAllMember(channel, buffer);
}

