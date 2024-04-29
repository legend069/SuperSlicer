#ifndef slic3r_SlicerUpdater_hpp_
#define slic3r_SlicerUpdater_hpp_


#include <string>



namespace Slic3r {
namespace GUI {

class SlicerUpdater
{
public:
    static bool CheckForUpdates();
    static bool DownloadUpdate(const std::string& updateURL);
    static bool ApplyUpdate(const std::string& updateFilePath);
    size_t      WriteCallback(void *contents, size_t size, size_t nmemb, std::string *responseString);
};

}
}
#endif
