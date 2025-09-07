#include "../../includes/Server.hpp"
#include "../../includes/Utils.hpp"


void Server::handlePrivateMessage(int clientFd, const std::string &line)
{
	size_t idx_after_command = line.find(' ');
	if (idx_after_command == std::string::npos)
		return;

	size_t idx_after_target = line.find(' ', idx_after_command + 1);
	if (idx_after_target == std::string::npos)
		return;

	std::string target = line.substr(idx_after_command + 1, idx_after_target - idx_after_command - 1);
	std::string message = line.substr(idx_after_target + 1);

	if (!message.empty() && message[0] == ':')
		message.erase(0, 1);

	if (target.empty())
		return;

	// si le destinateur commence par # ou & = channel
	if (target[0] == '#' || target[0] == '&') 
	{
		broadcastToChannel(target, message, clientFd);
		return;
	}

	sendPrivateMessage(target, message, clientFd);
}

void Server::broadcastToChannel(const std::string &channelName, const std::string &message, int senderFd)
{
	for (std::vector<Channel>::iterator it = channelList.begin(); it != channelList.end(); ++it)
	{
		if (it->getName() == channelName)
		{
			if (!it->isMember(senderFd))
			{
				sendRPL(senderFd, ERR_NOSUCHCHANNEL, this->findNameById(senderFd), "Cannot send to channel");
				return;
			}

			std::string nick = Users.at(senderFd).getNickname();
			std::string user = Users.at(senderFd).getUsername();
			std::string host = "localhost";

			std::string full = ":" + nick + "!" + user + "@" + host + " PRIVMSG " + channelName + " :" + message + "\r\n";

			std::vector<int> members = it->getAllMembers();
			for (std::vector<int>::iterator m = members.begin(); m != members.end(); ++m)
			{
				if (*m == senderFd)
					continue;
				send(*m, full.c_str(), full.length(), 0);
			}
			return;
		}
	}
	sendERR_NOSUCHCHANNEL(senderFd, channelName);
}


void Server::sendPrivateMessage(const std::string &targetNick, const std::string &message, int senderFd) 
{
	int targetFd = findIdByName(targetNick);
	if (targetFd == -1) {
		// Utilisateur introuvable
		sendRPL(senderFd, ERR_NOSUCHNICK, this->findNameById(senderFd), MSG_ERR_NOSUCHNICK);
		return;
	}

	std::string nick = Users.at(senderFd).getNickname();
	std::string user = Users.at(senderFd).getUsername();
	std::string host = "localhost";

	std::string full = ":" + nick + "!" + user + "@" + host + " PRIVMSG " + targetNick + " :" + message + "\r\n";
	send(targetFd, full.c_str(), full.length(), 0);
}

