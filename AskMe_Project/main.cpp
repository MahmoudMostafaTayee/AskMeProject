#include "common.hpp"
#include "questionsDb.hpp"
#include "usersDb.hpp"
#include <cassert>
#include <iostream>

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
