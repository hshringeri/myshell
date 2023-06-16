#ifndef simplcommand_hh
#define simplecommand_hh

#include <string>
#include <vector>
#include <cstdlib>
#include <iostream>

struct SimpleCommand {

  // Simple command is simply a vector of strings
  std::vector<std::string *> _arguments;

  SimpleCommand();
  ~SimpleCommand();
  void insertArgument( std::string * argument );
  void print();
  std::string * tildeExpansion(std::string * argument);

};

#endif
