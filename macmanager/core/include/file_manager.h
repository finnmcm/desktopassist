#pragma once 
#include <filesystem>
#include <string>
#include <vector>
#include <set>
#include <deque>
#include <mutex>
namespace macmanager {

    struct DbWriteQueue {
        std::mutex write_mutex;
        std::deque<std::string> writeQueue;
        std::atomic<int> workers_running{0};
        std::condition_variable cv;
    };

    class FileManager {
        public: 
        FileManager();
        bool find(const std::string& filename, std::filesystem::path& filepath);
        bool refresh_db_files(const std::vector<std::string>& locations, const std::set<std::string>& fileTypes, int numWorkers, Database& db);

        private:
        void file_worker(int id, std::deque<fs::path>& workQueue, std::mutex& work_mutex, Database& db, DbWriteQueue& dbWriteQueue);
        void db_worker(int id, DbWriteQueue& dbWriteQueue, Database& db);
        std::filesystem::path root;

    };
}