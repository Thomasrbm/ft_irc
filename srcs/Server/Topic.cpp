#include "../../includes/Server.hpp"
#include "../../includes/Utils.hpp"

const std::string Server::getTopic(const std::string &line) const {
	const size_t topicPos = line.find(" :");
	if (topicPos == std::string::npos) {
		return (EMPTY_STRING);
	}

	const std::string topic = line.substr(topicPos + JUMP_TO_TOPIC);

	return (topic);
}

void Server::broadcastNewTopic(const std::string &topic, const std::string &channelName, const int &clientFd, Channel &channel) {
	const std::string nick = this->Users[clientFd].getNickname();
	const std::string user = this->Users[clientFd].getUsername();
	const std::string host = "localhost";
	const std::string msg = ":" + nick + "!" + user + "@" + host + " TOPIC " + channelName + " :" + topic + "\r\n";

	broadcastToAllMember(channel, msg);
}

void Server::handleTopic(const int &clientFd, const std::string &line)
{
	const std::string channelName = getChannelName(line);

	if (!this->channelExists(channelName)) {
		sendERR_NOSUCHCHANNEL(clientFd, channelName);
		return ;
	}

	const std::string topic = getTopic(line);

		for (std::vector<Channel>::iterator it = channelList.begin(); it != channelList.end(); ++it) {
			if (it->getName() == channelName) {
				if (topic.empty()) {
					sendTopic(clientFd, *it);
					return ;
				} else {
					if (!it->setTopic(clientFd, topic, this->Users[clientFd].getNickname())) {
						sendERR_CHANOPRIVSNEEDED(clientFd, channelName);
					} else {
						broadcastNewTopic(topic, channelName, clientFd, *it);
					}
					return ;
				}
			}
		}
}

void Server::sendTopic(const int &clientFd, const Channel &channel) {

	if (channel.getTopic().empty()) {
		this->sendRPL_NOTOPIC(clientFd, channel);
	} else {
		this->sendRPL_TOPIC(clientFd, channel);
		this->sendRPL_TOPICWHOTIME(clientFd, channel);
	}
}
