import SwiftUI
import Foundation

struct FolderSelectView: View {
    @StateObject private var store = FolderBookmarks()

    @State private var editingLabelFor: UUID?
    @State private var draftLabel: String = ""

    var body: some View {
        VStack(alignment: .leading, spacing: 12) {
            header

            GroupBox {
                List {
                    ForEach(store.entries) { entry in
                        FolderRow(
                            entry: entry,
                            resolved: store.resolve(id: entry.id),
                            isEditing: editingLabelFor == entry.id,
                            draftLabel: $draftLabel,
                            onStartEdit: {
                                editingLabelFor = entry.id
                                draftLabel = entry.label ?? ""
                            },
                            onCommitEdit: {
                                store.updateLabel(id: entry.id, label: draftLabel)
                                editingLabelFor = nil
                            },
                            onCancelEdit: {
                                editingLabelFor = nil
                                draftLabel = ""
                            },
                            onRemove: {
                                store.remove(id: entry.id)
                            },
                            onReauthorize: {
                                reauthorize(entryID: entry.id)
                            }
                        )
                    }
                }
                .frame(minHeight: 240)
            }

            footer
        }
        .padding(16)
    }

    private var header: some View {
        HStack(alignment: .firstTextBaseline) {
            VStack(alignment: .leading, spacing: 4) {
                Text("Folder Access")
                    .font(.title2).bold()
                Text("Choose folders (Desktop, Downloads, project roots) to allow indexing and search.")
                    .foregroundStyle(.secondary)
            }
            Spacer()
            Button("Add Folder…") {
                addFolders()
            }
            .keyboardShortcut("o", modifiers: [.command])
        }
    }

    private var footer: some View {
        VStack(alignment: .leading, spacing: 6) {
            Text("Notes")
                .font(.headline)
            Text("• Access is limited to folders you select.\n• If a folder is moved/renamed, you may need to reauthorize.")
                .foregroundStyle(.secondary)
                .font(.callout)
        }
        .padding(.top, 4)
    }

    private func addFolders() {
        let urls = FolderPicker.pickFolders(
            allowsMultipleSelection: true,
            title: "Select Folder",
            message: "Choose folders to allow indexing and search.",
            startingAt: FileManager.default.homeDirectoryForCurrentUser
        )
        guard !urls.isEmpty else { return }

        for url in urls {
            // Best-effort dedupe by path hint
            let already = store.entries.contains { $0.pathHint == url.path }
            if already { continue }
            try? store.addFolder(url: url, label: suggestedLabel(for: url))
        }
    }

    private func reauthorize(entryID: UUID) {
        // Start in the last known location if possible
        let startURL = store.entries.first(where: { $0.id == entryID })
            .flatMap { $0.pathHint }
            .map { URL(fileURLWithPath: $0) }

        guard let url = FolderPicker.pickFolder(
            title: "Reauthorize",
            message: "Select the folder again to restore access.",
            startingAt: startURL
        ) else { return }

        try? store.replaceFolder(id: entryID, url: url, label: suggestedLabel(for: url))
    }

    private func suggestedLabel(for url: URL) -> String? {
        let name = url.lastPathComponent
        return name.isEmpty ? nil : name
    }
}

private struct FolderRow: View {
    let entry: FolderBookmarks.Entry
    let resolved: FolderBookmarks.ResolvedFolder?

    let isEditing: Bool
    @Binding var draftLabel: String

    let onStartEdit: () -> Void
    let onCommitEdit: () -> Void
    let onCancelEdit: () -> Void
    let onRemove: () -> Void
    let onReauthorize: () -> Void

    var body: some View {
        HStack(spacing: 12) {
            Image(systemName: resolved == nil ? "exclamationmark.triangle.fill" : "folder.fill")
                .foregroundStyle(resolved == nil ? .orange : .accentColor)

            VStack(alignment: .leading, spacing: 2) {
                HStack {
                    if isEditing {
                        TextField("Label", text: $draftLabel)
                            .textFieldStyle(.roundedBorder)
                            .frame(maxWidth: 240)
                        Button("Save") { onCommitEdit() }
                        Button("Cancel") { onCancelEdit() }
                    } else {
                        Text(entry.label?.isEmpty == false ? entry.label! : "Untitled Folder")
                            .font(.headline)
                        Button { onStartEdit() } label: {
                            Image(systemName: "pencil")
                        }
                        .buttonStyle(.plain)
                        .foregroundStyle(.secondary)
                    }

                    Spacer()

                    if resolved == nil {
                        Button("Reauthorize…") { onReauthorize() }
                    }
                }

                Text(displayPath)
                    .font(.callout)
                    .foregroundStyle(.secondary)
                    .lineLimit(1)
                    .truncationMode(.middle)
            }

            Button(role: .destructive) { onRemove() } label: {
                Image(systemName: "trash")
            }
            .buttonStyle(.borderless)
        }
        .padding(.vertical, 4)
    }

    private var displayPath: String {
        if let r = resolved { return r.url.path }
        return entry.pathHint ?? "(unavailable)"
    }
}

// MARK: - Preview

#Preview {
    FolderSelectView()
        .frame(width: 720, height: 420)
}
