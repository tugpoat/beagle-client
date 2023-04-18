#ifndef UTILITY_H
#define UTILITY_H

#include <vector>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <string>

#include "spdlog/spdlog.h"
#include "spdlog/async.h"
#include "ghc/filesystem.hpp"
#include "archive_reader.hpp"
#include "archive_writer.hpp"
#include "archive_exception.hpp"

#include "global.h"
#include "base64.h"

extern std::shared_ptr<spdlog::async_logger> g_logger;

struct membuf: std::streambuf // derive because std::streambuf constructor is protected
{
	membuf(char* p, size_t size)
	{
		setp( p, p + size); // set start end end pointers
	}
	size_t written() {return pptr()-pbase();} // how many bytes were really written?
};

std::string bytesToHexString(const unsigned char*, const uint64_t);
bool readFile(const char*, std::vector<char> &);
bool extractTarBuffer(std::string &, std::string);
bool writeTarFromDirectory(std::string, std::string);

#endif