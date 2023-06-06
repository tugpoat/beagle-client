
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
		g_logger->info("BeagleClient::getAuthToken() - Failed to get bearer token");
		return false;
	} else {
		// startpos = 17, which is how many characters that {"access token":" is comprised of.
		// This is kind of a dirty way to do this and isn't very fault/change tolerant.
		std::size_t endpos = res->body.find("\",\"token_type");
		this->apiToken = res->body.substr(17, endpos - 17);
#ifndef NDEBUG
		g_logger->debug("BeagleClient::getAuthToken() - Got new bearer token: " + this->apiToken);
#endif
	}
	return true;
}

bool BeagleClient::downloadSaveData(httplib::Client &cli, AppSettings *settings, std::string savedata_uid, std::string dest_path)
{
	g_logger->info("Downloading savedata");

	// Build request
	httplib::Headers headers = {
		{ "Authorization", "Bearer " + this->apiToken }
	};
	std::string jsonreq = "{\"card_uid\": \"";
	jsonreq += savedata_uid + "\", \"game_id\": \"1\"}";

	// Send request
	auto res = cli.Post("/v1/savedata/fetch", headers, jsonreq, "application/json");
	if (!res) {
		g_logger->error("BeagleClient::downloadSaveData() - Request failed!");
		return false;
	}

	// Validate response

	if (res->status == 401) {
		g_logger->info("Token invalid. Fetching new token");
		if (getAuthToken(cli, settings)) {
			if (!downloadSaveData(cli, settings, savedata_uid, dest_path))
				return false;
			return true;
		}
	} else if (res->status == 404) {
		g_logger->info("API endpoint not valid");
		return false;
	}
	else if (res->status != 200) {
		g_logger->info("Server error");
		return false;
	}

	// Validate & process body content

	if (res->body[0] == '{' || res->body.length() < 1) {
		// JSON returned. No data exists on server for this game and card, or there was an error
		// No data to write.
		g_logger->info("No savedata on server");
		return false;
	} else {
		extractTarBuffer(res->body, dest_path);
	}

	g_logger->info("Successfully acquired savedata");

	return true;
}

bool BeagleClient::uploadSaveData(httplib::Client &cli, AppSettings *settings, std::string savedata_uid, std::string savedata_path)
{
	g_logger->info("Uploading savedata");
	std::string tempfilepath = "/tmp/" + savedata_uid + ".tar";
	if (!writeTarFromDirectory(tempfilepath, savedata_path)) {
		g_logger->error("Couldn't write tempfile for savedata");
		return false;
	}

	std::vector<char> filedata;
	if (!readFile(tempfilepath.c_str(), filedata)) {
		g_logger->error("Couldn't read tempfile");
		return false;
	}

#ifndef NDEBUG
	g_logger->debug("tempfile: " + tempfilepath);
	g_logger->debug("tempfile size: " + std::to_string(filedata.size()));
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
		g_logger->error("BeagleClient::uploadSaveData() - Request failed!");
		return false;
	}

	filedata.clear();
	ghc::filesystem::remove(tempfilepath);

	// Validate response

	if (res->status == 401) {
		g_logger->info("Token invalid. Fetching new token");
		if (getAuthToken(cli, settings)) {
			if (!uploadSaveData(cli, settings, savedata_uid, savedata_path))
				return false;
			return true;
		}
	} else if (res->status != 200) {
		g_logger->info("HTTP error, code: " + std::to_string(res->status));
		return false;
	}

	//TODO: do something with the response

	return true;
}
