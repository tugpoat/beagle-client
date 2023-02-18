#include "utility.h"

std::string bytesToHexString(const unsigned char* str, const uint64_t s)
{
  std::ostringstream ret;

  for (size_t i = 0; i < s; ++i)
    ret << std::hex << std::setfill('0') << std::setw(2) << std::nouppercase
        << (int)str[i];

  return ret.str();
} 

bool readFile(const char* filename, std::vector<char> &target)
{
    // open the file:
    std::ifstream file(filename, std::ios::binary);

    if (!file) {
    	return false;
    }

    // Stop eating new lines in binary mode!!!
    file.unsetf(std::ios::skipws);

    // get its size:
    std::streampos fileSize;

    file.seekg(0, std::ios::end);
    fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    if (fileSize < 1) {
    	return false;
    }

    // reserve capacity
    std::vector<char> vec;
    target.reserve(fileSize);

    // read the data:
    target.insert(target.begin(),
               std::istream_iterator<char>(file),
               std::istream_iterator<char>());

    file.close();

    return true;
}

bool writeTarFromDirectory(std::string target, std::string source)
{
	std::ofstream ofs;
	try
	{
#ifndef NDEBUG
		spdlog::debug("opening file for writing: " + target);
#endif
		ofs.open(target);
		if (!ofs) {
			spdlog::error("writeTarFromDirectory(): Couldn't open file for writing: " + target);
			return false;
		}
		// Open tar to stream to
		namespace ar = ns_archive::ns_writer;
		ns_archive::writer writer = ns_archive::writer::make_writer<ns_archive::ns_writer::format::_TAR>(ofs, 10240);
		
		// Loop through file list in savedata directory and add them to the archive
		for (const auto &entry: ghc::filesystem::directory_iterator(source)) {
			std::string savedata{entry.path().string()};
			auto find = savedata.find(source);

			if (find != std::string::npos) {

				// Open the file.
				std::ifstream infile(savedata, std::ios::in | std::ios::binary);
#ifndef NDEBUG
				spdlog::debug("opening file for reading" + savedata);
#endif
				if (!infile) {
					spdlog::error("Couldn't open file for reading: " + find);

					return false;
				}

				// Add filedata
				std::stringstream ss;
				//std::cout << infile.rdbuf() << std::endl;
				ss << infile.rdbuf();
				//infile.close();

				ns_archive::entry out_entry(ss);

				// Add the filename and finish up
				out_entry.set_header_value_pathname(savedata.substr(savedata.find_last_of("/") + 1));
				writer.add_entry(out_entry);
#ifndef NDEBUG
				spdlog::debug("added " + savedata + " to tarfile");
#endif
			}
		}
		writer.finish();
		ofs.close();
	}
	catch(ns_archive::archive_exception& e)
	{
		spdlog::error(e.what());
		ofs.close();
		return false;
	}
	catch (ghc::filesystem::filesystem_error& e) {
		spdlog::error(e.what());
		ofs.close();
		return false;
	}
	return true;
}
