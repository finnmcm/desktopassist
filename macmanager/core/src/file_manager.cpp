#include "file_manager.h"
#include <cstdlib>
#include <iostream>
#include "database.cpp"
#include <thread>

namespace fs = std::filesystem;

namespace macmanager {
    FileManager::FileManager(){
        root = std::getenv("HOME");
    }
    bool FileManager::refresh_db_files(const std::vector<std::string>& locations, const std::set<std::string>& fileTypes, int numWorkers, Database& db){
        std::vector<std::thread> threads;
        std::deque<fs::path> workQueue;
        threads.reserve(numWorkers);
        std::mutex m;
        for(int i = 0; i < numWorkers; i++){
            threads.emplace_back(file_worker, i, std::ref(workQueue), std::ref(m), std::ref(db));
        }
        
        for(std::thread& t : threads){
            if(t.joinable())
                t.join();
        }
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
    void file_worker(int id, std::deque<fs::path> workQueue, std::mutex& m, Database& db){

    }
}
