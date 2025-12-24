// swift-tools-version: 5.9
import PackageDescription

let package = Package(
  name: "MacAssist",
  platforms: [
    .macOS(.v10_15)
  ],
  products: [
    .library(name: "MacAssist", targets: ["MacManagerBridge"])
  ],
  targets: [
    // 1) C++ core
    .target(
      name: "MacManagerCore",
      path: "macmanager/core",
      publicHeadersPath: "include",
      cxxSettings: [
        .headerSearchPath("include"),
        .define("SQLITE_THREADSAFE", to: "1"),
        .unsafeFlags(["-std=c++20"]) // or c++17
      ],
      linkerSettings: [
        // add system libs here if needed
      ]
    ),

    // 2) C bridge that Swift imports
    .target(
      name: "MacManagerBridge",
      dependencies: ["MacManagerCore"],
      path: "macmanager/bridge",
      publicHeadersPath: "include",
      cSettings: [
        .headerSearchPath("include")
      ],
      cxxSettings: [
        // if your bridge implementation is .cpp
        .unsafeFlags(["-std=c++20"])
      ],
      linkerSettings: [
        // e.g. sqlite3 if you use system sqlite:
         .linkedLibrary("sqlite3")
      ]
    )
  ]
)
