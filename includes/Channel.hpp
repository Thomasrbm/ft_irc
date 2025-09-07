#pragma once

#include <iostream>
#include <string>
#include <set>
#include <vector>
#include <ctime>

class Channel
{
private:
	std::string		name;
	std::string		key;			// mode k (mot de passe du channel)
	bool			invite_only;	// mode i (invitation uniquement)
	bool			topic_op_only;	// mode t (si actif seuls les ops peuvent changer le topic)
	bool			has_key;		// mode k actif ? = les passwords
	int				user_limit;		// mode l (limite d'utilisateurs, 0 = pas de limite)

	std::string		topic;
	std::string		topicSetter;
	time_t			topicTimeSet;

	// chaque user a un fd unique dont on regarde que le fd.  dont tab d int
	int				host;
	std::set<int>	operators;		//
	std::set<int>	users;			//
	std::set<int>	invited;		// fd des utilisateurs invit (si mode i)

public:
	Channel();
	Channel(const std::string &name, int creatorFd);
	Channel(const Channel &src);
	Channel &operator=(const Channel &src);
	~Channel();

	int				user_joined;


	// ===== GESTION DES MEMBRES =====
	bool	canJoin(int fd, const std::string &key, std::string &reason) const;
	bool	addMember(int fd);
	bool	removeMember(int fd);
	bool	isMember(int fd) const;
	bool	isEmpty() const;
	bool	hasPerm(const int & clientFd) const;
	bool	isHost(const int &clientFd) const;

	// ===== GESTION DES OPÉRATEURS =====
	bool	isOperator(int fd) const;
	bool	addOperator(int fd);
	bool	removeOperator(int fd);

	// ===== GESTION DES MODES (selon le sujet) =====
	void	setInviteOnly(bool on);		// mode i
	void	setTopicOpOnly(bool on);	// mode t
	void	setKey(const std::string &key);	// mode k
	void	clearKey();					// mode k
	void	setUserLimit(int limit);	// mode l
	void	resetUserLimit();			// mode l

	// ===== GESTION DU TOPIC =====
	bool	setTopic(int byFd, const std::string &topic, const std::string &setterName); // respecte le mode t
	const std::string &getTopic() const;

	// ===== GESTION DES INVITATIONS =====
	void	invite(int fd);
	bool	isInvited(int fd) const;
	void	clearInvite(int fd);


	// ===== UTILITAIRES =====
	const std::string	&getName() const;
	std::vector<int>	getAllMembers() const;
	const int 			&getHost() const;
	bool				isSpace();

	// ===== GETTERS POUR LES MODES =====
	bool	getInviteOnly() const;
	bool	getTopicOpOnly() const;
	bool	getHasKey() const;
	int		getUserLimit() const;
	const std::string &getKey() const;

	const std::string &getTopicSetter() const {return(this->topicSetter);};
	const time_t &getTopicTime() const {return(this->topicTimeSet);};
};
