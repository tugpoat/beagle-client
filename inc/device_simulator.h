#ifndef DEVICESIMULATOR_H
#define DEVICESIMULATOR_H

#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>

#include "spdlog/spdlog.h"
#include "ghc/filesystem.hpp"
#include "archive_reader.hpp"
#include "archive_writer.hpp"
#include "archive_exception.hpp"

#include "global.h"
#include "base64.h"
#include "utility.h"


struct DeviceSimulatorSettings
{
	std::string saveDataBasePath;
	std::string saveDataPath;
};
class DeviceSimulator
{
public:
	virtual bool readConfig(std::string);
	virtual bool DeviceInserted();
	virtual bool DeviceReady();
	virtual bool InsertDevice();
	virtual bool RemoveDevice();
	std::string getSaveDataPath();
protected:
	DeviceSimulatorSettings mDevSettings;
private:
};

extern std::atomic<bool> running;
#endif