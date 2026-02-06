//
//  MacManagerClient.swift
//  Donna
//
//  Created by Finn McMillan on 12/24/25.
//
import MacManagerBridge
import Foundation


func make_mm_string_array(arr: [String]) -> mm_string_array_t{
/*
 IMPORTANT: this is a C-style allocation, so we handle freeing/destroying in our C++ core, rather than in swift
 -> to do this, use mm_free_string_array
 */
    let arrSize = arr.count
    let cArray = UnsafeMutablePointer<UnsafeMutablePointer<CChar>?>.allocate(capacity: arrSize)
    for (i, s) in arr.enumerated(){
        cArray[i] = strdup(s)
    }
    
    return mm_string_array(data: cArray, size: arrSize)
    
}
func mmNSError(_ status: mm_status_t, message: String? = nil, underlying: Error? = nil) -> NSError {
    let code = MMErrorCode(rawValue: Int(status.rawValue)) ?? .unknown

        var userInfo: [String: Any] = [
            NSLocalizedDescriptionKey: message ?? code.description
        ]

        if let underlying {
            userInfo[NSUnderlyingErrorKey] = underlying
        }

        return NSError(
            domain: "com.finnmcm.donna",
            code: code.rawValue,
            userInfo: userInfo
        )
}

final class MacManagerClient {
    private var handle: OpaquePointer?

    deinit {
        if let h = handle { mm_destroy(h) }
    }
    
    func start() throws {
        var status: mm_status_t = MM_OK

        let fm = FileManager.default
        let appSupport = try fm.url(for: .applicationSupportDirectory,
                                    in: .userDomainMask,
                                    appropriateFor: nil,
                                    create: true)

        // This will be inside:
        // ~/Library/Containers/com.finnmcm.Donna/Data/Library/Application Support/
        let dbDir = appSupport.appendingPathComponent("Donna", isDirectory: true)
        try fm.createDirectory(at: dbDir, withIntermediateDirectories: true)

        let dbURL = dbDir.appendingPathComponent("state.db")
        
        dbURL.path.withCString { c_dbPath in
                handle = mm_create(&status, c_dbPath)
        }

        if status != MM_OK || handle == nil {
            throw NSError(domain: "MacManager", code: 0)
        }
    }
    func refresh_db_files(locations: [String], file_types: [String]) throws{
        var status: mm_status_t = MM_OK
        let numWorkers = Int32(2) //hardcoded for now
        var mm_locations_arr = make_mm_string_array(arr: locations)
        var mm_file_types_arr = make_mm_string_array(arr: file_types)
        mm_refresh_db_files(handle, &status, &mm_locations_arr, &mm_file_types_arr, numWorkers)
        guard status == MM_OK else {
            throw mmNSError(status)
        }
    }

}
