#pragma once

#include <iostream>
#include <poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <set>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <signal.h>
#include <map>
#include <fcntl.h>
#include <vector>

#include "User.hpp"
#include "Channel.hpp"
#include "Utils.hpp"

#define MAX_USER 1024
#define MAX_EVENTS 10

class Server
{
private:
	int			port;
	int			socketfd;
	std::string	password;
	std::vector<Channel>	channelList;
	int			epollFd;
	epoll_event	event;
	epoll_event	events[MAX_EVENTS];
	std::map<int, User>	Users;
	User		ircBot;
public:
	Server();
	Server(const Server &src);
	Server operator=(const Server &src);
	~Server();

	static bool running;
	void	initSocket();
	void	initEpoll();
	void	initServer(const int &port, const std::string &password);
	void	runServer();
	static void	signalHandler(int signum);
	void	handleCapReq(const int &userFd) const;

	int 			findIdByName(const std::string &name) const;
	std::string		findNameById(const int &clientFd) const;
	Channel			&findChannelByName(const std::string &channelName);

	bool	nickAlreadyInUse(const std::string &nick);
	void	welcomeUser(const int &code, const std::string &name) const;
	void	acceptUser();
	void	parseInput(int userFd);
	bool	hasPassword() const;
	void	sendRPL(const int &clientFd, const std::string code, const std::string &nick, const std::string &message) const;

	void	handleNick(const int &clientFd, const std::string &line);
	void	handleUsername(const int &clientFd, const std::string &line);
	void	handleLine(const int &clientFd, const std::string &line);
	void	handleJoin(const int &clientFd, const std::string &line);
	void	handlePass(const int &clientFd, const std::string &line);
	void	handleTopic(const int &clientFd, const std::string &line);
	void	handleKick(const int &clientFd, const std::string &line);
	void	handlePing(const int &clientFd, const std::string &line);
	void	handleInvite(const int &clientFd, const std::string &line);
	void	handlePart(const int &clientFd, const std::string &line);
	void	handleMode(int clientFd, const std::string &line);

	// KICK
	const	std::string getUserToKick(const std::string &line) const;
	const	std::string getReason(const std::string &line) const;
	void	broadcastKickConfirmation(const std::string &channelName, const std::string &kicker, const std::string &victim, const std::string &reason);

	const	std::string getTopic(const std::string &line) const;
	void	sendTopic(const int &clientFd, const Channel &channel);
	void	broadcastNewTopic(const std::string &topic, const std::string &channelName, const int &clientFd, Channel &channel);

	//   JOIN
	std::vector<std::string>	parseJoinChannelName(const std::string &line);
	bool						channelExists(const std::string &channelName);
	void						createChannel(const std::string &channelName, int creatorFd);
	bool						joinExistingChannel(const std::string &channelName, const std::string &key, int userFd);
	// Messages dans les channels (chat normal IRC) irssi écrit privmsg direct
	void		broadcastToChannel(const std::string &channelName, const std::string &message, int senderFd);
	void		sendChannelError(const int &clientFd, const std::string &code, const std::string &nick, const std::string &channel, const std::string &message) const;

	// messages prive /msg
	void		handlePrivateMessage(int clientFd, const std::string &line);
	void		sendPrivateMessage(const std::string &targetNick, const std::string &message, int senderFd);

	// DCC - Direct Client-to-Client file transfermake
	void		handleDccSend(int clientFd, const std::string &line);
	void		handleAccept(int clientFd, const std::string &line);
	void		sendDccOffer(const std::string &targetNick, const std::string &filename, int fileSize, int port, int senderFd);
	void		sendDccAccept(const std::string &targetNick, const std::string &filename, int port, int senderFd);

	//  MODE
	void 		execMode(int clientFd, const std::string &channelName, const std::string &mode, std::string arg);
	void 		setMode(int clientFd, const std::string &channelName, char mode, bool set_or_unset, std::string arg);
	char		extractFlag(const std::string &mode);

	// RFC confirmations
	void		notifyJoin(const std::string &channelName, int clientFd);
	void		broadcastToAllMember(Channel chanel, const std::string message);

	void	processToInvite(const int &clientFd, const std::string &toInvite, Channel &channel);

	void	notifyInvite(const int &clientFd, const std::string &toInvite, Channel &channel);
	void	notifyMode(const int &clientFd, const std::string &channelName, const char &mode, const bool status, const std::string &arg);
	void	notifyPart(const int &clientFd, const std::string &channelName);

	void	sendRPL_CHANNELMODEIS(const int &clientFd, const Channel &channel);
	void	sendRPL_TOPICWHOTIME(const int &clientFd, const Channel &channel);
	void	sendRPL_TOPIC(const int &clientFd, const Channel &channel);
	void	sendRPL_NOTOPIC(const int &clientFd, const Channel &channel);
	void	sendRPL_INVITED(const int &clientFd, const std::string &toInvite, const Channel &channel);
	void	sendERR_CHANOPRIVSNEEDED(const int &clientFd, const std::string &channelName);
	void	sendERR_NEEDMOREPARAMS(const int &clientFd, const std::string &command);
	void	sendERR_NOSUCHCHANNEL(const int &clientFd, const std::string &channelName);
	void	sendRPL_NAMEREPLY(const int &clientFd, Channel &channel);
	void	sendRPL_ENDOFNAMES(const int &clientFd, Channel &channel);
};