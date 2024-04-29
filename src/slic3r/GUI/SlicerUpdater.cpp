#include "SlicerUpdater.hpp"
#include "wx/wx.h"
#include "wx/url.h"
#include "wx/utils.h"
#include "wx/file.h"
#include "boost/filesystem.hpp"
#include <iostream>
#include <regex>
#include <curl/curl.h>


namespace Slic3r {
namespace GUI {


bool SlicerUpdater::CheckForUpdates() {
	
	std::string OWNER = "CR-3D";
    std::string REPOSITORY = "SliCR-3D";
    std::string RELEASE_URL = "https://github.com/" + OWNER + "/" + REPOSITORY + "/releases";

	CURL *curl = curl_easy_init();
	if (!curl) {
		return false;
	}

	curl_easy_setopt(curl, CURLOPT_URL, RELEASE_URL.c_str());

	struct curl_slist* headers = NULL;
    headers                   = curl_slist_append(headers, "User-Agent: curl");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

	std::string responseString;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseString);


	CURLcode res = curl_easy_perform(curl);
	if (res != CURLE_OK) {
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
		return false;
	}

	std::regex  versionRegex(R"(<a href="/)" + OWNER + R"(/)" + REPOSITORY + R"(/releases/tag/([^"]+))");
    std::smatch versionMatch;
	if (std::regex_search(responseString, versionMatch, versionRegex)) {
        std::string latestVersion = versionMatch[1];
        std::cout << "Latest version on Github: " << latestVersion << std::endl;

		// Compare latest version with current version
        std::string currentVersion = SLIC3R_VERSION;
		if (latestVersion != currentVersion) {
            std::cout << "Update is available" << std::endl;
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
			return true;
        } else {
            std::cout << "No update available." << std::endl;
            curl_slist_free_all(headers);
            curl_easy_cleanup(headers);
			return false;
		}
    } else {
        std::cerr << "Failed to extract version information from HTML response" << std::endl;
        curl_slist_free_all(headers);
        curl_easy_cleanup(headers);
		return false;
	}
	
	return false;
}

size_t SlicerUpdater::WriteCallback(void *contents, size_t size, size_t nmemb, std::string *responseString)
{
    size_t totalSize = size * nmemb;
    responseString->append((char *) contents, totalSize);
    return totalSize;
}


bool SlicerUpdater::DownloadUpdate(const std::string &updateURL)
{
    boost::filesystem::path tempDir = boost::filesystem::temp_directory_path();

    boost::filesystem::path tempFilePath = tempDir / "SliCR-3D_update.zip";
    wxFileOutputStream      outputStream(tempFilePath.c_str());
    if (!outputStream.IsOk()) {
        wxLogError("Failed to create temporary file for update.");
        return false;
    }

    wxURL url(updateURL);
    if (!url.IsOk()) {
        wxLogError("Invalid update URL: %s", updateURL);
        return false;
    }

    wxInputStream *inputStream = url.GetInputStream();
    if (!inputStream) {
        wxLogError("Failed to open input stream for update URL.");
        return false;
    }

    if (!inputStream->IsOk()) {
        wxDELETE(inputStream);
        wxLogError("Failed to retrieve update data from URL: %s", updateURL);
        return false;
    }

    if (!inputStream->Read(outputStream)) {
        wxDELETE(inputStream);
        wxLogError("Failed to download update from URL: %s", updateURL);
        return false;
    }

    wxDELETE(inputStream);
    outputStream.Close();

    return true;
        }
	}
}