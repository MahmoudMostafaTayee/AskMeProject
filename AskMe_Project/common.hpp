
#ifndef __COMMON_HPP__
#define __COMMON_HPP__
inline constexpr char NOT_ANSWERED_QUESTION[] = "Question Not Answered yet!";

enum class Retval {
  SUCCESS = 0,
  INVALID_STATE,
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
  Utils();

public:
  static Retval GetYOrNFromUser(bool &response);
};
#endif /*__COMMON_HPP__*/
