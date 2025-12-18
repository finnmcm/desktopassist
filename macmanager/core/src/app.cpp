#include <iostream>
#include <stdlib.h>
#include <string>
#include "run_app.h"
#include "file_manager.h"

namespace macmanager {
    int run_app(int argc, char** argv) {
        macmanager::FileManager fm;
        std::string dbPath = std::string(std::getenv("HOME")) + "/Library/Application Support/MacAssistant/state.db";
        macmanager::Database db(dbPath);
        if(argc < 3){
            std::cout << "invalid # params";
            return 0;
        }
        std::string manager = argv[0];
        std::string usage = argv[1];
        std::string args = argv[2];
        
       // db.printAllTablesAndSchema();
       std::set<std::string> types;
       types.insert(".pdf");
       types.insert(".txt");
       std::filesystem::path root = std::getenv("HOME");
       std::vector<std::filesystem::path> locs;
       locs.push_back(root / "Desktop");
       locs.push_back(root / "Downloads");
       locs.push_back(root / "Documents");
       fm.refresh_db_files(locs, types, 2, db);
       //db.listObjects();
        return 0;
    }
}