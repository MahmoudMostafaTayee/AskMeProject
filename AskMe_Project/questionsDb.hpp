#ifndef __QUESTIONSDB_HPP__
#define __QUESTIONSDB_HPP__
#include "common.hpp"
#include "usersDb.hpp"
#include <deque>
#include <iostream>
#include <unordered_map>
#include <vector>

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

class questionBankDb {
public:
  questionBankDb(usersDb &users_db);
  Retval read_questions_db();
  Retval print_from_questions(int user_id);
  Retval print_to_questions(int user_id);
  Retval get_question_answer(int current_user_id, int question_id,
                             std::string &answer);
  Retval update_answer(int current_user_id, int question_id,
                       std::string new_answer);
  Retval delete_question(int current_user_id, int question_id);
  Retval update_db();
  Retval add_question(Question &question);
  ~questionBankDb();

private:
  std::unordered_map<int, std::deque<Question>> users_from_questions_map;
  std::unordered_map<int, std::vector<Question *>> users_to_questions_map;
  usersDb &users_db;
};
#endif /*__QUESTIONSDB_HPP__*/