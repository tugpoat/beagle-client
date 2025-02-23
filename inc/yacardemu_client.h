#ifndef YACARDEMUCLIENT_H
#define YACARDEMUCLIENT_H

#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>

#include "httplib.h"
#include "spdlog/spdlog.h"
#include "spdlog/async.h"
#include "mini/ini.h"
#include "ghc/filesystem.hpp"
#include "archive_reader.hpp"
#include "archive_writer.hpp"
#include "archive_exception.hpp"

#include "global.h"
#include "device_simulator.h"
#include "utility.h"

extern std::atomic<bool> running;
extern std::shared_ptr<spdlog::async_logger> g_logger;

struct YACardEmuSettings
{
	std::string targetDevice;
	std::string apiHost;
	std::string apiBaseUrl;
	unsigned int apiPort;
	std::string saveDataBasePath;
	std::string saveDataPath;
	std::string defaultCardName;
};

class YACardEmuClient : public DeviceSimulator
{
public:
	YACardEmuClient();
	YACardEmuClient(std::string);
	~YACardEmuClient();
	bool readConfig(std::string) override;
	bool isDeviceInserted() override;
	bool isDeviceReady() override;
	DeviceSimulatorStatus InsertDevice(std::string);
	DeviceSimulatorStatus RemoveDevice() override;
protected:
	DeviceSimulatorStatus DeviceInserted() override;
	DeviceSimulatorStatus DeviceReady() override;
	YACardEmuSettings m_settings;
private:
	std::unique_ptr<httplib::Client> m_http_cli;
	std::string apiToken;
	bool lastCardState;
};

#endif