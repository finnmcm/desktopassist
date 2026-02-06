//
//  ContentView.swift
//  Donna
//
//  Created by Finn McMillan on 12/24/25.
//

import SwiftUI

struct RootView: View {
    @Environment(\.macManager) private var mm_client
    @State var locations: [String] = ["Desktop", "Downloads", "Documents"]
    @State var fileTypes: [String] = [".pdf", ".txt", ".mp3"]
    
    var body: some View {
        ZStack{
            FolderSelectView()
            VStack {
                Button("refresn files"){
                    Task{
                        do {
                            try mm_client.refresh_db_files(locations: locations, file_types: fileTypes)
                        } catch {
                            
                        }
                    }
                }
            }
            .padding()
        }
    }
}

#Preview {
    RootView()
}
