//
//  DonnaApp.swift
//  Donna
//
//  Created by Finn McMillan on 12/24/25.
//

import SwiftUI

private struct MacManagerClientKey: EnvironmentKey {
    static let defaultValue = MacManagerClient()
}

extension EnvironmentValues {
    var macManager: MacManagerClient {
        get { self[MacManagerClientKey.self] }
        set { self[MacManagerClientKey.self] = newValue }
    }
}
@main
struct DonnaApp: App {
    private let mm_client = MacManagerClient()
    init() {
        do { try mm_client.start() }
        catch {
            print("MacManager failed to start: ", error)
        }
        
    }
    var body: some Scene {
        WindowGroup {
            RootView()
                .environment(\.macManager, mm_client)
        }
        .defaultSize(width: 900, height: 600)
    }
}
