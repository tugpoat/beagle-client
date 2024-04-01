#include "yacardemu_client.h"

YACardEmuClient::YACardEmuClient() : DeviceSimulator()
{
	g_logger->info("Constructing YACardEmuClient");
}

YACardEmuClient::YACardEmuClient(std::string cfgfile) : DeviceSimulator()
{
	g_logger->info("Constructing YACardEmuClient with config {}", cfgfile);
	if (this->readConfig(cfgfile)) {
		this->m_http_cli = std::make_unique<httplib::Client>(this->m_settings.apiHost, this->m_settings.apiPort);
	}
}

YACardEmuClient::~YACardEmuClient()
{

}

bool YACardEmuClient::readConfig(std::string cfgfile)
{
	mINI::INIFile config(cfgfile);
	mINI::INIStructure ini;

	if (!config.read(ini)) {
		g_logger->critical("Unable to open %s", cfgfile.c_str());
		return false;
	}

	if (ini.has("config")) {
		this->m_settings.apiHost = ini["config"]["apihost"];
		this->m_settings.apiPort = std::stoi(ini["config"]["apiport"]);
		this->m_settings.saveDataBasePath = ini["config"]["basepath"];
		this->m_settings.targetDevice = ini["config"]["targetdevice"];

		return true;
	}

	return false;
}

bool YACardEmuClient::isDeviceInserted()
{
	if (this->DeviceInserted() == DeviceSimulatorStatus::DEVICE_INSERTED)
		return true;
	
	return false;
}

DeviceSimulatorStatus YACardEmuClient::DeviceInserted()
{
	//httplib::Client cli(this->m_settings.apiHost, this->m_settings.apiPort);

	auto res = this->m_http_cli->Get("/api/v1/hasCard");

	if (!res) {
		g_logger->error("YACardEmuClient::isDeviceInserted() - Request failed!");
		return DeviceSimulatorStatus::ERROR;
	}
	if (res->body == "true") {
		g_logger->info("hasCard=true");
		return DeviceSimulatorStatus::DEVICE_INSERTED;
	}
	else if (res->body == "false") {
		g_logger->info("hasCard=false");
		return DeviceSimulatorStatus::DEVICE_REMOVED;
	}
	
	return DeviceSimulatorStatus::ERROR;
}

bool YACardEmuClient::isDeviceReady()
{
	if (this->DeviceInserted() == DeviceSimulatorStatus::DEVICE_READY)
		return true;
	
	return false;
}

DeviceSimulatorStatus YACardEmuClient::DeviceReady()
{
	//httplib::Client cli(this->m_settings.apiHost, this->m_settings.apiPort);
	auto res = this->m_http_cli->Get("/api/v1/readyCard");

	if (!res) {
		g_logger->error("YACardEmuClient::DeviceReady() - Request failed!");
		return DeviceSimulatorStatus::ERROR;
	}
	if (res->body == "true")
	{
		g_logger->info("readyCard=true");
		return DeviceSimulatorStatus::DEVICE_READY;
	}
	else if (res->body == "false") {
		g_logger->info("readyCard=false");
		return DeviceSimulatorStatus::DEVICE_NOT_READY;
	}
	
	// WTF?
	return DeviceSimulatorStatus::ERROR;
}

DeviceSimulatorStatus YACardEmuClient::InsertDevice(std::string name = "card")
{
	httplib::Client cli(this->m_settings.apiHost, this->m_settings.apiPort);

	httplib::Headers headers = {
		{ "DUMMY", "" }
	};

	httplib::MultipartFormDataItems items = {
		 { "loadonly", "", "", "" },
		 { "cardname", name, "", "" },
	};

	// Send request
	auto res = this->m_http_cli->Post("/v1/insertedCard", headers, items, "boundaryhuehue");
	if (!res) {
		g_logger->error("YACardEmuClient::InsertDevice() - Request failed!");
		return DeviceSimulatorStatus::ERROR;
	}

	return DeviceSimulatorStatus::DEVICE_INSERTED;
}

DeviceSimulatorStatus YACardEmuClient::RemoveDevice()
{
	g_logger->critical("RemoveDevice() Invalid for this driver");
	return DeviceSimulatorStatus::ERROR;
}