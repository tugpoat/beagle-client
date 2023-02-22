
#include "beagle_client.h"

bool BeagleClient::getAuthToken(httplib::Client &cli, AppSettings *settings)
{
	httplib::MultipartFormDataItems items = {
		 { "username", settings->apiUser, "", "" },
		 { "password", settings->apiPassword, "", "" }
	};
	
	auto res = cli.Post("/token", items);

	if (res->status != 200)
	{
		spdlog::info("Failed to get bearer token");
		return false;
	} else {
		// startpos = 17, which is how many characters that {"access token":" is comprised of.
		// This is kind of a dirty way to do this and isn't very fault/change tolerant.
		std::size_t endpos = res->body.find("\",\"token_type");
		this->apiToken = res->body.substr(17, endpos - 17);
#ifdef NDEBUG
		spdlog::debug("Got new bearer token: " + this->apiToken);
#endif
	}
	return true;
}

bool BeagleClient::downloadSaveData(httplib::Client &cli, AppSettings *settings, std::string savedata_uid, std::string dest_path)
{
	spdlog::info("Downloading savedata");

	// Build request
	httplib::Headers headers = {
		{ "Authorization", "Bearer " + this->apiToken }
	};
	std::string jsonreq = "{\"card_uid\": \"";
	jsonreq += savedata_uid + "\", \"game_id\": \"1\"}";

	// Send request
	auto res = cli.Post("/v1/savedata/fetch", headers, jsonreq, "application/json");
	if (!res) {
		spdlog::error("Request failed! Network or server might be down.");
		return false;
	}

	// Validate response

	if (res->status == 401) {
		spdlog::info("Token invalid. Fetching new token");
		if (getAuthToken(cli, settings)) {
			if (!downloadSaveData(cli, settings, savedata_uid, dest_path))
				return false;
			return true;
		}
	} else if (res->status == 404) {
		spdlog::info("API endpoint not valid");
		return false;
	}
	else if (res->status != 200) {
		spdlog::info("Server error");
		return false;
	}

	// Validate & process body content

	if (res->body[0] == '{' || res->body.length() < 1) {
		// JSON returned. No data exists on server for this game and card, or there was an error
		// No data to write.
		spdlog::info("No savedata on server");
		return false;
	} else {

		// FIXME: BREAK THIS OUT INTO ITS OWN FUNCTION.
		// Extract tar file
		try
		{
			std::stringstream in(res->body);   // Create stream from what we got from the server
			// Generate archive reader from stream
			namespace ar = ns_archive::ns_reader;
			ns_archive::reader reader = ns_archive::reader::make_reader<ar::format::_ALL, ar::filter::_ALL>(in, 10240);

			// Output each file in archive to the target directory
			for(auto entry : reader)
			{
				std::ofstream savedata;
				savedata.open(dest_path + entry->get_header_value_pathname(), std::ofstream::out | std::ofstream::binary);
				savedata << entry->get_stream().rdbuf();
				savedata.close();
			}
		}
		catch(ns_archive::archive_exception& e)
		{
			spdlog::error(e.what());
		}
	}

	spdlog::info("Successfully acquired savedata");

	return true;
}

bool BeagleClient::uploadSaveData(httplib::Client &cli, AppSettings *settings, std::string savedata_uid, std::string savedata_path)
{
	spdlog::info("Uploading savedata");
	std::string tempfilepath = "/tmp/" + savedata_uid + ".tar";
	if (!writeTarFromDirectory(tempfilepath, savedata_path)) {
		spdlog::error("Couldn't write tempfile for savedata");
		return false;
	}

	std::vector<char> filedata;
	if (!readFile(tempfilepath.c_str(), filedata)) {
		spdlog::error("Couldn't read tempfile");
		return false;
	}

#ifdef NDEBUG
	spdlog::debug("tempfile: " + tempfilepath);
	spdlog::debug("tempfile size: " + std::to_string(filedata.size()));
#endif

	// Build request
	httplib::Headers headers = {
		{ "Authorization", "Bearer " + this->apiToken }
	};
	httplib::MultipartFormDataItems items = {
		 { "game_id", std::to_string(settings->gameId), "", "" },
		 { "card_uid", savedata_uid, "", "" },
		 { "savedata", std::string(filedata.data(),filedata.size()), savedata_uid, "application/octet-stream" }
	};

	// Send request
	auto res = cli.Post("/v1/savedata/push", headers, items, "boundaryhuehue");
	if (!res) {
		spdlog::error("Request failed! Network or server might be down.");
		return false;
	}

	filedata.clear();
	ghc::filesystem::remove(tempfilepath);

	// Validate response

	if (res->status == 401) {
		spdlog::info("Token invalid. Fetching new token");
		if (getAuthToken(cli, settings)) {
			if (!uploadSaveData(cli, settings, savedata_uid, savedata_path))
				return false;
			return true;
		}
	} else if (res->status != 200) {
		spdlog::info("HTTP error, code: " + std::to_string(res->status));
		return false;
	}

	//TODO: do something with the response

	return true;
}


std::string BeagleClient::bytesToHexString(const unsigned char* str, const uint64_t s) {
  std::ostringstream ret;

  for (size_t i = 0; i < s; ++i)
    ret << std::hex << std::setfill('0') << std::setw(2) << std::nouppercase
        << (int)str[i];

  return ret.str();
} 
