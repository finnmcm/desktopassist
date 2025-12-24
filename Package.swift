// swift-tools-version: 5.9
import PackageDescription

let package = Package(
    name: "MacAssist",
    platforms: [
        .macOS(.v13)
    ],
    products: [
        .library(
            name: "MacManagerBridge",
            targets: ["MacManagerBridge"]
        )
    ],
    targets: [
        .target(
            name: "MacManagerBridge",
            path: "macmanager/bridge",
            sources: [
                "src/macmanager_bridge.cpp"
            ],
            publicHeadersPath: "include",
            cxxSettings: [
                // If you vendored nlohmann headers at:
                // MacAssist/third_party/nlohmann/json.hpp  (or third_party/nlohmann/...)
                .headerSearchPath("../../third_party"),
                .headerSearchPath("include"),
                .unsafeFlags(["-std=c++20"])
            ],
            linkerSettings: [
                // Uncomment if you use sqlite3 directly in the bridge/core:
                 .linkedLibrary("sqlite3")
            ]
        )
    ],
    cxxLanguageStandard: .cxx20
)
