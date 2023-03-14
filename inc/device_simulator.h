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
	DeviceSimulator() {}
	virtual ~DeviceSimulator() {};
	virtual bool readConfig(std::string) {return false;};
	virtual bool isDeviceInserted() {return false;};
	virtual bool DeviceReady() {return false;};
	virtual bool InsertDevice() {return false;};
	virtual bool RemoveDevice() {return false;};
	std::string getSaveDataPath() { return m_DevSettings.saveDataPath; };
protected:
	DeviceSimulatorSettings m_DevSettings;
private:
};

extern std::atomic<bool> running;
#endif