//BRIDGE ABI exposed for the swift frontend
//extern "C" allows for compatibility between the c++ core and our swift frontend 
#pragma once
#include <stdint.h>
#include <stddef.h>


#ifdef __cplusplus
extern "C" {
#endif

typedef enum mm_status {
    MM_OK = 0,
    MM_ERR_INVALID_ARG = 1,
    MM_ERR_INTERNAL = 2,
    MM_ERR_OOM = 3, //out of memory
    

    MM_ERR_NOT_FOUND = 4,
    MM_ERR_PERM = 5,
    MM_ERR_BUSY = 6,
    MM_ERR_IO = 7,
    MM_ERR_DB = 8, //DB error (vague, will classify later)
    MM_ERR_FS = 9, //filesys error
    MM_ERR_UNKNOWN = 10


} mm_status_t;

typedef struct mm_get_file_args {
    char** file_names;
    char** file_extensions;
    int64_t modified_within; //checks whether file has been modified within the last modified_within timeframe
    int64_t modified_after; //checks whether file has been modified after given time
    size_t num_file_names;
    size_t num_file_extensions;
} mm_get_file_args_t;

typedef struct mm_string_array {
    char** data;
    size_t size;
} mm_string_array_t;
// Define OPAQUE TYPE -> we forward the struct so we can use it in our ABI reference, but we actually define what it is in the cpp
// implementation
typedef struct mm_handle mm_handle_t;


mm_handle_t* mm_create(mm_status_t* status);
void mm_destroy(mm_handle_t* h);

void mm_get_file_paths(mm_handle_t* h, mm_status_t* status, mm_get_file_args_t* args, mm_string_array_t* file_paths);
//TODO: change these to mm_string_arrays
void mm_refresh_db_files(mm_handle_t* h, mm_status_t* status, char** locations, int numLocations, char** fileTypes, int numFileTypes, int numWorkers);
//TODO -> change the char** and int to a mm_string_array
void mm_stage_files(mm_handle_t* h, mm_status_t* status, char** fileList, int numFiles);

// Error helpers / memory management
// Any char* returned through out params must be freed with mm_free().
void mm_free(void* p);

// Optional: get last error (per-handle) if you don't want out_error everywhere
const char* mm_last_error(mm_handle_t* h);


#ifdef __cplusplus
} // extern "C"
#endif
