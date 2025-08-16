#ifndef __USERSDB_HPP__
#define __USERSDB_HPP__
#include "common.hpp"
#include <iostream>
#include <unordered_map>
#include <sqlite3.h>

class User {
public:
  int id{};
  std::string user_name{};
  std::string name{};
  std::string password{};
  std::string email{};
  bool isAnonymousAllowed{};

  friend std::ostream &operator<<(std::ostream &os, User &user);
};

class usersDb {
public:
  virtual Retval verify_credentials(const std::string& username,
                                    const std::string& password) = 0;
  virtual Retval read_users_data(){
    return Retval::SUCCESS;
  }
  virtual Retval get_user_id(const std::string& user_name, int& user_id) = 0;
  virtual Retval add_new_user(User &new_user) = 0;
  virtual Retval update_db(){
    return Retval::SUCCESS;
  }
  virtual Retval is_user_exist(int user_id) = 0;
  virtual Retval is_anonymous_questions_allowed(int user_id,
                                                bool &is_allowed) = 0;
  virtual Retval print_users(int current_user_id) = 0;
  virtual ~usersDb() = default;

protected:
  std::unordered_map<int, User> id_to_users;
  std::unordered_map<std::string, User *> username_to_users;
  int max_id{0};
  bool is_data_read{false};
};

class usersDbCsv : public usersDb {
public:
  usersDbCsv();
  Retval verify_credentials(const std::string& username,
                            const std::string& password) override;
  Retval read_users_data() override;
  Retval get_user_id(const std::string& user_name, int& user_id) override;
  Retval add_new_user(User &new_user) override;
  Retval update_db() override;
  Retval is_user_exist(int user_id) override;
  Retval is_anonymous_questions_allowed(int user_id, bool &is_allowed) override;
  Retval print_users(int current_user_id) override;
  ~usersDbCsv();

private:
  std::unordered_map<int, User> id_to_users{};
  std::unordered_map<std::string, User *> username_to_users{};
  int max_id{0};
};

class usersDbSqlite : public usersDb {
public:
  usersDbSqlite(const std::string& filename="users.db");
  Retval verify_credentials(const std::string& username,
                            const std::string &password) override;
  Retval get_user_id(const std::string& user_name, int& user_id) override;
  Retval add_new_user(User &new_user) override;
  Retval is_user_exist(int user_id) override;
  Retval is_anonymous_questions_allowed(int user_id, bool &is_allowed) override;
  Retval print_users(int current_user_id) override;
  ~usersDbSqlite();

private:
  Retval create_table();
  std::unordered_map<int, User> id_to_users{};
  std::unordered_map<std::string, User *> username_to_users{};
  int max_id{0};
  sqlite3* db{nullptr};
};
#endif /*__USERSDB_HPP__*/
