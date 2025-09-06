/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rbardet- <rbardet-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/20 14:39:17 by rbardet-          #+#    #+#             */
/*   Updated: 2025/09/04 22:22:59 by rbardet-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <sstream>

#define PORT 1
#define PASSWORD 2

#define SERV_NAME "ircserv"

#define MODE_CMD 5
#define NICK_CMD 5
#define USER_CMD 5
#define PASS_CMD 5

#define MODE_CHAR 1

#define REMOVE_LIMIT 0

#define RPL_WELCOME "001"
#define RPL_YOURHOST "002"
#define RPL_CREATED "003"
#define RPL_NAMREPLY "353"
#define RPL_ENDOFNAMES "366"

#define ERR_NOSUCHCHANNEL "403"
#define ERR_CANNOTSENDTOCHAN "404"
#define ERR_UNKNOWNCOMMAND "421"
#define ERR_NONICKNAMEGIVEN "431"
#define ERR_NICKCOLLISION "436"
#define ERR_USERNOTINCHANNEL "441"
#define ERR_NOTREGISTERED "451"
#define ERR_NEEDMOREPARAMS "461"
#define ERR_NOPERMFORHOST "463"
#define ERR_PASSWDMISMATCH "464"
#define ERR_INVALIDUSERNAME "468"
#define ERR_KICKEDFROMCHAN	"470"
#define ERR_CHANNELISFULL "471"
#define ERR_UNKNOWNMODE "472"
#define ERR_INVITEONLYCHAN "473"
#define ERR_BADCHANNELKEY "475"
#define ERR_NOPRIVILEGES "481"

const std::string getParam(int cmdLength, const std::string &line);
