#ifndef shell_hh
#define shell_hh

#include "command.hh"

struct Shell {

  static void prompt();
  static bool flag;
  static void setFlag(bool f);
  static Command _currentCommand;
};

#endif
