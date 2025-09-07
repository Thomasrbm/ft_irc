#include "../../includes/Server.hpp"
#include "../../includes/Utils.hpp"
#include <sys/stat.h>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cstdio>

void Server::handleDccSend(int clientFd, const std::string &line) {
	// Format attendu: "DCC SEND <filename> <filesize> <port> <token>"
	// ou via PRIVMSG: "PRIVMSG <target> :\x01DCC SEND <filename> <filesize> <port> <token>\x01"
	
	std::string target, filename, filesizeStr, portStr, token;
	
	// Parser la ligne DCC
	size_t pos = line.find("DCC SEND");
	if (pos == std::string::npos) {
		sendRPL(clientFd, ERR_NEEDMOREPARAMS, this->findNameById(clientFd), "DCC SEND requires parameters");
		return;
	}
	
	// Extraire les paramètres après "DCC SEND"
	std::string params = line.substr(pos + 8); // +8 pour "DCC SEND"
	
	// Nettoyer les espaces en début
	while (!params.empty() && params[0] == ' ') {
		params.erase(0, 1);
	}
	
	// Parser les paramètres
	std::istringstream iss(params);
	std::vector<std::string> tokens;
	std::string token_temp;
	
	while (iss >> token_temp) {
		tokens.push_back(token_temp);
	}
	
	if (tokens.size() < 4) {
		sendRPL(clientFd, ERR_NEEDMOREPARAMS, this->findNameById(clientFd), "DCC SEND requires: filename filesize port token");
		return;
	}
	
	filename = tokens[0];
	filesizeStr = tokens[1];
	portStr = tokens[2];
	token = tokens[3];
	
	// Vérifier que le fichier existe et obtenir sa taille réelle
	struct stat fileStat;
	if (stat(filename.c_str(), &fileStat) != 0) {
		sendRPL(clientFd, ERR_NOSUCHNICK, this->findNameById(clientFd), "File not found: " + filename);
		return;
	}
	
	int actualFileSize = fileStat.st_size;
	int claimedFileSize = atoi(filesizeStr.c_str());
	
	// Vérifier que la taille déclarée correspond à la taille réelle
	if (actualFileSize != claimedFileSize) {
		sendRPL(clientFd, ERR_NOSUCHNICK, this->findNameById(clientFd), "File size mismatch");
		return;
	}
	
	// Extraire le destinataire de la ligne originale
	// Format: "PRIVMSG <target> :\x01DCC SEND ...\x01"
	size_t privmsgPos = line.find("PRIVMSG");
	if (privmsgPos != std::string::npos) {
		size_t targetStart = privmsgPos + 7; // +7 pour "PRIVMSG"
		while (targetStart < line.length() && line[targetStart] == ' ') targetStart++;
		
		size_t targetEnd = line.find(' ', targetStart);
		if (targetEnd != std::string::npos) {
			target = line.substr(targetStart, targetEnd - targetStart);
		}
	}
	
	if (target.empty()) {
		sendRPL(clientFd, ERR_NEEDMOREPARAMS, this->findNameById(clientFd), "DCC SEND requires target");
		return;
	}
	
	// Vérifier que le destinataire existe
	int targetFd = findIdByName(target);
	if (targetFd == -1) {
		sendRPL(clientFd, ERR_NOSUCHNICK, this->findNameById(clientFd), "Target user not found");
		return;
	}
	
	// Convertir le port en int
	int port = atoi(portStr.c_str());
	if (port <= 0 || port > 65535) {
		sendRPL(clientFd, ERR_NOSUCHNICK, this->findNameById(clientFd), "Invalid port number");
		return;
	}
	
	// Envoyer l'offre DCC au destinataire
	sendDccOffer(target, filename, actualFileSize, port, clientFd);
	
	// Confirmer l'envoi à l'expéditeur
	std::string senderNick = this->findNameById(clientFd);
	char sizeStr[32];
	sprintf(sizeStr, "%d", actualFileSize);
	std::string confirmation = "DCC SEND offer sent to " + target + " for file: " + filename + " (" + std::string(sizeStr) + " bytes)";
	sendRPL(clientFd, "DCC", senderNick, confirmation);
}

void Server::sendDccOffer(const std::string &targetNick, const std::string &filename, int fileSize, int port, int senderFd) {
	int targetFd = findIdByName(targetNick);
	if (targetFd == -1) {
		return; // L'utilisateur n'existe pas
	}
	
	std::string senderNick = this->findNameById(senderFd);
	std::string senderUser = Users.at(senderFd).getUsername();
	std::string host = "localhost";
	
	// Format DCC SEND selon RFC
	// :sender!user@host PRIVMSG target :\x01DCC SEND filename filesize port token\x01
	char fileSizeStr[32], portStr[32], tokenStr[32];
	sprintf(fileSizeStr, "%d", fileSize);
	sprintf(portStr, "%d", port);
	sprintf(tokenStr, "%d", rand());
	std::string dccMessage = "DCC SEND " + filename + " " + std::string(fileSizeStr) + " " + std::string(portStr) + " " + std::string(tokenStr);
	std::string fullMessage = ":" + senderNick + "!" + senderUser + "@" + host + " PRIVMSG " + targetNick + " :\x01" + dccMessage + "\x01\r\n";
	
	send(targetFd, fullMessage.c_str(), fullMessage.length(), 0);
	
	// Log pour debug
	std::cout << "DCC SEND offer sent from " << senderNick << " to " << targetNick << " for file: " << filename << std::endl;
}

void Server::handleDccAccept(int clientFd, const std::string &line) {
	// Format attendu: "DCC ACCEPT <filename> <port> <token>"
	// ou via PRIVMSG: "PRIVMSG <target> :\x01DCC ACCEPT <filename> <port> <token>\x01"
	
	std::string target, filename, portStr, token;
	
	// Parser la ligne DCC ACCEPT
	size_t pos = line.find("DCC ACCEPT");
	if (pos == std::string::npos) {
		sendRPL(clientFd, ERR_NEEDMOREPARAMS, this->findNameById(clientFd), "DCC ACCEPT requires parameters");
		return;
	}
	
	// Extraire les paramètres après "DCC ACCEPT"
	std::string params = line.substr(pos + 10); // +10 pour "DCC ACCEPT"
	
	// Nettoyer les espaces en début
	while (!params.empty() && params[0] == ' ') {
		params.erase(0, 1);
	}
	
	// Parser les paramètres
	std::istringstream iss(params);
	std::vector<std::string> tokens;
	std::string token_temp;
	
	while (iss >> token_temp) {
		tokens.push_back(token_temp);
	}
	
	if (tokens.size() < 3) {
		sendRPL(clientFd, ERR_NEEDMOREPARAMS, this->findNameById(clientFd), "DCC ACCEPT requires: filename port token");
		return;
	}
	
	filename = tokens[0];
	portStr = tokens[1];
	token = tokens[2];
	
	// Extraire le destinataire de la ligne originale
	size_t privmsgPos = line.find("PRIVMSG");
	if (privmsgPos != std::string::npos) {
		size_t targetStart = privmsgPos + 7; // +7 pour "PRIVMSG"
		while (targetStart < line.length() && line[targetStart] == ' ') targetStart++;
		
		size_t targetEnd = line.find(' ', targetStart);
		if (targetEnd != std::string::npos) {
			target = line.substr(targetStart, targetEnd - targetStart);
		}
	}
	
	if (target.empty()) {
		sendRPL(clientFd, ERR_NEEDMOREPARAMS, this->findNameById(clientFd), "DCC ACCEPT requires target");
		return;
	}
	
	// Vérifier que le destinataire existe
	int targetFd = findIdByName(target);
	if (targetFd == -1) {
		sendRPL(clientFd, ERR_NOSUCHNICK, this->findNameById(clientFd), "Target user not found");
		return;
	}
	
	// Convertir le port en int
	int port = atoi(portStr.c_str());
	if (port <= 0 || port > 65535) {
		sendRPL(clientFd, ERR_NOSUCHNICK, this->findNameById(clientFd), "Invalid port number");
		return;
	}
	
	// Envoyer l'acceptation DCC au destinataire
	sendDccAccept(target, filename, port, clientFd);
	
	// Confirmer l'acceptation à l'expéditeur
	std::string senderNick = this->findNameById(clientFd);
	char portStrConf[32];
	sprintf(portStrConf, "%d", port);
	std::string confirmation = "DCC ACCEPT sent to " + target + " for file: " + filename + " on port " + std::string(portStrConf);
	sendRPL(clientFd, "DCC", senderNick, confirmation);
}

void Server::sendDccAccept(const std::string &targetNick, const std::string &filename, int port, int senderFd) {
	int targetFd = findIdByName(targetNick);
	if (targetFd == -1) {
		return; // L'utilisateur n'existe pas
	}
	
	std::string senderNick = this->findNameById(senderFd);
	std::string senderUser = Users.at(senderFd).getUsername();
	std::string host = "localhost";
	
	// Format DCC ACCEPT selon RFC
	// :sender!user@host PRIVMSG target :\x01DCC ACCEPT filename port token\x01
	char portStr[32], tokenStr[32];
	sprintf(portStr, "%d", port);
	sprintf(tokenStr, "%d", rand());
	std::string dccMessage = "DCC ACCEPT " + filename + " " + std::string(portStr) + " " + std::string(tokenStr);
	std::string fullMessage = ":" + senderNick + "!" + senderUser + "@" + host + " PRIVMSG " + targetNick + " :\x01" + dccMessage + "\x01\r\n";
	
	send(targetFd, fullMessage.c_str(), fullMessage.length(), 0);
	
	// Log pour debug
	char portStrLog[32];
	sprintf(portStrLog, "%d", port);
	std::cout << "DCC ACCEPT sent from " << senderNick << " to " << targetNick << " for file: " << filename << " on port " << portStrLog << std::endl;
}
