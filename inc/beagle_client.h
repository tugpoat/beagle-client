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
#include "ghc/filesystem.hpp"
#include "archive_reader.hpp"
#include "archive_writer.hpp"
#include "archive_exception.hpp"

#include "global.h"
#include "base64.h"
#include "utility.h"




class BeagleClient
{
public:
	std::string formatUid(const uint8_t *pbtData, const size_t szBytes);
	bool getAuthToken(httplib::Client &, AppSettings *);
	bool downloadSaveData(httplib::Client &, AppSettings *, std::string, std::string);
	bool uploadSaveData(httplib::Client &, AppSettings *, std::string, std::string);
	static std::string bytesToHexString(const unsigned char* str, const uint64_t s);
protected:
	
private:
	httplib::Client *cli;
	std::string apiToken;
	bool lastCardState;
};

extern std::atomic<bool> running;
#endif