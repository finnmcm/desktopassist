//
//  MacManagerClient.swift
//  Donna
//
//  Created by Finn McMillan on 12/24/25.
//
import MacManagerBridge
import Foundation

final class MacManagerClient {
    private var handle: OpaquePointer?
    

    init() throws {
        var status: mm_status_t = MM_OK
        handle = mm_create(&status)
        if status != MM_OK || handle == nil {
                throw NSError(domain: "MacManager", code: 1)
            }
        }

  deinit {
    if let h = handle { mm_destroy(h) }
  }
    

}
