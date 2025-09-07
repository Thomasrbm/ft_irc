#include "../../includes/Server.hpp"
#include "../../includes/Utils.hpp"

const std::string Server::getUserToKick(const std::string &line) const {
	const size_t channelPos = line.find('#');
	if (channelPos == std::string::npos)
		return (EMPTY_STRING);

	const std::string tmp = line.substr(channelPos);

	size_t skipChannel = tmp.find(' ');
	if (skipChannel == std::string::npos)
		return (EMPTY_STRING);

	size_t userStart = skipChannel + 1;

	size_t userEnd = tmp.find(' ', userStart);
	if (userEnd == std::string::npos)
		userEnd = tmp.length();

	const std::string user = tmp.substr(userStart, userEnd - userStart);
	return (user);
}

const std::string Server::getReason(const std::string &line) const {
	const size_t reasonPos = line.find(':');
	if (reasonPos == std::string::npos) {
		return("No reason");
	}

	const std::string reason = line.substr(reasonPos + 1);

	return (reason);
}

void Server::handleKick(const int &clientFd, const std::string &line) {
	const std::string channelName = getChannelName(line);
	const std::string kick = getUserToKick(line);
	const std::string reason = getReason(line);
	const int kickId = findIdByName(kick);

	if (kickId == -1) {
		sendRPL(clientFd, ERR_INVALIDUSERNAME, this->Users[clientFd].getNickname(), MSG_ERR_WRONGUSER);
		return ;
	}


	if (kick.empty()) {
		sendERR_NEEDMOREPARAMS(clientFd, CMD_KICK);
		return ;
	}

	else {
		for (std::vector<Channel>::iterator it = channelList.begin(); it != channelList.end(); ++it) {
			if (it->getName() == channelName) {
				if (!it->hasPerm(clientFd)) {
					sendERR_CHANOPRIVSNEEDED(clientFd, channelName);
					return ;
				} else if (!it->isMember(kickId)) {
					sendRPL(clientFd, ERR_USERNOTINCHANNEL, this->Users[clientFd].getNickname(), MSG_ERR_USERNOTINCHANNEL);
					return ;
				} else {
					std::string kicker = findNameById(clientFd);
					broadcastKickConfirmation(channelName, kicker, kick, reason);
					it->removeMember(kickId);
					return ;
				}
			}
		}
	}
}


void Server::broadcastKickConfirmation(const std::string &channelName, const std::string &kicker, const std::string &victim, const std::string &reason) {
	for (std::vector<Channel>::const_iterator it = channelList.begin(); it != channelList.end(); ++it) {
		if (it->getName() == channelName) {
			std::string kickerUser = Users.at(findIdByName(kicker)).getUsername();
			std::string kickerHost = "localhost";

			const std::string kickMsg = ":" + kicker + "!" + kickerUser + "@" + kickerHost + " KICK " + channelName + " " + victim + " :" + reason + "\r\n";

			broadcastToAllMember(*it, kickMsg);
			return ;
		}
	}
}
