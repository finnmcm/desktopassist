#include <iostream>
#include <stdlib.h>
#include <string>
#include "run_app.h"
#include "file_manager.h"

namespace macmanager {
    macmanager::FileManager fm;
    std::string dbPath = std::string(std::getenv("HOME")) + "/Library/Application Support/MacAssistant/state.db";
    macmanager::Database db(dbPath);

    void refresh_db(){
        std::set<std::string> types;
       types.insert(".pdf");
       types.insert(".txt");
       std::filesystem::path root = std::getenv("HOME");
       std::vector<std::filesystem::path> locs;
       locs.push_back(root / "Desktop");
       locs.push_back(root / "Downloads");
       locs.push_back(root / "Documents");
     // fm.refresh_db_files(locs, types, 2, db);
    }
    void stage_files(){
        std::vector<std::filesystem::path> files;
        std::filesystem::path f1 = "/Users/finnmcmillion/Desktop/test.txt";
        std::filesystem::path f2 = "/Users/finnmcmillion/Downloads/FinnMcMillan-resume.pdf";
        std::filesystem::path f3 = "/Users/finnmcmillion/Documents/SNAP_20220804-202803.jpg";

        files.push_back(f1);
        files.push_back(f2);
        files.push_back(f3);

        fm.stage_files(files);

    }

    int run_app(int argc, char** argv) {
        if(argc < 3){
            std::cout << "invalid # params";
            return 0;
        }
        std::string manager = argv[0];
        std::string usage = argv[1];
        std::string args = argv[2];
        
       // db.printAllTablesAndSchema();
       
       //db.listObjects();
       stage_files();
        return 0;
    }
}