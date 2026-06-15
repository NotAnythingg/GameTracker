#include "database.h"
#include "sqlite3.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>
#include <iomanip>
#include <windows.h>

// Вспомогательная функция для получения текущей даты
std::string getCurrentDate() {
    time_t now = time(0);
    tm* ltm = localtime(&now);
    std::ostringstream oss;
    oss << 1900 + ltm->tm_year << "-" << 1 + ltm->tm_mon << "-" << ltm->tm_mday;
    return oss.str();
}

Database::Database() : db(nullptr) {}

Database::~Database() {
    if (db) {
        sqlite3_close((sqlite3*)db);
    }
}

bool Database::init() {
    OutputDebugStringA("DEBUG init: Opening database...\n");

    int rc = sqlite3_open("games.db", (sqlite3**)&db);
    if (rc) {
        std::string err = "DEBUG init: Failed to open DB - ";
        err += sqlite3_errmsg((sqlite3*)db);
        err += "\n";
        OutputDebugStringA(err.c_str());
        return false;
    }
    OutputDebugStringA("DEBUG init: Database opened successfully\n");

    const char* sql = "CREATE TABLE IF NOT EXISTS games ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "title TEXT NOT NULL,"
        "status TEXT CHECK(status IN ('Не начата', 'Играю', 'Пройдена', 'Заброшена')),"
        "progress_percent INTEGER DEFAULT 0 CHECK(progress_percent >= 0 AND progress_percent <= 100),"
        "hours_played REAL DEFAULT 0.0,"
        "last_updated TEXT);";

    char* errMsg = nullptr;
    rc = sqlite3_exec((sqlite3*)db, sql, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::string err = "DEBUG init: SQL Error - ";
        err += errMsg;
        err += "\n";
        OutputDebugStringA(err.c_str());
        sqlite3_free(errMsg);
        return false;
    }

    OutputDebugStringA("DEBUG init: Table created/verified successfully\n");
    return true;
}

bool Database::addGame(const std::string& title, const std::string& status) {
    OutputDebugStringA("DEBUG addGame: Starting...\n");
    OutputDebugStringA(("DEBUG addGame: Title = '" + title + "'\n").c_str());
    OutputDebugStringA(("DEBUG addGame: Status = '" + status + "'\n").c_str());

    const char* sql = "INSERT INTO games (title, status, progress_percent, hours_played, last_updated) VALUES (?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt;

    OutputDebugStringA("DEBUG addGame: Preparing SQL...\n");
    int rc = sqlite3_prepare_v2((sqlite3*)db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::string err = "DEBUG addGame: Prepare failed - ";
        err += sqlite3_errmsg((sqlite3*)db);
        err += "\n";
        OutputDebugStringA(err.c_str());
        return false;
    }

    OutputDebugStringA("DEBUG addGame: Binding parameters...\n");
    sqlite3_bind_text(stmt, 1, title.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, status.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, 0);  // progress_percent
    sqlite3_bind_double(stmt, 4, 0.0);  // hours_played
    std::string date = getCurrentDate();
    sqlite3_bind_text(stmt, 5, date.c_str(), -1, SQLITE_STATIC);

    OutputDebugStringA("DEBUG addGame: Executing step...\n");
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        std::string err = "DEBUG addGame: Step failed - ";
        err += sqlite3_errmsg((sqlite3*)db);
        err += "\n";
        OutputDebugStringA(err.c_str());
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    OutputDebugStringA("DEBUG addGame: SUCCESS!\n");
    return true;
}

std::vector<Game> Database::getAllGames() {
    std::vector<Game> games;
    const char* sql = "SELECT * FROM games;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2((sqlite3*)db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            Game g;
            g.id = sqlite3_column_int(stmt, 0);
            g.title = (const char*)sqlite3_column_text(stmt, 1);
            g.status = (const char*)sqlite3_column_text(stmt, 2);
            g.progress_percent = sqlite3_column_int(stmt, 3);
            g.hours_played = sqlite3_column_double(stmt, 4);
            g.last_updated = (const char*)sqlite3_column_text(stmt, 5);
            games.push_back(g);
        }
    }
    sqlite3_finalize(stmt);
    return games;
}

bool Database::updateProgress(int id, int progress, double hours) {
    const char* sql = "UPDATE games SET progress_percent = ?, hours_played = ?, last_updated = ? WHERE id = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2((sqlite3*)db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_int(stmt, 1, progress);
    sqlite3_bind_double(stmt, 2, hours);
    std::string date = getCurrentDate();
    sqlite3_bind_text(stmt, 3, date.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 4, id);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE;
}

bool Database::updateStatus(int id, const std::string& newStatus) {
    const char* sql = "UPDATE games SET status = ? WHERE id = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2((sqlite3*)db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, newStatus.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, id);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE;
}

bool Database::deleteGame(int id) {
    const char* sql = "DELETE FROM games WHERE id = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2((sqlite3*)db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_int(stmt, 1, id);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE;
}

bool Database::updateGame(int id, const std::string& title, const std::string& status,
    int progress, double hours) {
    OutputDebugStringA("DEBUG updateGame: Starting...\n");

    const char* sql = "UPDATE games SET title = ?, status = ?, progress_percent = ?, "
        "hours_played = ?, last_updated = ? WHERE id = ?;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2((sqlite3*)db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        OutputDebugStringA("DEBUG updateGame: Prepare failed\n");
        return false;
    }

    sqlite3_bind_text(stmt, 1, title.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, status.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, progress);
    sqlite3_bind_double(stmt, 4, hours);
    std::string date = getCurrentDate();
    sqlite3_bind_text(stmt, 5, date.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 6, id);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc == SQLITE_DONE) {
        OutputDebugStringA("DEBUG updateGame: SUCCESS\n");
        return true;
    }
    else {
        OutputDebugStringA("DEBUG updateGame: FAILED\n");
        return false;
    }
}