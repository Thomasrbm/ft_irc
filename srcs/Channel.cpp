#include "../includes/Channel.hpp"

Channel::Channel()
{
	this->invite_only = false;
	this->topic_op_only = false;
	this->has_key = false;
	this->user_limit = 0;
	this->user_joined = 1;
}

Channel::Channel(const std::string &name, int creator)
{
	this->name = name;
	this->invite_only = false;
	this->topic_op_only = false;
	this->has_key = false;
	this->user_limit = 0;
	this->user_joined = 1;

	// le createur est membre + op direct a la creation
	this->host = creator;
	this->users.insert(creator);
	this->operators.insert(creator);
}

Channel::Channel(const Channel &src)
{
	*this = src;
}

Channel &Channel::operator=(const Channel &src)
{
	if (this == &src)
		return (*this);

	this->name = src.name;
	this->topic = src.topic;
	this->host = src.host;
	this->key = src.key;
	this->invite_only = src.invite_only;
	this->topic_op_only = src.topic_op_only;
	this->has_key = src.has_key;
	this->user_limit = src.user_limit;
	this->operators = src.operators;
	this->users = src.users;
	this->invited = src.invited;
	this->user_joined = src.user_joined;
	this->topicSetter = src.topicSetter;
	this->topicTimeSet = src.topicTimeSet;

	return (*this);
}

Channel::~Channel()
{
}


// ===== GESTION DES MEMBRES =====


// iterateurs pour les set avec find :
// Les iterateurs sont un peut comme des pointeur intelligent.
// Les set sont un peut comme des liste chaines ameliorer et l iterateur c est le ->next.
// il sait vers ou aller pour atteindre le prochain noeud du set.

// end est APRES le dernier element (un \0 mais pour pointeurs)

// find revoit un iterateur  l equivalent de l adresse / position du noeud.

bool Channel::canJoin(int fd, const std::string &key, std::string &reason) const
{
	// Mode i (invitation)
	if (this->invite_only && this->invited.find(fd) == this->invited.end())  // ca veut dire que find est aller a fin sans trouver fd
	{
		reason = "You cannot join channel because you have not been invited";
		return (false);
	}

	// Mode k (mot de passe)
	if (this->has_key && key != this->key)
	{
		reason = "You cannot join channel because the password is incorrect";
		return (false);
	}

	// Mode l (limite users)
	if (this->user_limit > 0 && (int)this->users.size() >= this->user_limit) // y a limit + elle est deja atteinte
	{
		reason = "You cannot join channel because it has reach its limit of users";
		return (false);
	}

	return (true);
}


// insert = renvoit paire iterateur +  bool    pair<iterator,bool>  donc  le .first = iterateur (position) vers new elem et

// .second  est le bool  qui sait si l insert a   success
// 	et false si etait deja present donc pas eu d insert

bool Channel::addMember(int fd)
{
	// return true si ajout reussi, false si deja membre
	return this->users.insert(fd).second;
}


// erase fait deja un find en lui meme donc pas besoin de faire avant
bool Channel::removeMember(int fd)
{
	// Retirer de tous les sets
	this->operators.erase(fd);
	this->invited.erase(fd);

	// return true si remove reussi, false si pas retire
	return this->users.erase(fd) > 0;
}

bool Channel::isMember(int fd) const
{
	return this->users.find(fd) != this->users.end();
}

bool Channel::isEmpty() const
{
	return this->users.empty();
}

// ===== GESTION DES OPÉRATEURS =====

bool Channel::isOperator(int fd) const
{
	return this->operators.find(fd) != this->operators.end();
}

bool Channel::addOperator(int fd)
{
	if (!this->isMember(fd))
		return (false);

	return this->operators.insert(fd).second;
}

bool Channel::removeOperator(int fd)
{
	return this->operators.erase(fd) > 0;
}

// ===== GESTION DES MODES =====

void Channel::setInviteOnly(bool on)
{
	this->invite_only = on;
}

void Channel::setTopicOpOnly(bool on)
{
	this->topic_op_only = on;
}

void Channel::setKey(const std::string &key)
{
	this->key = key;
	this->has_key = true;
}

void Channel::clearKey()
{
	this->key.clear();
	this->has_key = false;
}

void Channel::setUserLimit(int limit)
{
	this->user_limit = limit;
}

void Channel::resetUserLimit()
{
	this->user_limit = 0;
}

// ===== GESTION DU TOPIC =====

bool Channel::setTopic(int user, const std::string &topic, const std::string &setterName)
{
	// si op only pour modif topic verif que bien op
	if (this->topic_op_only && this->operators.find(user) == this->operators.end()) {
		std::cout << "Only operators can change the topic of this channel." << std::endl;
		return (false);
	}

	time(&this->topicTimeSet);
	this->topicSetter = setterName;
	std::cout << topicSetter << std::endl;
	this->topic = topic;
	return (true);
}

const std::string &Channel::getTopic() const
{
	return this->topic;
}

// ===== GESTION DES INVITATIONS =====

void Channel::invite(int fd)
{
	this->invited.insert(fd);
}

bool Channel::isInvited(int fd) const
{
	return this->invited.find(fd) != this->invited.end();
}

void Channel::clearInvite(int fd)
{
	this->invited.erase(fd);
}

// ===== UTILITAIRES =====

const std::string &Channel::getName() const
{
	return this->name;
}

const int &Channel::getHost() const { return (this->host); }

// find sert a chercher 1 element precis, pas iterer sur tous.

// donc on copie tout dans vector  pour brodcast a tous par exemple car le stockage est contigue ( tableau d int )
// alors que les element du set peuvent n importe ou dans la memoire car c est plus une liste chainee. donc moins opti
// pour iterer.
std::vector<int> Channel::getAllMembers() const
{
	std::vector<int> vector;

	for (std::set<int>::const_iterator it = this->users.begin(); it != this->users.end(); ++it)
		vector.push_back(*it); // derefenrence l iterateur envoit le contenu

	return vector;
}


bool Channel::isSpace()
{
	std::cout << "test" << std::endl;
	if (user_limit > 0)
	{
		if (user_joined == user_limit)
			return (false);
	}
	return (true);
}

// ===== GETTERS POUR LES MODES =====

bool Channel::getInviteOnly() const
{
	return this->invite_only;
}

bool Channel::getTopicOpOnly() const
{
	return this->topic_op_only;
}

bool Channel::getHasKey() const
{
	return this->has_key;
}

int Channel::getUserLimit() const
{
	return this->user_limit;
}

const std::string &Channel::getKey() const
{
	return this->key;
}

bool Channel::isHost(const int &clientFd) const {
	if (this->host == clientFd) {
		return (true);
	}
	return (false);
}

bool Channel::hasPerm(const int &clientFd) const {
	for (std::set<int>::iterator it = operators.begin(); it != operators.end(); ++it) {
		if (*it == clientFd) {
			return (true);
		}
	}
	return (false);
}
