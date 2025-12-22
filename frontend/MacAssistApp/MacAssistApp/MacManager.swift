//
//  MacManager.swift
//  MacAssistApp
//
//  Created by Finn McMillan on 12/22/25.
//

import MacManagerBridge
import Foundation
final class MacManager {
    private var handle: UnsafeMutablePointer<mm_handle_t>?

    init() throws {
        var status: mm_status_t = MM_OK
        handle = mm_create(&status)

        guard status == MM_OK, handle != nil else {
            throw NSError(domain: "MacManager", code: 1)
        }
    }

    deinit {
        if let h = handle {
            mm_destroy(h)
        }
    }
}

