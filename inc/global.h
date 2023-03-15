#ifndef GLOBAL_H
#define GLOBAL_H

#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <string>

struct AppSettings
{
	std::string targetDevice;
	unsigned int gameId;
	std::string apiHost;
	std::string apiBaseUrl;
	unsigned int apiPort;
	std::string apiUser;
	std::string apiPassword;
	std::string apiToken;
	std::string uploadQueuePath;

	std::string nfc_uid;

	std::string apiListenHost;
	unsigned int apiListenPort;
	
};

#endif