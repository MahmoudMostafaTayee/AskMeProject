#ifndef __USERSDB_HPP__
#define __USERSDB_HPP__
#include "common.hpp"
#include <iostream>
#include <unordered_map>

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
  usersDb();

  Retval verify_credentials(std::string username, std::string password);

  Retval read_users_data();

  int get_user_id(std::string user_name);

  void add_new_user(User &new_user);

  Retval update_db();

  Retval is_user_exist(int user_id);

  Retval is_anonymous_questions_allowed(int user_id, bool &is_allowed);

  Retval print_users(int current_user_id);

  ~usersDb();

private:
  std::unordered_map<int, User> id_to_users{};
  std::unordered_map<std::string, User *> username_to_users{};
  int max_id{0};
};
#endif /*__USERSDB_HPP__*/