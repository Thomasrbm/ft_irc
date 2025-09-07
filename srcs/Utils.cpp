#include "../includes/Utils.hpp"

const std::string getParam(int cmdLength, const std::string &line) {
	std::string args = line.substr(cmdLength);
	std::string firstParam = args.substr(0, args.find(' '));
	return(firstParam);
}


const std::string getChannelName(const std::string &line) {
	const size_t channelPos = line.find('#');
	if (channelPos == std::string::npos) {
		return (EMPTY_STRING);
	}

	const std::string tmp = line.substr(channelPos, line.length());

	const std::string channelName = tmp.substr(0, tmp.find(' '));

	return (channelName);
}
