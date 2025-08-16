#include "usersDb.hpp"
#include "common.hpp"
#include <algorithm>
#include <fstream>
#include <sqlite3.h>
#include <sstream>
#include <stdexcept>

std::ostream &operator<<(std::ostream &os, User &user) {
  os << "Name: " << user.name << std::endl;
  os << "Email: " << user.email << std::endl;
  os << "Id: " << user.id << std::endl;
  return os;
}

usersDbCsv::usersDbCsv() {}

Retval usersDbCsv::verify_credentials(const std::string &username,
                                      const std::string &password) {
  Retval retval = Retval::DB_USERS_ERROR_WRONG_CREDENTIALS;
  if (is_data_read) {
    if (username_to_users.find(username) != username_to_users.end()) {
      if (username_to_users[username]->password == password) {
        retval = Retval::SUCCESS;
      }
    } else {
      retval = Retval::DB_USERS_ERROR_USER_CREDENTIALS_NOT_FOUND;
    }
  } else {
    retval = Retval::DB_ERROR_DATA_NOT_READ_YET;
  }
  return retval;
}

Retval usersDbCsv::read_users_data() {
  Retval retval = Retval::SUCCESS;
  std::string line{};
  std::ifstream fin{"users.csv"};
  if (fin.is_open()) {
    while (std::getline(fin, line)) {
      if (line.empty() || std::all_of(line.begin(), line.end(), [](char c) {
            return std::isspace(static_cast<unsigned char>(c));
          }))
        continue;
      User user{};
      std::string field{};
      std::istringstream line_stream{line};
      // Get the id
      std::getline(line_stream, field, ',');
      user.id = std::stoi(field);
      if (max_id < user.id) {
        max_id = user.id;
      }

      std::getline(line_stream, user.user_name, ',');
      std::getline(line_stream, user.password, ',');
      std::getline(line_stream, user.name, ',');
      std::getline(line_stream, user.email, ',');
      std::getline(line_stream, field, ',');
      user.isAnonymousAllowed = (field == "1");

      id_to_users[user.id] = user;
      username_to_users[user.user_name] = &id_to_users[user.id];
    }

    fin.close();
    is_data_read = true;
  } else {
    retval = Retval::DB_ERROR_READING_DB;
  }

  return retval;
}

Retval usersDbCsv::get_user_id(const std::string &user_name, int &user_id) {
  Retval retval{Retval::SUCCESS};
  user_id = -1;
  if (is_data_read) {
    if (username_to_users.find(user_name) != username_to_users.end()) {
      user_id = username_to_users[user_name]->id;
    }
  }
  return retval;
}

Retval usersDbCsv::add_new_user(User &new_user) {
  Retval retval{Retval::SUCCESS};
  if (is_data_read) {
    max_id++;
    id_to_users[max_id] = new_user;
    username_to_users[new_user.user_name] = &id_to_users[max_id];
    update_db();
  } else {
    retval = Retval::DB_ERROR_DATA_NOT_READ_YET;
  }
  return retval;
}

Retval usersDbCsv::update_db() {
  Retval retval{Retval::SUCCESS};
  if (is_data_read) {
    std::ofstream fout("users.csv");
    for (auto element : id_to_users) {
      fout << element.second.id << ',';
      fout << element.second.user_name << ',';
      fout << element.second.password << ',';
      fout << element.second.name << ',';
      fout << element.second.email << ',';
      fout << (element.second.isAnonymousAllowed ? 1 : 0) << '\n';
    }
    fout.close();
  } else {
    retval = Retval::DB_ERROR_DATA_NOT_READ_YET;
  }
  return retval;
}

Retval usersDbCsv::is_user_exist(int user_id) {
  Retval retval{Retval::DB_USERS_ERROR_USER_ID_NOT_FOUND};
  if (is_data_read) {
    if (id_to_users.find(user_id) != id_to_users.end()) {
      retval = Retval::SUCCESS;
    }
  } else {
    retval = Retval::DB_ERROR_DATA_NOT_READ_YET;
  }
  return retval;
}

Retval usersDbCsv::is_anonymous_questions_allowed(int user_id,
                                                  bool &is_allowed) {
  Retval retval{Retval::DB_USERS_ERROR_USER_ID_NOT_FOUND};
  if (is_data_read) {
    retval = is_user_exist(user_id);
    if (Retval::SUCCESS == retval) {
      retval = Retval::SUCCESS;
      is_allowed = id_to_users[user_id].isAnonymousAllowed;
    }
  } else {
    retval = Retval::DB_ERROR_DATA_NOT_READ_YET;
  }
  return retval;
}

Retval usersDbCsv::print_users(int current_user_id) {
  Retval retval{Retval::SUCCESS};
  if (is_data_read) {
    for (auto id_to_user : id_to_users) {
      if (current_user_id != id_to_user.first) {
        std::cout << id_to_user.second << std::endl;
      }
    }
  } else {
    retval = Retval::DB_ERROR_DATA_NOT_READ_YET;
  }
  return retval;
}

usersDbCsv::~usersDbCsv() {}

usersDbSqlite::usersDbSqlite(const std::string &filename) {
  if (SQLITE_OK != sqlite3_open(filename.c_str(), &db)) {
    throw std::runtime_error("Couldn't open the database!");
  }
  create_table();
}

Retval usersDbSqlite::verify_credentials(const std::string &username,
                                         const std::string &password) {
  Retval retval{Retval::SUCCESS};
  const char *sql{"SELECT 1 FROM users WHERE user_name = ? AND password = ?;"};
  sqlite3_stmt *stmt{nullptr};

  if (SQLITE_OK != sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr)) {
    throw std::runtime_error("SQLite prepare failed: " +
                             std::string(sqlite3_errmsg(db)));
  }

  if (SQLITE_OK !=
      sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT)) {
    sqlite3_finalize(stmt);
    throw std::runtime_error("SQLite bind failed: " +
                             std::string(sqlite3_errmsg(db)));
  }

  if (SQLITE_OK !=
      sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_TRANSIENT)) {
    sqlite3_finalize(stmt);
    throw std::runtime_error("SQLite bind failed: " +
                             std::string(sqlite3_errmsg(db)));
  }

  int step_retval = sqlite3_step(stmt);
  if (step_retval == SQLITE_ROW) {
    retval = Retval::SUCCESS;
  } else if (step_retval == SQLITE_DONE) {
    retval = Retval::DB_USERS_ERROR_USER_CREDENTIALS_NOT_FOUND;
  } else {
    retval = Retval::INVALID_STATE;
  }

  sqlite3_finalize(stmt);
  return retval;
}

Retval usersDbSqlite::get_user_id(const std::string &user_name, int &user_id) {
  Retval retval{Retval::SUCCESS};
  const char *sql = "SELECT id from users WHERE user_name = ?;";
  sqlite3_stmt *stmt{nullptr};

  if (SQLITE_OK != sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr)) {
    throw std::runtime_error("SQLITE PREPARE ERROR: " +
                             std::string(sqlite3_errmsg(db)));
  }

  if (SQLITE_OK !=
      sqlite3_bind_text(stmt, 1, user_name.c_str(), -1, SQLITE_TRANSIENT)) {
    sqlite3_finalize(stmt);
    throw std::runtime_error("SQLITE BIND ERROR: " +
                             std::string(sqlite3_errmsg(db)));
  }

  int rc = sqlite3_step(stmt);
  if (rc == SQLITE_ROW) {
    user_id = sqlite3_column_int(stmt, 0);
  } else if (rc == SQLITE_DONE) {
    retval = Retval::DB_USERS_ERROR_USER_ID_NOT_FOUND;
  } else {
    sqlite3_finalize(stmt);
    throw std::runtime_error("SQLITE STEP ERROR: " +
                             std::string(sqlite3_errmsg(db)));
  }

  sqlite3_finalize(stmt);
  return retval;
}
Retval usersDbSqlite::add_new_user(User &new_user) {
  Retval retval{Retval::SUCCESS};
  char const *sql{
      "INSERT INTO users(name, password, user_name, email, isAnonymousAllowed) "
      "values (?, ?, ?, ?, ?)"};
  sqlite3_stmt *stmt{nullptr};
  if (SQLITE_OK != sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr)) {
    throw std::runtime_error("SQLITE PREPARE ERROR: " +
                             std::string(sqlite3_errmsg(db)));
  }

  if (SQLITE_OK !=
      sqlite3_bind_text(stmt, 1, new_user.name.c_str(), -1, SQLITE_TRANSIENT)) {
    sqlite3_finalize(stmt);
    throw std::runtime_error("SQLITE BIND ERROR: " +
                             std::string(sqlite3_errmsg(db)));
  }

  if (SQLITE_OK != sqlite3_bind_text(stmt, 2, new_user.password.c_str(), -1,
                                     SQLITE_TRANSIENT)) {
    sqlite3_finalize(stmt);
    throw std::runtime_error("SQLITE BIND ERROR: " +
                             std::string(sqlite3_errmsg(db)));
  }

  if (SQLITE_OK != sqlite3_bind_text(stmt, 3, new_user.user_name.c_str(), -1,
                                     SQLITE_TRANSIENT)) {
    sqlite3_finalize(stmt);
    throw std::runtime_error("SQLITE BIND ERROR: " +
                             std::string(sqlite3_errmsg(db)));
  }

  if (SQLITE_OK != sqlite3_bind_text(stmt, 4, new_user.email.c_str(), -1,
                                     SQLITE_TRANSIENT)) {
    sqlite3_finalize(stmt);
    throw std::runtime_error("SQLITE BIND ERROR: " +
                             std::string(sqlite3_errmsg(db)));
  }

  if (SQLITE_OK != sqlite3_bind_int(stmt, 5, new_user.isAnonymousAllowed)) {
    sqlite3_finalize(stmt);
    throw std::runtime_error("SQLITE BIND ERROR: " +
                             std::string(sqlite3_errmsg(db)));
  }

  int rc = sqlite3_step(stmt);
  if (rc != SQLITE_DONE) {
    std::string err = sqlite3_errmsg(db);
    sqlite3_finalize(stmt);
    throw std::runtime_error("SQLITE STEP ERROR: " + err);
  }

  sqlite3_finalize(stmt);
  return retval;
}

Retval usersDbSqlite::is_user_exist(int user_id) {
  Retval retval{Retval::SUCCESS};
  const char *sql{"SELECT 1 FROM users WHERE id = ?;"};
  sqlite3_stmt *stmt{nullptr};

  if (SQLITE_OK != sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr)) {
    throw std::runtime_error("SQLite prepare failed: " +
                             std::string(sqlite3_errmsg(db)));
  }

  if (SQLITE_OK != sqlite3_bind_int(stmt, 1, user_id)) {
    sqlite3_finalize(stmt);
    throw std::runtime_error("SQLite bind failed: " +
                             std::string(sqlite3_errmsg(db)));
  }

  int step_retval = sqlite3_step(stmt);
  if (step_retval == SQLITE_ROW) {
    retval = Retval::SUCCESS;
  } else if (step_retval == SQLITE_DONE) {
    retval = Retval::DB_USERS_ERROR_USER_CREDENTIALS_NOT_FOUND;
  } else {
    std::string err = sqlite3_errmsg(db);
    sqlite3_finalize(stmt);
    throw std::runtime_error("SQLITE STEP ERROR: " + err);
  }

  sqlite3_finalize(stmt);
  return retval;
}

Retval usersDbSqlite::is_anonymous_questions_allowed(int user_id,
                                                     bool &is_allowed) {
  Retval retval{Retval::SUCCESS};

  const char *sql{"SELECT isAnonymousAllowed from users WHERE id = ?;"};
  sqlite3_stmt *stmt{nullptr};

  if (SQLITE_OK != sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr)) {
    throw std::runtime_error("SQLite prepare failed: " +
                             std::string(sqlite3_errmsg(db)));
  }

  if (SQLITE_OK != sqlite3_bind_int(stmt, 1, user_id)) {
    sqlite3_finalize(stmt);
    throw std::runtime_error("SQLite bind failed: " +
                             std::string(sqlite3_errmsg(db)));
  }

  int rv = sqlite3_step(stmt);
  if (SQLITE_ROW == rv) {
    is_allowed = sqlite3_column_int(stmt, 0) ? true : false;
  } else if (rv == SQLITE_DONE) {
    retval = Retval::DB_USERS_ERROR_USER_CREDENTIALS_NOT_FOUND;
  } else {
    std::string err = sqlite3_errmsg(db);
    sqlite3_finalize(stmt);
    throw std::runtime_error("SQLITE STEP ERROR: " + err);
  }
  sqlite3_finalize(stmt);
  return retval;
}

Retval usersDbSqlite::create_table() {
  Retval retval{Retval::SUCCESS};
  const char *sql{"CREATE TABLE IF NOT EXISTS users"
                  "("
                  "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                  "name TEXT UNIQUE NOT NULL, "
                  "password TEXT, "
                  "user_name TEXT, "
                  "email TEXT, "
                  "isAnonymousAllowed INTEGER"
                  ");"};

  char *errMsg = nullptr;
  if (sqlite3_exec(db, sql, nullptr, nullptr, &errMsg) != SQLITE_OK) {
    std::string error = errMsg;
    sqlite3_free(errMsg);
    throw std::runtime_error("SQLite Error: " + error);
  }
  return retval;
}

Retval usersDbSqlite::print_users(int current_user_id) {
  Retval retval{Retval::SUCCESS};
  sqlite3_stmt *stmt{nullptr};
  const char *sql{"SELECT * FROM users WHERE id != ?;"};

  if (SQLITE_OK != sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr)) {
    throw std::runtime_error("Failed to prepare parameter: " +
                             std::string(sqlite3_errmsg(db)));
  }

  if (SQLITE_OK != sqlite3_bind_int(stmt, 1, current_user_id)) {
    throw std::runtime_error("Failed to bind parameter: " +
                             std::string(sqlite3_errmsg(db)));
  }

  while (sqlite3_step(stmt) == SQLITE_ROW) {
    int id = sqlite3_column_int(stmt, 0);
    const unsigned char *name = sqlite3_column_text(stmt, 1);
    const unsigned char *password = sqlite3_column_text(stmt, 2);
    const unsigned char *user_name = sqlite3_column_text(stmt, 3);
    const unsigned char *email = sqlite3_column_text(stmt, 4);
    bool isAnonymousAllowed = sqlite3_column_int(stmt, 5) ? true : false;

    std::cout << "ID: " << id << ", Name: "
              << (name ? reinterpret_cast<const char *>(name) : "NULL")
              << ", password: "
              << (password ? reinterpret_cast<const char *>(password) : "NULL")
              << ", User Name: "
              << (user_name ? reinterpret_cast<const char *>(user_name)
                            : "NULL")
              << ", Email: "
              << (email ? reinterpret_cast<const char *>(email) : "NULL")
              << ", isAnonymousAllowed: " << isAnonymousAllowed << std::endl;
  }

  sqlite3_finalize(stmt);
  return retval;
}

usersDbSqlite::~usersDbSqlite() {
  if (db) {
    sqlite3_close(db);
  }
}
