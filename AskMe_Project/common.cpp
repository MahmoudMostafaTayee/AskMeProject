#include "common.hpp"
#include <iostream>

Utils::Utils() {}

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
