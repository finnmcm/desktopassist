#include "macmanager_bridge.h"
#include "../../core/include/file_manager.h"
//#include "../../core/src/database.cpp"

//PUBLIC API TYPE: mm_handle_t
//INTERNAL REFERENCE TYPE: mm_handle
//the typedef in the header file allows us to reference the struct there, so we alias it 

struct mm_handle {
    macmanager::FileManager fm;
    macmanager::Database db;

    mm_handle(std::string& dbPath) :
        fm(),
        db(dbPath) {}
};

mm_handle_t* mm_create(mm_status* status){
    if(status) *status = MM_OK;
    try {
        //hardcoded for now
        std::string dbPath = std::string(std::getenv("HOME")) + "/Library/Application Support/MacAssistant/state.db";
        mm_handle* m = new mm_handle(dbPath);
        return m;
    } catch(std::bad_alloc&){ // catch all exceptions
        if(status) *status = MM_ERR_OOM;
        return nullptr;
    }
    catch(...) {
        if(status) *status = MM_ERR_INTERNAL;
        return nullptr;
    }
}
void mm_refresh_db_files(mm_handle_t* h, mm_status_t* status, char** locations, int numLocations, char** fileTypes, int numFilesTypes, int numWorkers){
    if(status) *status = MM_OK;
    //convert a bunch of char**'s to their actual cpp objects that fm expects
    if(numFilesTypes <= 0 || numLocations <= 0){
        if(status){ *status == MM_ERR_INVALID_ARG;}
        return;
    }
    std::vector<std::filesystem::path> fs_locations;
    fs_locations.resize(numLocations);
    std::set<std::string> fs_fileTypes;
    try {
        for(int i = 0; i < numLocations; i++){
            if(!locations[i]){ 
                if(status){ *status = MM_ERR_INVALID_ARG; } 
                return;
            }
            std::string curLocation(locations[i]);
            fs_locations[i] = curLocation;
        }
        for(int i = 0; i < numFilesTypes; i++){
            if(!fileTypes[i]){ 
                if(status){ *status = MM_ERR_INVALID_ARG; } 
                return;
            }
            std::string fileType(fileTypes[i]);
            fs_fileTypes.insert(fileType);
            fileTypes++;
        }
            
        h->fm.refresh_db_files(fs_locations, fs_fileTypes, numWorkers, h->db);
    }
    catch(...){
        
    }
}
void mm_destroy(mm_handle_t* h){
    delete h;
}
void mm_free(void* p){
    delete p;
}

