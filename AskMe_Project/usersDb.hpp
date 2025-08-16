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
  virtual Retval verify_credentials(std::string username,
                                    std::string password) = 0;
  virtual Retval read_users_data() = 0;
  virtual int get_user_id(std::string user_name) = 0;
  virtual void add_new_user(User &new_user) = 0;
  virtual Retval update_db() = 0;
  virtual Retval is_user_exist(int user_id) = 0;
  virtual Retval is_anonymous_questions_allowed(int user_id,
                                                bool &is_allowed) = 0;
  virtual Retval print_users(int current_user_id) = 0;
  virtual ~usersDb() = default;

protected:
  std::unordered_map<int, User> id_to_users;
  std::unordered_map<std::string, User *> username_to_users;
  int max_id{0};
};

class usersDbCsv : public usersDb {
public:
  usersDbCsv();
  Retval verify_credentials(std::string username,
                            std::string password) override;
  Retval read_users_data() override;
  int get_user_id(std::string user_name) override;
  void add_new_user(User &new_user) override;
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
#endif /*__USERSDB_HPP__*/