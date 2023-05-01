
#include <iostream>
#include <chrono>
#include <thread>
#include <atomic>
#include <csignal>
#include <string>
#include <filesystem>

#include "base64.h"

#include "global.h"
#include "httplib.h"
#include "mini/ini.h"
#include "spdlog/spdlog.h"
#include "spdlog/async.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "ghc/filesystem.hpp"

#include "beagle_client.h"
#include "device_simulator.h"
#include "yacardemu_client.h"
#include "utility.h"

// Globals
constexpr static auto delay = std::chrono::microseconds(250);
std::atomic<bool> running{true};
std::shared_ptr<spdlog::async_logger> g_logger;


const char *helptext = 
	"YACardEmu - A simulator for magnetic card readers\n"
	"Commandline arguments:\n"
	"-d : debug log level\n"
	"-t : trace log level\n"
	"-f : log to yacardemu.log\n"
	"-h : show this help text\n"
	"\n";


void sigHandler(int sig)
{
	if (sig == SIGINT || sig == SIGTERM) {
		running = false;
	}
}

std::string pollNFC(void) {
	std::string retval = "";

	nfc_device *pnd;
	nfc_target nt;
	const nfc_modulation nmMifare = {
		.nmt = NMT_ISO14443A,
		.nbr = NBR_106,
	};

	// Allocate only a pointer to nfc_context
	nfc_context *context;
	//this->initNfc(pnd, context);

	//spdlog::info("Initializing NFC");
	// Initialize libnfc and set the nfc_context
	nfc_init(&context);
	if (context == NULL) {
		g_logger->critical("Unable to init NFC");
		return "";
	}

	if ((pnd = nfc_open(context, NULL)) == NULL){
		// Close NFC device
		nfc_close(pnd);
		// Release the context
		nfc_exit(context);
		pnd = nullptr;
		context = nullptr;
		return "";
	}

	// Set opened NFC device to initiator mode
	if (nfc_initiator_init(pnd) < 0) {
		//nfc_perror(pnd, "nfc_initiator_init");
	} else {
		g_logger->info("Polling NFC...");
		if (nfc_initiator_select_passive_target(pnd, nmMifare, NULL, 0, &nt) > 0)
			retval=bytesToHexString(nt.nti.nai.abtUid, nt.nti.nai.szUidLen);
	}
	// Close NFC device
	nfc_close(pnd);
	// Release the context
	nfc_exit(context);
	pnd = nullptr;
	context = nullptr;
	return retval;
}

static void NFCHost(AppSettings *settings)
{
	bool do_download = false;
	bool do_upload = false;
	bool lastDeviceState = false;
	unsigned int wait_counter = 0;

	const unsigned int default_sleep = 2;
	unsigned int sleep_for = default_sleep;
	std::string download_uid;

	std::unique_ptr<DeviceSimulator> simulator;
	simulator = std::make_unique<YACardEmuClient>("./yacardemu.ini");

	BeagleClient apiControl;
	httplib::Client httpcli(settings->apiHost, settings->apiPort);

/*
		Possible flows:
		NEW DATA
		1. user comes with nfc card, taps it. no data is downloadable.
		2. register card with server and keep uid.
		3. user plays game
		4. when device is removed, user confirms with a tap and data is uploaded as normal.

		NEW DATA BUT NO PRE-TAP
		1. user does not tap nfc card to attempt to load data.
		2. user plays game
		3. we wait for a confirmation tap to get the user's card information.
		4. if we do not get a confirmation tap after a predetermined interval, we delete their data because fuck em.
			if we do get a tap, we upload their data and then delete the local copy.

		DOWNLOAD DATA
		1. user taps nfc card. data is downloaded.
		2. user plays game
		3. when device is removed, user taps card to confirm upload. Then, data is uploaded as normal.
	
*/
	while (running)
	{
		//sleep for predetermined interval according to state
		sleep(sleep_for);
		sleep_for = default_sleep;

		// if card is not inserted, Accept NFC
		if (simulator->isDeviceInserted() && settings->nfc_uid.size() == 0) {

			// Poll NFC
			std::string polled_uid = pollNFC();

			if (polled_uid.size() > 0) {
				// If we downloaded data before and this new tap doesn't match up, don't store the new UID. 
				// Wait for the old one until the counter expires.
				if (download_uid.size() > 0 && polled_uid != download_uid){
					//TODO: Alert user somehow
					g_logger->error("upload tap mismatches download tap");
					//perhaps a button press to confirm using the new tap to upload data?
					continue;
				} else {
					settings->nfc_uid = polled_uid;
				}
				if (wait_counter > 0) {
					//we've been waiting for a card tap to confirm upload.
					do_upload = true;
				} else {
					// Download
					do_download = true;
				}
			}
		} else if (!simulator->isDeviceInserted()  && lastDeviceState) {
			// virtual device reshly removed. wait for an nfc tap for a limited time only!
			wait_counter = 1;
			lastDeviceState = false;
		} else if (simulator->isDeviceInserted()) {
			// virtual device freshly inserted
			lastDeviceState = true;
		}

		if (do_download) {
			//Download

			if (apiControl.downloadSaveData(httpcli, settings, settings->nfc_uid, simulator->getSaveDataPath())) {
				//download is successful. insert virtual device
				simulator->InsertDevice();
				download_uid = settings->nfc_uid;
			}
			else {
				// Alert user somehow
			}
			do_download = false;
		}

		if (do_upload) {
			//loop will never get here if download_uid and the currently polled uid do not match
			if (apiControl.uploadSaveData(httpcli, settings, settings->nfc_uid, simulator->getSaveDataPath())) {
				ghc::filesystem::remove_all(simulator->getSaveDataPath());
				ghc::filesystem::create_directory(simulator->getSaveDataPath());
			} else {
				//Alert user that upload failed. Add it to a persistent queue?
			}
			do_upload = false;
			wait_counter = 0;
			sleep_for=15; //sleep 15s so we don't accidentally start a new session
		}

		if (wait_counter > 30) {
			g_logger->info("fuckem");
			//Fuck em, we waited an entire minute.
			ghc::filesystem::remove_all(simulator->getSaveDataPath());
			download_uid.clear();
			wait_counter = 0;
		}
		else if (wait_counter > 0)
			wait_counter++;
	}

}

bool readAppConfig(AppSettings &settings)
{
	// Read in config values
	mINI::INIFile config("config.ini");

	mINI::INIStructure ini;

	if (!config.read(ini)) {
		g_logger->critical("Unable to open config.ini!");
		return false;
	}

	if (ini.has("config")) {
		//TODO: Read config
		settings.targetDevice = ini["config"]["targetdevice"];
		settings.gameId = std::stoi(ini["config"]["game_id"]);
		settings.apiHost = ini["config"]["api_host"];
		settings.apiBaseUrl = ini["config"]["api_base_url"];
		settings.apiPort = std::stoi(ini["config"]["api_port"]);
		settings.apiUser = ini["config"]["api_user"];
		settings.apiPassword = ini["config"]["api_password"];
		settings.uploadQueuePath = ini["config"]["upload_queue_path"];
	}

	return true;
}

int main(int argc, char *argv[])
{

	// Handle quitting gracefully via signals
	std::signal(SIGINT, sigHandler);
	std::signal(SIGTERM, sigHandler);

#ifdef NDEBUG
	spdlog::level::level_enum log_level = spdlog::level::info;
#else
	spdlog::level::level_enum log_level = spdlog::level::debug;
#endif

	bool file_log = false;
	if (argc > 1) {
		for (int i = 1; i < argc; i++) {
			std::string arg = argv[i];
			while (arg[0] == '-') arg.erase(0,1);
			switch (arg[0]) {
				case 'd': 
					log_level = spdlog::level::debug;
					break;
				case 't': 
					log_level = spdlog::level::trace;
					break;
				case 'f':
					file_log = true;
					break;
				case 'h':
				default:
					std::cout << helptext;
					return 0;
			}
		}
	}

	// Set up logger
	spdlog::init_thread_pool(8192, 1);
	auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt >();
	std::vector<spdlog::sink_ptr> sinks;

	if (file_log) {
		auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("yacardemu.log", true);
		sinks.push_back(file_sink);
	}

	sinks.push_back(stdout_sink);
	g_logger = std::make_shared<spdlog::async_logger>("main", sinks.begin(), sinks.end(), spdlog::thread_pool(), spdlog::async_overflow_policy::block);
	g_logger->set_level(log_level);
	g_logger->flush_on(spdlog::level::info);
	g_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");


	AppSettings settings;
	readAppConfig(settings);



	
	// Start Device Simulator
	g_logger->info("Starting API client");
	std::unique_ptr<DeviceSimulator> devSim;
	if (settings.targetDevice == "yacardemu")
		devSim = std::make_unique<YACardEmuClient>("./yacardemu.ini");
	else {
		g_logger->critical("Invalid target device specified");
		return 1;
	}

	/*
	if (devSim->isDeviceInserted())
		spdlog::info("device inserted");
	else
		spdlog::info("device not inserted");
	*/

	g_logger->info("Starting NFC Host/Network client");
	//std::thread(NFCHost, &settings).detach();
	NFCHost(&settings);

	
/*
	spdlog::info("Entering main loop");
	while (running) {
		std::this_thread::sleep_for(delay);
	}

	spdlog::info("Waiting 5s for threads to exit");
	sleep(5);
	spdlog::info("Exiting main thread normally");
*/
	return 0;
}
