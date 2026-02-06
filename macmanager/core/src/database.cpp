#include <sqlite3.h>
#include <string>
#include <stdexcept>
#include <vector>
#include <iostream>
#include <nlohmann/json.hpp>


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
        std::cerr << "[Database] constructor path=" << path << "\n";
        if (sqlite3_open(path.c_str(), &db) != SQLITE_OK) {
            std::cerr << "[Database] sqlite3_open failed for: " << path << std::endl;
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
    std::cout << "file successfully written!" << std::endl;
    }

    std::vector<DbFile> query_files(const std::vector<std::string>& file_names, const std::vector<std::string>& file_extensions, 
                                    int64_t modified_within, int64_t modified_after){
        //SQL QUERY STRUCTURE                             
        constexpr const char* kQueryFilesSql = R"SQL(
            SELECT path, filename, extension, size, content_hash, last_modified
            FROM files
            WHERE
                (:num_filenames = 0 OR EXISTS (
                    SELECT 1
                    FROM json_each(:filenames)
                    WHERE files.filename LIKE '%' || value || '%'
                ))
            AND
                (:num_extensions = 0 OR files.extension IN (
                    SELECT value FROM json_each(:extensions)
                ))
            AND
                (:modified_within IS NULL
                OR files.last_modified >= (strftime('%s','now') - :modified_within))
            AND
                (:modified_after IS NULL
                OR files.last_modified >= :modified_after)
            ;
            )SQL";

        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, kQueryFilesSql, -1, &stmt, nullptr) != SQLITE_OK) {
                throw std::runtime_error(sqlite3_errmsg(db));
        }

        auto finalize = [&]() {
            if (stmt) sqlite3_finalize(stmt);
            stmt = nullptr;
        };
        try {
            //bind our parameters to the query
            const std::string filename_json = to_json_array(file_names);
            const std::string file_ext_json = to_json_array(file_extensions);
            bind_text(stmt, ":filenames", filename_json);
            bind_int64(stmt, ":num_filenames", (int64_t)file_names.size());

            bind_text(stmt, ":extensions", file_ext_json);
            bind_int64(stmt, ":num_extensions", (int64_t)file_extensions.size());

            if(modified_after != -1) bind_int64(stmt, ":modified_after", modified_after);
            else bind_null(stmt, ":modified_after");

            if(modified_within != -1) bind_int64(stmt, ":modified_within", modified_within);
            else bind_null(stmt, ":modified_within");

            std::vector<DbFile> files;
            while(true){
                const int rc = sqlite3_step(stmt);
                if (rc == SQLITE_ROW){
                    DbFile f;
                    const unsigned char* c0 = sqlite3_column_text(stmt, 0);
                    const unsigned char* c1 = sqlite3_column_text(stmt, 1);
                    const unsigned char* c2 = sqlite3_column_text(stmt, 2);
                //    const unsigned char* c4 = sqlite3_column_text(stmt, 4);

                    f.filepath     = c0 ? reinterpret_cast<const char*>(c0) : "";
                    f.filename     = c1 ? reinterpret_cast<const char*>(c1) : "";
                    f.extension    = c2 ? reinterpret_cast<const char*>(c2) : "";
                    f.filesize     = sqlite3_column_int64(stmt, 3);
                  //  f.content_hash = c4 ? reinterpret_cast<const char*>(c4) : "";
                  f.last_modified  = sqlite3_column_int64(stmt, 5);

                  files.push_back(f);
                }
                else if (rc == SQLITE_DONE) break;
                else throw std::runtime_error(sqlite3_errmsg(db));
            }
            finalize();
            return files;
        }
        catch(...){
            finalize();
            throw;
        }



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

/*=========
SQL PARAMETER BUILDERS - use these to bind parameters for non-static queries
=========*/
static int bind_index(sqlite3_stmt* stmt, const char* name) {
    int idx = sqlite3_bind_parameter_index(stmt, name);
    if (idx == 0) throw std::runtime_error(std::string("Missing bind parameter: ") + name);
    return idx;
}

static void bind_text(sqlite3_stmt* stmt, const char* name, const std::string& val) {
    int idx = bind_index(stmt, name);
    if (sqlite3_bind_text(stmt, idx, val.c_str(), (int)val.size(), SQLITE_TRANSIENT) != SQLITE_OK) {
        throw std::runtime_error("sqlite3_bind_text failed");
    }
}

static void bind_int64(sqlite3_stmt* stmt, const char* name, int64_t val) {
    int idx = bind_index(stmt, name);
    if (sqlite3_bind_int64(stmt, idx, val) != SQLITE_OK) {
        throw std::runtime_error("sqlite3_bind_int64 failed");
    }
}

static void bind_null(sqlite3_stmt* stmt, const char* name) {
    int idx = bind_index(stmt, name);
    if (sqlite3_bind_null(stmt, idx) != SQLITE_OK) {
        throw std::runtime_error("sqlite3_bind_null failed");
    }
}

static std::string to_json_array(const std::vector<std::string>& items) {
    nlohmann::json j = nlohmann::json::array();
    for (const auto& s : items) j.push_back(s);

    return j.dump();
}
private:
    sqlite3* db;
};
}
