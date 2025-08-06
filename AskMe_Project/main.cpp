#include <algorithm>
#include <cassert>
#include <deque>
#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <vector>

#define NOT_ANSWERED_QUESTION "Question Not Answered yet!"
enum class Retval {
  SUCCESS = 0,
  INVLAID_STATE,
  EXIT_SIGNAL,
  LOGOUT_NEEDED,

  INVALID_INPUT,

  MENU_INVALID_OPTION = 100,
  MENU_LOGIC_PASSWORD_MISMATCH,

  DB_USERS_ERROR_WRONG_CREDENTIALS = 200,
  DB_USERS_ERROR_USER_CREDENTIALS_NOT_FOUND,
  DB_USERS_ERROR_USER_ID_NOT_FOUND,
  DB_ERROR_READING_DB,
  DB_ERROR_NONEXISTENT_ID,

};

class Utils {
private:
  Utils() {}

public:
  static Retval GetYOrNFromUser(bool &response);
};
Retval Utils::GetYOrNFromUser(bool &response) {
  Retval retval{Retval::SUCCESS};
  std::string input{};
  std::getline(std::cin, input);
  if (input.length() == 1) {
    switch (input[0]) {
    case 'y':
    case 'Y':
      response = true;
      break;
    case 'n':
    case 'N':
      response = false;
      break;
    default:
      retval = Retval::INVALID_INPUT;
      break;
    }
  } else {
    retval = Retval::INVALID_INPUT;
  }
  return retval;
}
class Question;
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
std::ostream &operator<<(std::ostream &os, User &user) {
  os << "Name: " << user.name << std::endl;
  os << "Email: " << user.email << std::endl;
  os << "Id: " << user.id << std::endl;
  return os;
}

class Question {
public:
  static int next_id;
  int id;
  int unknown;
  int from;
  int to;
  bool isAnonymous;
  std::string question;
  std::string answer{NOT_ANSWERED_QUESTION};

  friend std::ostream &operator<<(std::ostream &os, const Question &question);
};
int Question::next_id{1};

std::ostream &operator<<(std::ostream &os, const Question &question) {
  os << "Questin ID: " << question.id << std::endl;
  os << "Question: " << question.question << std::endl;
  os << "Answer: " << question.answer << std::endl;
  if (question.isAnonymous)
    os << "Question directed to: " << "Anonymous user" << std::endl;
  else
    os << "Question directed to: " << question.to << std::endl;
  return os;
}
class usersDb {
public:
  usersDb() { read_users_data(); }

  Retval verify_credentials(std::string username, std::string password) {
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

  Retval read_users_data() {
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

  int get_user_id(std::string user_name) {
    int user_id = -1;

    if (username_to_users.find(user_name) != username_to_users.end()) {
      user_id = username_to_users[user_name]->id;
    }

    return user_id;
  }

  void add_new_user(User &new_user) {
    max_id++;
    id_to_users[max_id] = new_user;
    username_to_users[new_user.user_name] = &id_to_users[max_id];
  }

  Retval update_db() {
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

  Retval is_user_exist(int user_id) {
    Retval retval{Retval::DB_USERS_ERROR_USER_ID_NOT_FOUND};
    if (id_to_users.find(user_id) != id_to_users.end()) {
      retval = Retval::SUCCESS;
    }
    return retval;
  }

  Retval is_anonymous_questions_allowed(int user_id, bool &is_allowed) {
    Retval retval{Retval::DB_USERS_ERROR_USER_ID_NOT_FOUND};
    retval = is_user_exist(user_id);
    if (Retval::SUCCESS == retval) {
      retval = Retval::SUCCESS;
      is_allowed = id_to_users[user_id].isAnonymousAllowed;
    }
    return retval;
  }

  Retval print_users(int current_user_id) {
    Retval retval{Retval::SUCCESS};
    for (auto id_to_user : id_to_users) {
      if (current_user_id != id_to_user.first) {
        std::cout << id_to_user.second << std::endl;
      }
    }
    return retval;
  }

  ~usersDb() { update_db(); }

private:
  std::unordered_map<int, User> id_to_users{};
  std::unordered_map<std::string, User *> username_to_users{};
  int max_id{0};
};

class questionBankDb {
public:
  questionBankDb(usersDb &users_db) : users_db(users_db) {
    read_questions_db();
  }

  Retval read_questions_db() {
    Retval retval{Retval::SUCCESS};
    std::ifstream fin{"questions.csv"};
    std::string line;
    while (std::getline(fin, line)) {
      if (line.empty() || std::all_of(line.begin(), line.end(), [](char c) {
            return std::isspace(static_cast<unsigned char>(c));
          })) {
        continue;
      }

      std::istringstream line_stream{line};
      std::string field{};
      Question question{};

      std::getline(line_stream, field, ',');
      question.id = std::stoi(field);
      if (question.id > Question::next_id) {
        Question::next_id = question.id + 1;
      }
      std::getline(line_stream, field, ',');
      question.unknown = std::stoi(field);
      std::getline(line_stream, field, ',');
      question.from = std::stoi(field);
      std::getline(line_stream, field, ',');
      question.to = std::stoi(field);
      std::getline(line_stream, field, ',');
      question.isAnonymous = (field == "1");
      std::getline(line_stream, question.question, ',');
      if (std::getline(line_stream, field, ',')) {
        question.answer = field;
      }

      users_from_questions_map[question.from].push_back(std::move(question));
      users_to_questions_map[question.to].push_back(
          &users_from_questions_map[question.from].back());
    }

    fin.close();
    return retval;
  }

  Retval print_from_questions(int user_id) {
    Retval retval{Retval::SUCCESS};
    if (users_from_questions_map.find(user_id) !=
        users_from_questions_map.end()) {
      for (auto q : users_from_questions_map[user_id]) {
        std::cout << q << std::endl;
      }
    }
    return retval;
  }

  Retval print_to_questions(int user_id) {
    Retval retval{Retval::INVLAID_STATE};
    if (users_to_questions_map.find(user_id) != users_to_questions_map.end()) {
      for (auto question : users_to_questions_map[user_id]) {
        std::cout << *question << std::endl;
      }
    }

    return retval;
  }

  Retval get_question_answer(int current_user_id, int question_id,
                             std::string &answer) {
    Retval retval = Retval::DB_ERROR_NONEXISTENT_ID;
    if (users_to_questions_map.find(current_user_id) !=
        users_to_questions_map.end()) {
      for (auto question : users_to_questions_map[current_user_id]) {
        if (question->id == question_id) {
          answer = question->answer;
          retval = Retval::SUCCESS;
        }
      }
    }
    return retval;
  }

  Retval update_answer(int current_user_id, int question_id,
                       std::string new_answer) {
    Retval retval = Retval::SUCCESS;
    if (users_to_questions_map.find(current_user_id) !=
        users_to_questions_map.end()) {
      for (auto question : users_to_questions_map[current_user_id]) {
        if (question_id == question->id) {
          question->answer = new_answer;
        }
      }
    }
    return retval;
  }

  Retval delete_question(int current_user_id, int question_id) {
    Retval retval = Retval::SUCCESS;
    bool deleted = false;
    int to_id{-1};
    int from_index_to_delete{-1};
    for (const Question &question : users_from_questions_map[current_user_id]) {
      from_index_to_delete++;
      if (question.id == question_id) {
        deleted = true;
        to_id = question.to;
        break;
      }
    }

    if (!deleted) {
      retval = Retval::DB_ERROR_NONEXISTENT_ID;
    } else {
      int to_index_to_delete = -1;
      deleted = false;
      for (const Question *question : users_to_questions_map[to_id]) {
        to_index_to_delete++;
        if (question->id == question_id) {
          deleted = true;
          users_to_questions_map[to_id].erase(
              users_to_questions_map[to_id].begin() + to_index_to_delete);
          break;
        }
      }
    }

    if (!deleted) {
      retval = Retval::DB_ERROR_NONEXISTENT_ID;
    } else {
      users_from_questions_map[current_user_id].erase(
          users_from_questions_map[current_user_id].begin() +
          from_index_to_delete);
    }

    return retval;
  }

  Retval update_db() {
    Retval retval{Retval::SUCCESS};
    std::ofstream fout("questions.csv");

    for (auto user_questions_pair : users_from_questions_map) {
      for (auto question : user_questions_pair.second) {
        fout << question.id << ',';
        fout << question.unknown << ',';
        fout << question.from << ',';
        fout << question.to << ',';
        fout << (question.isAnonymous ? 1 : 0) << ',';
        fout << question.question;
        if (question.answer != NOT_ANSWERED_QUESTION) {
          fout << ',';
          fout << question.answer;
        }
        fout << '\n';
      }
    }
    fout.close();
    return retval;
  }

  Retval add_question(Question &question) {
    Retval retval = Retval::SUCCESS;
    users_from_questions_map[question.from].push_back(std::move(question));
    users_to_questions_map[question.to].push_back(
        &users_from_questions_map[question.from].back());
    return retval;
  }

  ~questionBankDb() { update_db(); }

private:
  std::unordered_map<int, std::deque<Question>> users_from_questions_map;
  std::unordered_map<int, std::vector<Question *>> users_to_questions_map;
  usersDb &users_db;
};

class Menu {
public:
  Menu(usersDb &users_db, questionBankDb &question_bank)
      : users_db(users_db), question_bank(question_bank) {}
  void run_main_menu() {
    Retval retval{Retval::MENU_INVALID_OPTION};
    do {
      std::string option{};
      std::string username;
      std::string password;
      std::string confirm_password;
      std::cout << login_signup_main_menu;
      std::getline(std::cin, option);

      if (option.length() == 1) {
        switch (option[0]) {
        case '1':
          retval = login_handler();
          break;
        case '2':
          signup_handler();
          break;
        default:
          std::cout << "Wrong choice, try again!";
          retval = Retval::MENU_INVALID_OPTION;
          break;
        }
      }
    } while (retval != Retval::SUCCESS && retval != Retval::EXIT_SIGNAL);
    if (retval == Retval::EXIT_SIGNAL) {
      std::cout << "We will miss you, take care :)" << std::endl;
    }
  }

  Retval login_handler() {
    Retval retval{Retval::SUCCESS};
    std::string username;
    std::string password;

    std::cout << "Enter username: ";
    std::getline(std::cin, username);
    std::cout << "Password: ";
    std::getline(std::cin, password);
    retval = users_db.verify_credentials(username, password);
    if (Retval::DB_USERS_ERROR_WRONG_CREDENTIALS == retval) {
      std::cout << "Wrong password\n";
      do {
        std::cout << "Password: ";
        std::getline(std::cin, password);
        retval = users_db.verify_credentials(username, password);
      } while (Retval::SUCCESS != retval);
    } else if (Retval::DB_USERS_ERROR_USER_CREDENTIALS_NOT_FOUND == retval) {
      std::cout << "Invalide credentials!" << std::endl;
    } else if (Retval::SUCCESS == retval) {
      current_user_id = users_db.get_user_id(username);
      std::cout << "current_user_id: " << current_user_id << std::endl;
      retval = internal_menu_handler();
    }

    return retval;
  }

  void signup_handler() {
    User user{};
    std::cout << "Enter Email: ";
    std::getline(std::cin, user.email);
    std::cout << "Enter your name: ";
    std::getline(std::cin, user.name);
    std::cout << "Enter password: ";
    std::getline(std::cin, user.password);
    std::cout << "Enter user name: ";
    std::getline(std::cin, user.user_name);
    users_db.add_new_user(user);
  }

  Retval password_checker(std::string password, std::string confirm_password) {
    Retval retval = Retval::MENU_LOGIC_PASSWORD_MISMATCH;
    if (password == confirm_password) {
      retval = Retval::SUCCESS;
    }
    return retval;
  }

  Retval internal_menu_handler() {
    Retval retval{Retval::SUCCESS};

    do {
      retval = Retval::SUCCESS;
      std::string option{};
      std::cout << internal_menu_options;
      std::getline(std::cin, option);

      if (option.length() == 1) {
        switch (option[0]) {
        case '1':
          question_bank.print_to_questions(current_user_id);
          press_enter_to_continue();
          break;
        case '2':
          question_bank.print_from_questions(current_user_id);
          press_enter_to_continue();
          break;
        case '3':
          answer_question();
          press_enter_to_continue();
          break;
        case '4':
          retval = delete_question();
          if (retval != Retval::SUCCESS) {
            std::cout << "Invalid question Id to delete!" << std::endl;
          }
          press_enter_to_continue();
          break;
        case '5':
          retval = add_question();
          if (retval != Retval::SUCCESS) {
            std::cout << "Error happened while adding your question!"
                      << std::endl;
          }
          press_enter_to_continue();
          break;
        case '6':
          users_db.print_users(current_user_id);
          press_enter_to_continue();
          break;
        case '7':
          std::cout << "This feature is not available yet!" << std::endl;
          press_enter_to_continue();
          break;
        case '8':
          std::cout << "Logging out ..." << std::endl;
          retval = Retval::LOGOUT_NEEDED;
          break;
        case '9':
          retval = Retval::EXIT_SIGNAL;
          break;
        default:
          std::cout << "Wrong choice, try again!" << std::endl;
          retval = Retval::MENU_INVALID_OPTION;
          break;
        }
      }
    } while (retval != Retval::LOGOUT_NEEDED && retval != Retval::EXIT_SIGNAL);
    return retval;
  }

  Retval add_question() {
    Retval retval = Retval::SUCCESS;
    std::string field{};
    Question question{};

    std::cout << "Enter the User ID that you want to ask for: ";
    std::getline(std::cin, field);
    question.to = std::stoi(field);
    retval = users_db.is_user_exist(question.to);
    if (Retval::SUCCESS == retval) {
      bool user_wants_anonymous{false};
      bool isAnonymousAllowed{false};
      do {

        std::cout << "Do you want to ask this question anonymously(y/n)?"
                  << std::endl;

        retval = Utils::GetYOrNFromUser(user_wants_anonymous);
      } while (retval != Retval::SUCCESS);
      users_db.is_anonymous_questions_allowed(question.to, isAnonymousAllowed);
      if ((!user_wants_anonymous) || isAnonymousAllowed) {
        std::cout << "Please enter the question: ";
        std::getline(std::cin, question.question);
        question.answer = NOT_ANSWERED_QUESTION;
        question.id = Question::next_id++;
        question.isAnonymous = user_wants_anonymous;
        question.unknown = -1;
        question.from = current_user_id;

        question_bank.add_question(question);
      } else {
        std::cout << user_wants_anonymous << isAnonymousAllowed << std::endl;
        std::cout << "Sorry this user doesn't allow anonymous questions!"
                  << std::endl;
      }
    } else {
      std::cout
          << "The user ID that you entered doesn't match any of the users."
          << std::endl;
    }

    return retval;
  }

  Retval delete_question() {
    Retval retval = Retval::SUCCESS;
    std::string question_id_str{};
    std::cout << "Please enter the ID of the question you want to delete: ";
    std::getline(std::cin, question_id_str);

    int question_id = std::stoi(question_id_str);
    retval = question_bank.delete_question(current_user_id, question_id);
    return retval;
  }

  void answer_question() {
    Retval retval = Retval::SUCCESS;
    std::string answer{NOT_ANSWERED_QUESTION};
    std::string question_id_str{};
    int question_id{-1};
    do {
      std::cout << "Please enter the ID of the question you want to answer: ";
      std::getline(std::cin, question_id_str);

      question_id = std::stoi(question_id_str);
      retval = question_bank.get_question_answer(current_user_id, question_id,
                                                 answer);
      if (retval == Retval::DB_ERROR_NONEXISTENT_ID) {
        std::cout << "The id that you entered is invalid!" << std::endl;
        std::cout << "Do you want to enter another ID?(Yes(y)/No(n)).\n";
        std::string option{};
        std::getline(std::cin, option);
        if (option.length() == 1) {
          if (option[0] == 'y' || option[0] == 'Y') {
          } else {
            if (option[0] != 'n' && option[0] != 'N') {
              std::cout << "Wrong option; getting to previous menu!"
                        << std::endl;
            }
            break;
          }
        } else {
          std::cout << "Wrong option; getting to previous menu!" << std::endl;
          break;
        }
      }
    } while (retval != Retval::SUCCESS);
    if (retval == Retval::SUCCESS) {
      if (NOT_ANSWERED_QUESTION != answer) {
        std::cout << "The question is already answered before, that way your "
                     "previous "
                     "answer would be overwritten(Overwrite(y)/Skip(n)).\n";
        std::string option{};
        std::getline(std::cin, option);
        if (option.length() == 1) {
          switch (option[0]) {
          case 'y':
          case 'Y': {
            std::string answer{};
            std::cout << "Enter the new answer: ";
            std::getline(std::cin, answer);
            question_bank.update_answer(current_user_id, question_id, answer);
            std::cout << "Answer has been overwritten successfully!"
                      << std::endl;
          } break;
          case 'n':
          case 'N':
            std::cout << "Skipping ..." << std::endl;
            break;
          default:
            std::cout << "Wrong option chosen; Skipping ..." << std::endl;
            break;
          }
        } else {
          std::cout << "Wrong option chosen; Skipping ..." << std::endl;
        }
      } else {
        std::string answer{};
        std::cout << "Enter the answer: ";
        std::getline(std::cin, answer);
        question_bank.update_answer(current_user_id, question_id, answer);
        std::cout << "Answer has added successfully!" << std::endl;
      }
    } else {
      std::cout << "Answering Question Failed!" << std::endl;
    }
  }

  void press_enter_to_continue() {
    std::cout << "Press Enter to continue...";
    // std::cin.ignore(std::numeric_limits<std::streamsize>::max(),
    //                 '\n'); // clear input buffer
    std::cin.get(); // wait for Enter
  }

private:
  usersDb &users_db;
  questionBankDb &question_bank;
  int current_user_id{-1};
  std::string login_signup_main_menu{""
                                     "Menu:\n"
                                     "\t\t\t\t1: Login\n"
                                     "\t\t\t\t2: Sign Up\n"
                                     "Enter Number in range(1-2): "
                                     ""};

  std::string internal_menu_options{""
                                    "Menu:\n"
                                    "\t\t\t\t1: Print questions To Me.\n"
                                    "\t\t\t\t2: Print questions From Me.\n"
                                    "\t\t\t\t3: Answer questions.\n"
                                    "\t\t\t\t4: Delete question.\n"
                                    "\t\t\t\t5: Ask Question\n"
                                    "\t\t\t\t6: List system users\n"
                                    "\t\t\t\t7: Feed\n"
                                    "\t\t\t\t8: Logout\n"
                                    "\t\t\t\t9: Exit\n"
                                    "Enter number within range (1-9): "
                                    ""};
};

class Manager {
public:
  void operator()() {
    std::cout << "#############################################################"
                 "####\n";
    std::cout << "######################### AskMe Project "
                 "#########################\n";
    std::cout << "#############################################################"
                 "####\n";
    usersDb users_db{};
    questionBankDb question_bank{users_db};
    Menu menu{users_db, question_bank};
    menu.run_main_menu();
  }
};

int main() {
  Manager manager;
  manager();
  return 0;
}
