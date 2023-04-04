#include "yacardemu_client.h"

YACardEmuClient::YACardEmuClient() : DeviceSimulator()
{
spdlog::info("Constructing YACardEmuClient");
}

YACardEmuClient::YACardEmuClient(std::string cfgfile) : DeviceSimulator()
{
	spdlog::info("Constructing YACardEmuClient with config {}", cfgfile);
	if (this->readConfig(cfgfile)) {
		this->cli = new httplib::Client(this->m_settings.apiHost, this->m_settings.apiPort);
	}
}

YACardEmuClient::~YACardEmuClient()
{
	delete this->cli;
}

bool YACardEmuClient::readConfig(std::string cfgfile)
{
	mINI::INIFile config(cfgfile);
	mINI::INIStructure ini;

	if (!config.read(ini)) {
		spdlog::critical("Unable to open %s", cfgfile.c_str());
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

//FIXME: needs a status type enum to accomodate for errors properly
bool YACardEmuClient::isDeviceInserted()
{
	//httplib::Client cli(this->m_settings.apiHost, this->m_settings.apiPort);

	auto res = this->cli->Get("/api/v1/hasCard");

	if (!res) {
		spdlog::error("YACardEmuClient::isDeviceInserted() - Request failed!");
		return false;
	}
	if (res->body == "true") {
		spdlog::info("hasCard=true");
		return true;
	}
	else if (res->body == "false")
		spdlog::info("hasCard=false");
	
	return false;
}

bool YACardEmuClient::DeviceReady()
{
	//httplib::Client cli(this->m_settings.apiHost, this->m_settings.apiPort);
	auto res = this->cli->Get("/api/v1/readyCard");

	if (!res) {
		spdlog::error("YACardEmuClient::DeviceReady() - Request failed!");
		return false;
	}
	if (res->body == "true")
	{
		spdlog::info("readyCard=true");
		return true;
	}
	else if (res->body == "false")
		spdlog::info("readyCard=false");
	
	return false;
}

bool YACardEmuClient::InsertDevice(std::string name = "card")
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
	auto res = cli.Post("/v1/savedata/push", headers, items, "boundaryhuehue");
	if (!res) {
		spdlog::error("YACardEmuClient::InsertDevice() - Request failed!");
		return false;
	}

	return true;
}

bool YACardEmuClient::RemoveDevice()
{
	spdlog::critical("RemoveDevice() Invalid for this driver");
	return true;
}