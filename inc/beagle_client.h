#ifndef APICLIENT_H
#define APICLIENT_H

#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>

#include <nfc/nfc.h>

#include "httplib.h"
#include "spdlog/spdlog.h"
#include "spdlog/async.h"
#include "ghc/filesystem.hpp"
#include "archive_reader.hpp"
#include "archive_writer.hpp"
#include "archive_exception.hpp"

#include "global.h"
#include "base64.h"
#include "utility.h"

extern std::shared_ptr<spdlog::async_logger> g_logger;

class BeagleClient
{
public:
	std::string formatUid(const uint8_t *pbtData, const size_t szBytes);
	bool getAuthToken(httplib::Client &, AppSettings *);
	bool downloadSaveData(httplib::Client &, AppSettings *, std::string, std::string);
	bool uploadSaveData(httplib::Client &, AppSettings *, std::string, std::string);
protected:
	
private:
	std::unique_ptr<httplib::Client> m_http_cli;
	std::string apiToken;
	bool lastCardState;
};

extern std::atomic<bool> running;
#endif