#include <sqlite3.h>
#include <string>
#include <stdexcept>
#include <vector>
#include <iostream>

namespace macmanager {
struct DbFile {
        std::string filepath;
        std::string filename;
        std::string extension;
        int64_t last_modified;
        size_t filesize;
    };

class Database {
public:
    explicit Database(const std::string& path) {
        if (sqlite3_open(path.c_str(), &db) != SQLITE_OK) {
            throw std::runtime_error("Failed to open DB");
        }
    }

    ~Database() {
        sqlite3_close(db);
    }
    void check(int rc, sqlite3* db, const char* msg) {
        if (rc != SQLITE_OK && rc != SQLITE_ROW && rc != SQLITE_DONE) {
            std::cerr << msg << ": " << sqlite3_errmsg(db) << "\n";
            std::exit(1);
        }
    }

    void exec(const std::string& sql) {
        char* err = nullptr;
        if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &err) != SQLITE_OK) {
            std::string msg = err;
            sqlite3_free(err);
            throw std::runtime_error(msg);
        }
    }
    void insert_file(const DbFile& f){
        exec(
        "INSERT OR REPLACE INTO files "
        "(path, filename, extension, size, content_hash, last_modified) VALUES ("
        "'" + f.filepath + "', "
        "'" + f.filename + "', "
        "'" + f.extension + "', "
        + std::to_string(f.filesize) + ", "
        "'" + "null" + "', "
        + std::to_string(f.last_modified) +
        ");"
    );
    }
    std::vector<std::pair<std::string, std::string>> listObjects(){
    // Exclude internal sqlite_* tables unless you want them.
    const char* sql =
        "SELECT name, type "
        "FROM sqlite_master "
        "WHERE type IN ('table','view') "
        "  AND name NOT LIKE 'sqlite_%' "
        "ORDER BY type, name;";

    sqlite3_stmt* stmt = nullptr;
    check(sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr), db, "prepare(listObjects)");

    std::vector<std::pair<std::string, std::string>> out;
    while (true) {
        int rc = sqlite3_step(stmt);
        if (rc == SQLITE_DONE) break;
        check(rc, db, "step(listObjects)");
        const unsigned char* name = sqlite3_column_text(stmt, 0);
        const unsigned char* type = sqlite3_column_text(stmt, 1);
        out.emplace_back(reinterpret_cast<const char*>(name),
                         reinterpret_cast<const char*>(type));
    }
    sqlite3_finalize(stmt);
    return out;
}
 void printTableInfo(const std::string& table) {
    // PRAGMA statements can’t be bound with ? placeholders reliably for identifiers,
    // so we carefully quote the table name. (If table names are trusted, this is fine.)
    std::string sql = "PRAGMA table_info(\"" + table + "\");";

    sqlite3_stmt* stmt = nullptr;
    check(sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr), db, "prepare(table_info)");

    std::cout << "  Columns:\n";
    // table_info columns: cid, name, type, notnull, dflt_value, pk
    while (true) {
        int rc = sqlite3_step(stmt);
        if (rc == SQLITE_DONE) break;
        check(rc, db, "step(table_info)");

        int cid = sqlite3_column_int(stmt, 0);
        const char* name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        const char* type = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        int notnull = sqlite3_column_int(stmt, 3);
        const char* dflt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        int pk = sqlite3_column_int(stmt, 5);

        std::cout << "    [" << cid << "] " << (name ? name : "")
                  << " " << (type ? type : "")
                  << (notnull ? " NOT NULL" : "")
                  << (pk ? " PRIMARY KEY" : "")
                  << (dflt ? (std::string(" DEFAULT ") + dflt) : "")
                  << "\n";
    }
    sqlite3_finalize(stmt);
}

void printIndexes(const std::string& table) {
    std::string sql = "PRAGMA index_list(\"" + table + "\");";
    sqlite3_stmt* stmt = nullptr;
    check(sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr), db, "prepare(index_list)");

    // index_list columns: seq, name, unique, origin, partial  (varies slightly by version)
    bool any = false;
    while (true) {
        int rc = sqlite3_step(stmt);
        if (rc == SQLITE_DONE) break;
        check(rc, db, "step(index_list)");
        any = true;

        const char* idxName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        int unique = sqlite3_column_int(stmt, 2);

        std::cout << "  Index: " << (idxName ? idxName : "")
                  << (unique ? " (UNIQUE)" : "") << "\n";

        // Print indexed columns
        if (idxName && *idxName) {
            std::string sql2 = "PRAGMA index_info(\"" + std::string(idxName) + "\");";
            sqlite3_stmt* stmt2 = nullptr;
            check(sqlite3_prepare_v2(db, sql2.c_str(), -1, &stmt2, nullptr), db, "prepare(index_info)");

            std::cout << "    Columns:";
            bool first = true;
            while (true) {
                int rc2 = sqlite3_step(stmt2);
                if (rc2 == SQLITE_DONE) break;
                check(rc2, db, "step(index_info)");
                const char* col = reinterpret_cast<const char*>(sqlite3_column_text(stmt2, 2)); // name
                std::cout << (first ? " " : ", ") << (col ? col : "");
                first = false;
            }
            std::cout << "\n";
            sqlite3_finalize(stmt2);
        }
    }
    sqlite3_finalize(stmt);

    if (!any) std::cout << "  Indexes: (none)\n";
}

void printAllTablesAndSchema() {
    sqlite3* db = nullptr;

    auto objects = listObjects();
    for (const auto& [name, type] : objects) {
        std::cout << "\n" << type << ": " << name << "\n";
        if (type == "table") {
            printTableInfo(name);
            printIndexes(name);
        } else {
            std::cout << "  (view schema lives in sqlite_master.sql)\n";
        }
    }

    sqlite3_close(db);
}

private:
    sqlite3* db;
};
}
