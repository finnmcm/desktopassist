#include <iostream>
#include <stdlib.h>
#include <string>
#include "run_app.h"
#include "file_manager.h"
#include "database.cpp"

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
        
        db.printAllTablesAndSchema();
        return 0;
    }
}