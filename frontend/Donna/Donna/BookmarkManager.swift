import SwiftUI
import AppKit
import Foundation

// MARK: - Bookmark Store (UserDefaults-backed)

final class FolderBookmarks: ObservableObject {
    private let defaultsKey = "folder_bookmarks_v1"

    struct Entry: Codable, Identifiable, Equatable {
        let id: UUID
        var label: String?
        var bookmark: Data
        var pathHint: String?
        var createdAt: Date
    }

    struct ResolvedFolder: Identifiable {
        let id: UUID
        let label: String?
        let url: URL
        let isStale: Bool
    }

    @Published private(set) var entries: [Entry] = []

    init() {
        entries = load()
    }

    func addFolder(url: URL, label: String? = nil) throws {
        let bookmark = try url.bookmarkData(
            options: [.withSecurityScope],
            includingResourceValuesForKeys: nil,
            relativeTo: nil
        )

        let entry = Entry(
            id: UUID(),
            label: label,
            bookmark: bookmark,
            pathHint: url.path,
            createdAt: Date()
        )

        entries.append(entry)
        save(entries)

        _ = resolve(id: entry.id) // refresh if stale
    }

    func replaceFolder(id: UUID, url: URL, label: String?) throws {
        // Replace bookmark in-place (nice for reauth)
        guard let i = entries.firstIndex(where: { $0.id == id }) else { return }
        let bookmark = try url.bookmarkData(options: [.withSecurityScope],
                                            includingResourceValuesForKeys: nil,
                                            relativeTo: nil)
        entries[i].bookmark = bookmark
        entries[i].pathHint = url.path
        entries[i].label = label
        save(entries)
    }

    func remove(id: UUID) {
        entries.removeAll { $0.id == id }
        save(entries)
    }

    func updateLabel(id: UUID, label: String?) {
        guard let i = entries.firstIndex(where: { $0.id == id }) else { return }
        entries[i].label = (label?.isEmpty == true) ? nil : label
        save(entries)
    }

    func resolve(id: UUID) -> ResolvedFolder? {
        guard let e = entries.first(where: { $0.id == id }) else { return nil }
        var isStale = false
        do {
            let url = try URL(
                resolvingBookmarkData: e.bookmark,
                options: [.withSecurityScope],
                relativeTo: nil,
                bookmarkDataIsStale: &isStale
            )

            if isStale {
                refreshBookmark(id: id, resolvedURL: url)
            }

            return ResolvedFolder(id: id, label: e.label, url: url, isStale: isStale)
        } catch {
            return nil
        }
    }

    func resolveAll() -> [ResolvedFolder] {
        entries.compactMap { resolve(id: $0.id) }
    }

    func withSecurityScope<T>(_ url: URL, _ body: () throws -> T) rethrows -> T {
        let ok = url.startAccessingSecurityScopedResource()
        defer { if ok { url.stopAccessingSecurityScopedResource() } }
        return try body()
    }

    // MARK: - Persistence

    private func load() -> [Entry] {
        guard let data = UserDefaults.standard.data(forKey: defaultsKey) else { return [] }
        return (try? JSONDecoder().decode([Entry].self, from: data)) ?? []
    }

    private func save(_ entries: [Entry]) {
        self.entries = entries
        let data = try? JSONEncoder().encode(entries)
        UserDefaults.standard.set(data, forKey: defaultsKey)
    }

    private func refreshBookmark(id: UUID, resolvedURL: URL) {
        guard let i = entries.firstIndex(where: { $0.id == id }) else { return }
        guard let newBookmark = try? resolvedURL.bookmarkData(
            options: [.withSecurityScope],
            includingResourceValuesForKeys: nil,
            relativeTo: nil
        ) else { return }

        entries[i].bookmark = newBookmark
        entries[i].pathHint = resolvedURL.path
        save(entries)
    }
}

// MARK: - NSOpenPanel Bridge

enum FolderPicker {
    /// Pick one or more folders. Returns empty array on cancel.
    static func pickFolders(
        allowsMultipleSelection: Bool,
        title: String = "Select Folder",
        message: String = "Choose folders to allow indexing and search.",
        startingAt startURL: URL? = FileManager.default.homeDirectoryForCurrentUser
    ) -> [URL] {
        let panel = NSOpenPanel()
        panel.canChooseFiles = false
        panel.canChooseDirectories = true
        panel.allowsMultipleSelection = allowsMultipleSelection
        panel.prompt = title
        panel.message = message
        panel.directoryURL = startURL

        let response = panel.runModal()
        guard response == .OK else { return [] }
        return panel.urls
    }

    /// Pick exactly one folder. Returns nil on cancel.
    static func pickFolder(
        title: String = "Select Folder",
        message: String = "Choose a folder.",
        startingAt startURL: URL? = FileManager.default.homeDirectoryForCurrentUser
    ) -> URL? {
        pickFolders(allowsMultipleSelection: false, title: title, message: message, startingAt: startURL).first
    }
}

