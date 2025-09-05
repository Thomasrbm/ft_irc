#include "../../includes/Server.hpp"
#include "../../includes/Utils.hpp"

void Server::handleJoin(int clientFd, const std::string &line)
{
	std::string channelName = parseJoinChannelName(line);

	if (channelName.empty()) {
		std::cout << "Invalid JOIN command received from user " << clientFd << std::endl;
		return;
	}

	if (channelExists(channelName)) {
		joinExistingChannel(channelName, clientFd);
		std::cout << "User " << clientFd << " joined existing channel: " << channelName << std::endl;
	}
	else {
		createChannel(channelName, clientFd);
		std::cout << "New channel created: " << channelName << " by user " << clientFd << std::endl;
	}
	
	// RFC confirmations - DIFFUSER À TOUS LES MEMBRES DU CANAL
	broadcastJoinToChannel(channelName, clientFd);
	sendNamesList(clientFd, channelName);
}

std::string Server::parseJoinChannelName(const std::string &line)
{
	// Format attendu: JOIN #channelname ou avec &
	size_t spacePos = line.find(' ');

	if (spacePos == std::string::npos)
		return "";

	std::string channelName = line.substr(spacePos + 1);

	// verif que commence par # ou &
	if (channelName.empty() || (channelName[0] != '#' && channelName[0] != '&'))
		return "";

	return channelName;
}


bool Server::channelExists(const std::string &channelName) {
	for (std::vector<Channel>::iterator it = channelList.begin(); it != channelList.end(); ++it) {
		if (it->getName() == channelName)
			return (true);
	}
	return (false);
}

void Server::createChannel(const std::string &channelName, int creatorFd) {
	channelList.push_back(Channel(channelName, creatorFd));
}

void Server::joinExistingChannel(const std::string &channelName, int userFd) {
	for (std::vector<Channel>::iterator it = channelList.begin(); it != channelList.end(); ++it) {
		if (it->getName() == channelName) {
			it->addMember(userFd);
			break;
		}
	}
}

// Fonctions utilitaires pour les confirmations RFC
void Server::broadcastJoinToChannel(const std::string &channelName, int clientFd) const {
    std::string nick = findNameById(clientFd);
    std::string user = Users.at(clientFd).getUsername();
    std::string host = "localhost"; // ou récupérer la vraie host
    
    // :nick!user@host JOIN :#chan
    std::string joinMsg = ":" + nick + "!" + user + "@" + host + " JOIN :" + channelName + "\r\n";
    
    // Diffuser à TOUS les membres du canal (y compris le joiner)
    for (std::vector<Channel>::const_iterator it = channelList.begin(); it != channelList.end(); ++it) {
        if (it->getName() == channelName) {
            std::vector<int> members = it->getAllMembers();
            for (size_t i = 0; i < members.size(); ++i) {
                send(members[i], joinMsg.c_str(), joinMsg.size(), 0);
            }
            break;
        }
    }
}

void Server::sendNamesList(int clientFd, const std::string &channelName) const {
    // RPL_NAMREPLY (353)
    std::string nick = findNameById(clientFd);
    std::string namesList = ":server " + std::string(RPL_NAMREPLY) + " " + nick + " = " + channelName + " :";
    
    // Construire la liste des noms avec préfixes d'opérateurs
    for (std::vector<Channel>::const_iterator it = channelList.begin(); it != channelList.end(); ++it) {
        if (it->getName() == channelName) {
            std::vector<int> members = it->getAllMembers();
            for (size_t i = 0; i < members.size(); ++i) {
                if (i > 0) namesList += " ";
                if (it->isOperator(members[i])) namesList += "@";
                namesList += findNameById(members[i]);
            }
            break;
        }
    }
    namesList += "\r\n";
    send(clientFd, namesList.c_str(), namesList.size(), 0);
    
    // RPL_ENDOFNAMES (366)
    std::string endNames = ":server " + std::string(RPL_ENDOFNAMES) + " " + nick + " " + channelName + " :End of /NAMES list\r\n";
    send(clientFd, endNames.c_str(), endNames.size(), 0);
}

void Server::broadcastKickConfirmation(const std::string &channelName, const std::string &kicker, const std::string &victim, const std::string &reason) const {
    // Trouver le channel et diffuser à tous les membres
    for (std::vector<Channel>::const_iterator it = channelList.begin(); it != channelList.end(); ++it) {
        if (it->getName() == channelName) {
            std::vector<int> members = it->getAllMembers();
            std::string kickerUser = Users.at(findIdByName(kicker)).getUsername();
            std::string kickerHost = "localhost";
            
            // :kicker!user@host KICK #chan victim :reason
            std::string kickMsg = ":" + kicker + "!" + kickerUser + "@" + kickerHost + " KICK " + channelName + " " + victim + " :" + reason + "\r\n";
            
            for (size_t i = 0; i < members.size(); ++i) {
                send(members[i], kickMsg.c_str(), kickMsg.size(), 0);
            }
            break;
        }
    }
}

