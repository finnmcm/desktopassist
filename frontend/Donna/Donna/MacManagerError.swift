//
//  MacManagerError.swift
//  Donna
//
//  Created by Finn McMillan on 12/24/25.
//

enum MMErrorCode: Int {
    case ok          = 0
    case invalidArg  = 1
    case internalErr = 2
    case outOfMemory = 3

    case notFound    = 4
    case permission  = 5
    case busy        = 6
    case io          = 7
    case db          = 8
    case filesystem  = 9
    case unknown     = 10
}

extension MMErrorCode {
    var description: String {
        switch self {
        case .ok:
            return "No error"
        case .invalidArg:
            return "Invalid argument"
        case .internalErr:
            return "Internal error"
        case .outOfMemory:
            return "Out of memory"
        case .notFound:
            return "Requested item not found"
        case .permission:
            return "Permission denied"
        case .busy:
            return "Resource is busy"
        case .io:
            return "I/O error"
        case .db:
            return "Database error"
        case .filesystem:
            return "Filesystem error"
        case .unknown:
            return "Unknown error"
        }
    }
}
