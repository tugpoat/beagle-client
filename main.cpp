
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
#include "spdlog/sinks/basic_file_sink.h"
#include "ghc/filesystem.hpp"

#include "beagle_client.h"
#include "utility.h"

// Globals
static const auto delay{std::chrono::microseconds(250)};
std::atomic<bool> running{true};

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
		spdlog::critical("Unable to init NFC");
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
		spdlog::info("Polling NFC...");
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
/*
static void NFCHost(AppSettings *settings, CardIo::Settings *card)
{
	bool downloadedSaveData = false;
	bool do_download = false;
	bool do_upload = false;
	bool lastDeviceState = false;
	unsigned int wait_counter = 0;

	const unsigned int default_sleep = 2;
	unsigned int sleep_for = default_sleep;
	std::string download_uid;

	ApiClient apiControl;
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
	

	while (running)
	{
		//sleep for predetermined interval according to state
		sleep(sleep_for);
		sleep_for = default_sleep;

		// if card is not inserted, Accept NFC
		if (!card->insertedCard && settings->nfc_uid.size() == 0) {

			// Poll NFC
			std::string polled_uid = pollNFC();

			if (polled_uid.size() > 0) {
				// If we downloaded data before and this new tap doesn't match up, don't store the new UID. 
				// Wait for the old one until the counter expires.
				if (download_uid.size() > 0 && polled_uid != download_uid){
					//TODO: Alert user somehow
					spdlog::error("upload tap mismatches download tap");
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
		} else if (!card->insertedCard && lastDeviceState) {
			// virtual device reshly removed. wait for an nfc tap for a limited time only!
			wait_counter = 1;
			lastDeviceState = false;
		} else if (card->insertedCard) {
			// virtual device freshly inserted
			lastDeviceState = true;
		}

		if (do_download) {
			//Download

			if (apiControl.downloadSaveData(httpcli, settings, settings->nfc_uid, card->cardPath)) {
				//download is successful. insert virtual device
				card->insertedCard = true;
				downloadedSaveData = true;
				download_uid = settings->nfc_uid;
			}
			else {
				// Alert user somehow
			}
			do_download = false;
		}

		if (do_upload) {
			//loop will never get here if download_uid and the currently polled uid do not match
			if (apiControl.uploadSaveData(httpcli, settings, settings->nfc_uid, card->cardPath)) {
				ghc::filesystem::remove_all(card->cardPath);
				ghc::filesystem::create_directory(card->cardPath);
			} else {
				//Alert user that upload failed. Add it to a persistent queue?
			}
			do_upload = false;
			wait_counter = 0;
			sleep_for=15; //sleep 15s so we don't accidentally start a new session
		}

		if (wait_counter > 30) {
			spdlog::info("fuckem");
			//Fuck em, we waited an entire minute.
			ghc::filesystem::remove_all(card->cardPath);
			download_uid.clear();
			wait_counter = 0;
		}
		else if (wait_counter > 0)
			wait_counter++;
	}

}
*/
bool readConfig(AppSettings &settings)
{
	// Read in config values
	mINI::INIFile config("config.ini");

	mINI::INIStructure ini;

	if (!config.read(ini)) {
		spdlog::critical("Unable to open config.ini!");
		return false;
	}

	if (ini.has("config")) {
		//TODO: Read config
	}

	return true;
}

int main()
{

	// Handle quitting gracefully via signals
	std::signal(SIGINT, sigHandler);
	std::signal(SIGTERM, sigHandler);

	AppSettings *settings = new AppSettings;


	spdlog::info("Starting NFC Host/Network client");
	//std::thread(NFCHost, settings, &cardHandler->cardSettings).detach();

	spdlog::info("Entering main loop");
	while (running) {
		// TODO: device read/write should probably be a separate thread
		std::this_thread::sleep_for(delay);

		spdlog::dump_backtrace();
		std::this_thread::sleep_for(delay);
	}

	spdlog::info("Waiting 5s for threads to exit");
	sleep(5);
	spdlog::info("Exiting main thread normally");

	return 0;
}
