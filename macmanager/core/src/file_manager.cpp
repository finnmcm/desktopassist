#include "file_manager.h"
#include <cstdlib>
#include <iostream>
#include <thread>
#include <unistd.h>



namespace macmanager {
    namespace fs = std::filesystem;
    inline int64_t filetime_to_unix_seconds(std::filesystem::file_time_type ftime) {
        using namespace std::chrono;

        // Convert file clock -> system_clock by “bridging” via now()
        auto sctp = time_point_cast<system_clock::duration>(
            ftime - std::filesystem::file_time_type::clock::now() + system_clock::now()
        );

        return static_cast<int64_t>(system_clock::to_time_t(sctp));
    }

    std::mutex print_mutex;
    void ts_print(const std::string& msg) {
        std::lock_guard<std::mutex> lock(print_mutex);
        std::cout << msg << std::endl;
    }
    /*
    TIMER for thread timing:
    refesh_db_files: 2 threads optimal
    struct Timer {
        using clock = std::chrono::steady_clock;
        std::string label;
        clock::time_point start;
        Timer(std::string lbl) : label(std::move(lbl)), start(clock::now()) {}
        ~Timer() {
            auto end = clock::now();
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
            std::cerr << label << ": " << ms << " ms\n";
        }
    };
    */
        //WORKS FOR refresh_db_files
    void file_worker(int id, std::deque<fs::path>& workQueue, std::mutex& work_mutex, DbWriteQueue& dbWriteQueue, const std::set<std::string>& fileTypes){
        while(true){
            //LOCK GUARD - define an inner scope, workQueue immediately unlocks when mutex goes out of scope 
            //prevents early exits / exceptions crashing the program
            fs::path curDir;
            {
                std::lock_guard<std::mutex> lock(work_mutex);
                if(workQueue.empty()) break;
                curDir = workQueue.front();
                workQueue.pop_front();
            }
            std::error_code ec;
            for(const auto& dirEntry : fs::directory_iterator(curDir, ec)){
                if (ec) break;
                fs::path fp = dirEntry.path();
                if(dirEntry.is_directory() && !ec){
                        std::lock_guard<std::mutex> lock(work_mutex);
                        workQueue.push_back(fp);
                }
                else if(dirEntry.is_regular_file() && !ec && fileTypes.count(fp.extension().string()) > 0){
                    int64_t lastModified = macmanager::filetime_to_unix_seconds(dirEntry.last_write_time());
                    DbFile obj{fp.string(), fp.stem().string(), fp.extension().string(), lastModified, dirEntry.file_size()};
                    {
                        std::lock_guard<std::mutex> lock(dbWriteQueue.write_mutex);
                        dbWriteQueue.writeQueue.push_back(obj);
                    }
                    //WAKE THE WRITE THREAD since we just pushed a write obj to the queue
                    //cv == condition variable db_writer is dependent on
                    dbWriteQueue.cv.notify_one();
                }

            }
        }
        //subtract 1 from num running threads
        dbWriteQueue.workers_running.fetch_sub(1);
        dbWriteQueue.cv.notify_one();

    }
    void db_worker(DbWriteQueue& dbWriteQueue, Database& db){
        //unique lock gives more control than lock guard, allows the cv to control it
        std::unique_lock<std::mutex> lock(dbWriteQueue.write_mutex);

        while(true){
            //unlock/stop waiting ONLY when the predicate is true (no worker threads or objects on the write queue)
            //lambda func defines when thread is allowed to proceed
            //[&] == capture everything by reference
            //unlock mutex while asleep, lock while awake
            dbWriteQueue.cv.wait(lock, [&]{
                return !dbWriteQueue.writeQueue.empty() ||
                   dbWriteQueue.workers_running.load(std::memory_order_acquire) == 0;
            });
            //shutdown condition - no more writes, no more active file exploring
            if(dbWriteQueue.writeQueue.empty() && dbWriteQueue.workers_running.load(std::memory_order_acquire) == 0){
                break;
            }   
            ts_print("GETTING FILES");
            DbFile curFile = dbWriteQueue.writeQueue.front();
            dbWriteQueue.writeQueue.pop_front();

            //unlock the write queue while we make the write to the db (that way we can get back to processing files)
            lock.unlock();
            db.insert_file(curFile);
            lock.lock();

        }
        //flush commit here maybe?

    }
    FileManager::FileManager(){
    }

    void FileManager::refresh_db_files(const std::vector<std::string>& locationStrs, const std::set<std::string>& fileTypes, int numWorkers, Database& db){
        std::vector<std::thread> threads;
        //work queue for storing directories
        std::deque<fs::path> workQueue;
        //depends on # root directories -> caller handled
        DbWriteQueue dbWriteQueue;
        dbWriteQueue.workers_running.store(numWorkers, std::memory_order_release);
        threads.reserve(numWorkers);
        std::mutex m;
        
        std::vector<fs::path> locations;
        locations.resize(locationStrs.size());
        for(size_t i = 0; i < locations.size(); i++){
            fs::path curPath = root / locationStrs[i];
          //  std::cout << "locStr : " << locationStrs[i] << std::endl;
           // std::cout << curPath << std::endl;
            locations[i] = this->string_to_path(locationStrs[i]);
        }

        //dedicated writer thread:
        std::cout << "start of refresh db" << std::endl;
      //  Timer t("scan+process");
        std::thread writer(db_worker, std::ref(dbWriteQueue), std::ref(db));
        ts_print("created db worker");
        //insert root directories into workQueue:
        for(const auto& p : locations){
            workQueue.push_back(p);
        }
        for(int i = 0; i < numWorkers; i++){
            threads.emplace_back(file_worker, i, std::ref(workQueue), std::ref(m), std::ref(dbWriteQueue), std::ref(fileTypes));
        }
        ts_print("files running");
        for(std::thread& t : threads){
            if(t.joinable())
                t.join();
        }
        writer.join();
        std::cout << "finished" << std::endl;
        return;
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
    void FileManager::stage_files(const std::vector<fs::path>& files){
        fs::path stageDir = fs::temp_directory_path() / "MacAssist-" / std::to_string(getpid());
        fs::create_directories(stageDir);
        //create symlinks for files
        for(auto& f : files){
            fs::path symPath = stageDir / f.filename();
            int n = 1;
            while(fs::exists(symPath)){
                symPath = stageDir / (f.stem().string() + "-" + std::to_string(n) + f.extension().string());
                n++;
            }
            try {
                fs::create_symlink(fs::absolute(f), symPath);
                std::cout << "symlink created between " << fs::absolute(f).string() << " and " << symPath.string() << std::endl;
            } catch(const fs::filesystem_error& e){
                std::cerr << e.what() << std::endl;
            }
        }
        //open staging folder
        std::string cmd = "open \"" + stageDir.string() + "\"";
        int rc = std::system(cmd.c_str());
        (void)rc;
        //FIND A WAY TO CLEAN THIS
    }
    fs::path FileManager::string_to_path(const std::string& str){
        return root / str;
    }

}
