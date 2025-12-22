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
void mm_get_file_paths(mm_handle_t* h, mm_status_t* status, mm_get_file_args_t* args, mm_string_array_t* return_paths){
    /*
     std::vector<DbFile> query_files(const std::vector<std::string>& file_names, const std::vector<std::string>& file_extensions, 
                                    int64_t modified_within, int64_t modified_after){
    */
    if(args->num_file_names <= 0 && args->num_file_extensions <= 0 && args->modified_within == -1 && args->modified_after == -1){
        if(status) *status = MM_ERR_INVALID_ARG;
        return;
    }
    if((args->modified_after < 0 && args->modified_after != -1) || (args->modified_within < 0 && args->modified_within != -1)){
        if(status) *status = MM_ERR_INVALID_ARG;
        return;
    }
    std::vector<std::string> db_file_names;
    std::vector<std::string> db_file_exts;
    if(args->num_file_names > 0){
        db_file_names.resize(args->num_file_names);
        for(size_t i = 0; i < args->num_file_names; i++){
            if(!args->file_names[i]){
                if(status) *status = MM_ERR_INVALID_ARG;
                return;
            }
            db_file_names[i] = std::string(args->file_names[i]);
        }
    }
    if(args->num_file_extensions > 0){
        db_file_exts.resize(args->num_file_extensions);
        for(size_t i = 0; i < args->num_file_extensions; i++){
            if(!args->file_extensions[i]){
                if(status) *status = MM_ERR_INVALID_ARG;
                return;
            }
            db_file_exts[i] = std::string(args->file_extensions[i]);
        }
    }
    try {
        int64_t db_modified_within = args->modified_within;
        int64_t db_modified_after = args->modified_after;
        std::vector<macmanager::DbFile> db_files = h->db.query_files(db_file_names, db_file_exts, db_modified_within, db_modified_after);
        return_paths->size = db_files.size();
        return_paths->data = (char**)malloc(sizeof(return_paths->size * sizeof(char*)));
        if(!return_paths->data){
            if(status) *status = MM_ERR_OOM;
            return;
        }
        for(size_t i = 0; i < db_files.size(); i++){
            char* path = (char*)malloc(sizeof(char)*(1+db_files[i].filepath.size()));
            std::memcpy(path, db_files[i].filepath.c_str(), db_files[i].filepath.size()+1);
            return_paths->data[i] = path;
        }
        return;
    }

    catch(std::bad_alloc&){
        if(status) *status = MM_ERR_OOM;
        return;
    }
    catch(std::exception&){
        if(status) *status = MM_ERR_INTERNAL;
        return;
    }
    catch(...){
        if(status) *status = MM_ERR_UNKNOWN;
        return;
    }
}
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
        if(status) *status = MM_ERR_UNKNOWN;
        return nullptr;
    }
}
void mm_refresh_db_files(mm_handle_t* h, mm_status_t* status, char** locations, int numLocations, char** fileTypes, int numFilesTypes, int numWorkers){
    if(status) *status = MM_OK;
    //convert a bunch of char**'s to their actual cpp objects that fm expects
    if(numFilesTypes <= 0 || numLocations <= 0 || !h || numWorkers == 0){
        if(status){ *status = MM_ERR_INVALID_ARG;}
        return;
    }
    std::vector<std::string> fs_locations;
    fs_locations.resize(numLocations);
    std::set<std::string> fs_fileTypes;
    try {
        for(int i = 0; i < numLocations; i++){
            if(!locations[i]){ 
                if(status) *status = MM_ERR_INVALID_ARG; 
                return;
            }
            std::string curLocation(locations[i]);
        }
        for(int i = 0; i < numFilesTypes; i++){
            if(!fileTypes[i]){ 
                if(status) *status = MM_ERR_INVALID_ARG; 
                return;
            }
            std::string fileType(fileTypes[i]);
            fs_fileTypes.insert(fileType);
        }
            
        h->fm.refresh_db_files(fs_locations, fs_fileTypes, numWorkers, h->db);
    }
    catch(std::bad_alloc&){
        if(status) *status = MM_ERR_OOM;
        return;
    }
    catch(std::filesystem::filesystem_error&){
        if(status) *status = MM_ERR_FS;
        return;
    }
    catch(std::exception&){
        if(status) *status = MM_ERR_INTERNAL;
        return;
    }
    catch(...){
        if(status) *status = MM_ERR_UNKNOWN;
        return;
    }
        
}
void mm_stage_files(mm_handle_t* h, mm_status_t* status, char** fileList, int numFiles){
    if(status) *status = MM_OK; 
    if(!h || numFiles <= 0)
        if(status) *status = MM_ERR_INVALID_ARG;
    std::vector<std::filesystem::path> filePaths;
    filePaths.resize(numFiles);
    try {
        for(int i = 0; i < numFiles; i++){
        if(!fileList[i]){
            if(status) *status = MM_ERR_INVALID_ARG;
            return;
        }
        std::filesystem::path pathStr = std::string(fileList[i]);
        filePaths.push_back(pathStr);
    }
    h->fm.stage_files(filePaths);
    }
    catch(std::bad_alloc&){
        if(status) *status = MM_ERR_OOM;
        return;
    }
    catch(std::filesystem::filesystem_error&){
        if(status) *status = MM_ERR_FS;
        return;
    }
    catch(std::exception&){
        if(status) *status = MM_ERR_INTERNAL;
        return;
    }
    catch(...){
        if(status) *status = MM_ERR_UNKNOWN;
        return;
    }


}

void mm_destroy(mm_handle_t* h){
    delete h;
}
void mm_free(void* p){
    delete p;
}

