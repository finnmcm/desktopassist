// swift-tools-version: 5.9
import PackageDescription

let package = Package(
  name: "MacAssist",
  platforms: [.macOS(.v13)],
  products: [
    .library(name: "MacManagerBridge", targets: ["MacManagerBridge"])
  ],
  targets: [
    .target(
    name: "NlohmannJSON",
    path: "third_party/nlohmann",
    publicHeadersPath: "include"
  ),
    .target(
  name: "MacManagerCore",
  path: "macmanager/core",
  sources: ["src"],
  publicHeadersPath: "include",
  cxxSettings: [.unsafeFlags(["-std=c++20"])],
  linkerSettings: [.linkedLibrary("sqlite3")]
),
.target(
  name: "MacManagerBridge",
  dependencies: ["MacManagerCore"],
  path: "macmanager/bridge",
  sources: ["src"],
  publicHeadersPath: "include",
  cxxSettings: [.unsafeFlags(["-std=c++20"])],
  linkerSettings: [.linkedLibrary("sqlite3")]
)
  ]
)
