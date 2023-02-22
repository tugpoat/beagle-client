#ifndef YACARDEMUCLIENT_H
#define YACARDEMUCLIENT_H

#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>

#include "httplib.h"
#include "spdlog/spdlog.h"
#include "ghc/filesystem.hpp"
#include "archive_reader.hpp"
#include "archive_writer.hpp"
#include "archive_exception.hpp"

#include "global.h"
#include "device_simulator.h"
#include "utility.h"


struct YACardEmuSettings
{
	std::string targetDevice;
	std::string apiHost;
	std::string apiBaseUrl;
	unsigned int apiPort;
	std::string saveDataBasePath;
	std::string saveDataPath;
};
class YACardEmuClient : DeviceSimulator
{
public:
	bool readConfig(std::string);
	bool DeviceInserted();
	bool DeviceReady();
	bool InsertDevice();
	bool RemoveDevice();
protected:
	YACardEmuSettings settings;
private:
	httplib::Client *cli;
	std::string apiToken;
	bool lastCardState;
};

extern std::atomic<bool> running;
#endif