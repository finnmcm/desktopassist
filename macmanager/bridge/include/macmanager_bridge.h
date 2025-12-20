//BRIDGE ABI exposed for the swift frontend
//extern "C" allows for compatibility between the c++ core and our swift frontend 
#pragma once
#include <stdint.h>

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
// Define OPAQUE TYPE -> we forward the struct so we can use it in our ABI reference, but we actually define what it is in the cpp
// implementation
typedef struct mm_handle mm_handle_t;


mm_handle_t* mm_create(mm_status_t* status);
void mm_destroy(mm_handle_t* h);


void mm_refresh_db_files(mm_handle_t* h, mm_status_t* status, char** locations, int numLocations, char** fileTypes, int numFileTypes, int numWorkers);
//void mm_stage_files(mm_handle)

// Error helpers / memory management
// Any char* returned through out params must be freed with mm_free().
void mm_free(void* p);

// Optional: get last error (per-handle) if you don't want out_error everywhere
const char* mm_last_error(mm_handle_t* h);


#ifdef __cplusplus
} // extern "C"
#endif
