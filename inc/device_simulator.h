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

enum class DeviceSimulatorStatus
{
	ERROR = -1,
	SUCCESS = 0,
	DEVICE_INSERTED = 1,
	DEVICE_REMOVED = 2,
	DEVICE_READY = 3,
	DEVICE_NOT_READY = 4
};

class DeviceSimulator
{
public:
	DeviceSimulator() {}
	virtual ~DeviceSimulator() {};
	virtual bool readConfig(std::string) {return false;};
	virtual bool isDeviceInserted() {return false;};
	virtual bool isDeviceReady() {return false;};
	virtual DeviceSimulatorStatus InsertDevice() {return DeviceSimulatorStatus::ERROR;};
	virtual DeviceSimulatorStatus RemoveDevice() {return DeviceSimulatorStatus::ERROR;};
	std::string getSaveDataPath() { return m_DevSettings.saveDataPath; };
protected:
	virtual DeviceSimulatorStatus DeviceInserted() {return DeviceSimulatorStatus::ERROR;};
	virtual DeviceSimulatorStatus DeviceReady() {return DeviceSimulatorStatus::ERROR;};
	DeviceSimulatorSettings m_DevSettings;
private:
};

extern std::atomic<bool> running;
#endif