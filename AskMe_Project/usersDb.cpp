#include "usersDb.hpp"
#include <algorithm>
#include <fstream>
#include <sstream>

std::ostream &operator<<(std::ostream &os, User &user) {
  os << "Name: " << user.name << std::endl;
  os << "Email: " << user.email << std::endl;
  os << "Id: " << user.id << std::endl;
  return os;
}

usersDb::usersDb() { read_users_data(); }

Retval usersDb::verify_credentials(std::string username, std::string password) {
  Retval retval = Retval::DB_USERS_ERROR_WRONG_CREDENTIALS;
  if (username_to_users.find(username) != username_to_users.end()) {
    if (username_to_users[username]->password == password) {
      retval = Retval::SUCCESS;
    }
  } else {
    retval = Retval::DB_USERS_ERROR_USER_CREDENTIALS_NOT_FOUND;
  }
  return retval;
}

Retval usersDb::read_users_data() {
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
  } else {
    retval = Retval::DB_ERROR_READING_DB;
  }

  return retval;
}

int usersDb::get_user_id(std::string user_name) {
  int user_id = -1;

  if (username_to_users.find(user_name) != username_to_users.end()) {
    user_id = username_to_users[user_name]->id;
  }

  return user_id;
}

void usersDb::add_new_user(User &new_user) {
  max_id++;
  id_to_users[max_id] = new_user;
  username_to_users[new_user.user_name] = &id_to_users[max_id];
}

Retval usersDb::update_db() {
  Retval retval{Retval::SUCCESS};
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
  return retval;
}

Retval usersDb::is_user_exist(int user_id) {
  Retval retval{Retval::DB_USERS_ERROR_USER_ID_NOT_FOUND};
  if (id_to_users.find(user_id) != id_to_users.end()) {
    retval = Retval::SUCCESS;
  }
  return retval;
}

Retval usersDb::is_anonymous_questions_allowed(int user_id, bool &is_allowed) {
  Retval retval{Retval::DB_USERS_ERROR_USER_ID_NOT_FOUND};
  retval = is_user_exist(user_id);
  if (Retval::SUCCESS == retval) {
    retval = Retval::SUCCESS;
    is_allowed = id_to_users[user_id].isAnonymousAllowed;
  }
  return retval;
}

Retval usersDb::print_users(int current_user_id) {
  Retval retval{Retval::SUCCESS};
  for (auto id_to_user : id_to_users) {
    if (current_user_id != id_to_user.first) {
      std::cout << id_to_user.second << std::endl;
    }
  }
  return retval;
}

usersDb::~usersDb() { update_db(); }
