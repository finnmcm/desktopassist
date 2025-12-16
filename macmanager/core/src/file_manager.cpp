#include "file_manager.h"
#include <cstdlib>
#include <iostream>

namespace fs = std::filesystem;

namespace macmanager {
    FileManager::FileManager(){
        root = std::getenv("HOME");
    }
    bool FileManager::find(const std::string& filename, fs::path& filepath){
        //DESKTOP
        fs::path desktop = root / "Desktop";
        if(!fs::exists(desktop) || !fs::is_directory(desktop)){
            return false;
        }
        std::error_code ec;
        for(const auto& dirEntry : fs::recursive_directory_iterator
                                                            (desktop, 
                                                            fs::directory_options::skip_permission_denied, 
                                                            ec)){
            if(dirEntry.path().stem() == filename){
                filepath = dirEntry.path();
                std::cout << "true" << std::endl;
                return true;
            }
        }

        fs::path downloads = root / "Downloads";
        fs::path documents = root / "Documents";
        return false;
    }
}
