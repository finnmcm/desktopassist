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
    bool refresh_db_files(const std::vector<std::string>& locations, const std::set<std::string>& fileTypes, int numWorkers, Database& db){
        std::vector<std::thread> threads;
        //work queue for storing directories
        std::deque<fs::path> workQueue;
        //depends on # root directories -> caller handled
        DbWriteQueue dbWriteQueue;
        threads.reserve(numWorkers);
        std::mutex m;
        for(int i = 0; i < numWorkers; i++){
            threads.emplace_back(file_worker, i, std::ref(workQueue), std::ref(m), std::ref(dbWriteQueue), std::ref(db));
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
    //WORKS FOR refresh_db_files
    void file_worker(int id, std::deque<fs::path>& workQueue, std::mutex& work_mutex, DbWriteQueue& dbWriteQueue, Database& db){
        for(;;){
            //LOCK GUARD - define an inner scope, workQueue immediately unlocks when mutex goes out of scope 
            //prevents early exits / exceptions crashing the program
            fs::path curDir;
            {
                std::lock_guard<std::mutex> lock(m);
                if(workQueue.empty()) return;
                curDir = workQueue.front();
                workQueue.pop_front();
            }
            std::error_code ec;
            for(const auto& dirEntry : fs::directory_iterator(curDir, ec)){
                if (ec) break;
                fs::path fp = dirEntry.path();
                if(dirEntry.is_directory() && !ec){
                        std::lock_guard<std::mutex> lock(m);
                        workQueue.push_back(fp);
                }
                else{
                    const std::string filename = fp.stem().string();
                    const std::string ext = fp.extension().string();
                    //do something
                }

            }
        }
    }
    void db_worker(int id, DbWriteQueue& dbWriteQueue, Database& db){

    }

}
