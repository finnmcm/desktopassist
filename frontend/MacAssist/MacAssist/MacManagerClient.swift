//
//  MacManagerClient.swift
//  MacAssist
//
//  Created by Finn McMillan on 12/24/25.
//

import MacManagerBridge
import Foundation

final class MacManagerClient {
    private var handle: OpaquePointer?
    private var status: UnsafeMutablePointer<mm_status_t>?
    

    init() throws {
            handle = mm_create(status)
        if status?.pointee != MM_OK || handle == nil {
                throw NSError(domain: "MacManager", code: 1)
            }
        }

  deinit {
    if let h = handle { mm_destroy(h) }
  }

}
