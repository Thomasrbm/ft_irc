#pragma once

#include <iostream>
#include <sstream>

#define PORT 1
#define PASSWORD 2
#define IP 3

#define SERV_NAME ":server "

#define EMPTY_STRING ""

#define BUFFER_SIZE 1024

#define CMD_MODE "MODE"
#define CMD_NICK "NICK"
#define CMD_USER "USER"
#define CMD_PASS "PASS"
#define CMD_JOIN "JOIN"
#define CMD_KICK "KICK"
#define CMD_PRIVMSG "PRIVMSG"
#define CMD_TOPIC "TOPIC"
#define CMD_PING "PING"
#define CMD_PART "PART"
#define CMD_INVITE "INVITE"
#define CMD_CAP "CAP"
#define CMD_DCC "DCC"

#define MODE_CMD_LENGTH 5
#define NICK_CMD_LENGTH 5
#define USER_CMD_LENGTH 5
#define PASS_CMD_LENGTH 5
#define PING_CMD_LENGTH 5
#define INVITE_CMD_LENGTH 7
#define PART_CMD_LENGTH 5
#define CAP_CMD_LENGTH 4
#define DCC_CMD_LENGTH 4

#define MODE_FLAG 1

#define REMOVE_LIMIT 0

#define JUMP_TO_CHANNEL 1
#define JUMP_TO_MODE 1
#define JUMP_TO_ARG 4
#define JUMP_TO_TOPIC 2

#define RPL_WELCOME "001"
#define RPL_YOURHOST "002"
#define RPL_CREATED "003"
#define RPL_CHANNELMODEIS "324"
#define RPL_NOTOPIC "331"
#define RPL_TOPIC "332"
#define RPL_TOPICWHOTIME "333"
#define RPL_INVITING "341"
#define RPL_NAMREPLY "353"
#define RPL_ENDOFNAMES "366"

#define ERR_NOSUCHNICK "401"
#define ERR_NOSUCHCHANNEL "403"
#define ERR_CANNOTSENDTOCHAN "404"
#define ERR_NOORIGIN "409"
#define ERR_UNKNOWNCOMMAND "421"
#define ERR_NONICKNAMEGIVEN "431"
#define ERR_NICKNAMEINUSE "433"
#define ERR_NICKCOLLISION "436"
#define ERR_USERNOTINCHANNEL "441"
#define ERR_USERONCHANNEL "443"
#define ERR_NOTREGISTERED "451"
#define ERR_NEEDMOREPARAMS "461"
#define ERR_NOPERMFORHOST "463"
#define ERR_PASSWDMISMATCH "464"
#define ERR_YOUWILLBEBANNED "466"
#define ERR_INVALIDUSERNAME "468"
#define ERR_KICKEDFROMCHAN	"470"
#define ERR_CHANNELISFULL "471"
#define ERR_UNKNOWNMODE "472"
#define ERR_INVITEONLYCHAN "473"
#define ERR_BADCHANNELKEY "475"
#define ERR_NOPRIVILEGES "481"
#define ERR_CHANOPRIVSNEEDED "482"

#define MSG_ERR_NOSUCHNICK "Nickname does not exist"
#define MSG_ERR_PASSWDMISMATCH "Wrong password"
#define MSG_ERR_WRONGUSER "This user does not exist"
#define MSG_ERR_NOORIGIN "Error no messsage send by PING"
#define MSG_ERR_NEEDMOREPARAMS "Not enough parameters"
#define MSG_ERR_NOPERMS "You have no perm on this channel"
#define MSG_ERR_NOSUCHCHANEL "This channel does not exist"
#define MSG_ERR_USERNOTINCHANNEL "This user is not in the channel"
#define MSG_ERR_NICKNAMEINUSE "This nickname is already in use"
#define MSG_ERR_INVALIDNICK "Invalid charater in nickname"
#define MSG_ERR_UNKNOWNCOMMAND "Unknown command"
#define MSG_ERR_YOUWILLBEBANNED "You are kicked of the server"
#define MSG_NEED_PASS "This server has a password enter it before using it"
#define MSG_NEED_NICK "You have no nickname, enter one </nick>"
#define MSG_NEED_USER "You have no username, enter one </user>"
#define MSG_SUCCESS_KICK "The user got kicked from the channel"
#define MSG_NOTOPIC "No topic is set"
#define MSG_RPL_INVITED "has been invited by"
#define MSG_ERR_USERONCHANNEL "User already on the channel"
#define MSG_ERR_CHANOPRIVSNEEDED "You’re not channel operator"
#define MSG_ERR_UNKNOWNMODE "is unknown mode char to me"
#define MSG_PART "Leaved the channel"
#define MSG_RPL_ENDOFNAMES "End of /NAMES list"

const std::string getParam(int cmdLength, const std::string &line);
const	std::string getChannelName(const std::string &line);
