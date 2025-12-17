#pragma once 
#include <filesystem>
#include <string>
#include <vector>
#include <set>
#include <deque>
#include <mutex>
namespace macmanager {
    class FileManager {
        public: 
        FileManager();
        bool find(const std::string& filename, std::filesystem::path& filepath);
        bool refresh_db_files(const std::vector<std::string>& locations, const std::set<std::string>& fileTypes, int numWorkers, Database& db);

        private:
        void file_worker(int id, std::deque<fs::path>& workQueue, std::mutex& m, Database& db);
        std::filesystem::path root;

    };
}