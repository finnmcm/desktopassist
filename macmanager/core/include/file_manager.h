#pragma once 
#include <filesystem>
#include <string>
#include <vector>
#include <set>
#include <deque>
#include <mutex>
#include "../src/database.cpp"
namespace macmanager {


    struct DbWriteQueue {
        std::mutex write_mutex;
        std::deque<DbFile> writeQueue;
        std::atomic<int> workers_running{0};
        std::condition_variable cv;
    };
    class FileManager {
        public: 
        FileManager();
        bool find(const std::string& filename, std::filesystem::path& filepath);
        bool refresh_db_files(const std::vector<std::filesystem::path>& locations, const std::set<std::string>& fileTypes, int numWorkers, Database& db);
        bool stage_files(const std::vector<std::filesystem::path>& files);
        private:
        std::filesystem::path root;

    };
}