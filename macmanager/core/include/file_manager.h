#pragma once 
#include <filesystem>
#include <string>
namespace macmanager {
    class FileManager {
        public: 
        FileManager();
        bool find(const std::string& filename, std::filesystem::path& filepath);

        private:
        std::filesystem::path root;
    };
}