#include "questionsDb.hpp"
#include <algorithm>
#include <fstream>
#include <sstream>

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

questionBankDb::questionBankDb(usersDb &users_db) : users_db(users_db) {
  read_questions_db();
}

Retval questionBankDb::read_questions_db() {
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

Retval questionBankDb::print_from_questions(int user_id) {
  Retval retval{Retval::SUCCESS};
  if (users_from_questions_map.find(user_id) !=
      users_from_questions_map.end()) {
    for (auto q : users_from_questions_map[user_id]) {
      std::cout << q << std::endl;
    }
  }
  return retval;
}

Retval questionBankDb::print_to_questions(int user_id) {
  Retval retval{Retval::INVALID_STATE};
  if (users_to_questions_map.find(user_id) != users_to_questions_map.end()) {
    for (auto question : users_to_questions_map[user_id]) {
      std::cout << *question << std::endl;
    }
  }

  return retval;
}

Retval questionBankDb::get_question_answer(int current_user_id, int question_id,
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

Retval questionBankDb::update_answer(int current_user_id, int question_id,
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

Retval questionBankDb::delete_question(int current_user_id, int question_id) {
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

Retval questionBankDb::update_db() {
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

Retval questionBankDb::add_question(Question &question) {
  Retval retval = Retval::SUCCESS;
  users_from_questions_map[question.from].push_back(std::move(question));
  users_to_questions_map[question.to].push_back(
      &users_from_questions_map[question.from].back());
  return retval;
}

questionBankDb::~questionBankDb() { update_db(); }
