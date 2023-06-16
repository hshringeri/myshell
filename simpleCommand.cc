#include <cstdio>
#include <cstdlib>

#include <iostream>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <regex.h>
#include <pwd.h>
#include <unistd.h>

#include "simpleCommand.hh"

SimpleCommand::SimpleCommand() {
  _arguments = std::vector<std::string *>();
}

SimpleCommand::~SimpleCommand() {
  // iterate over all the arguments and delete them
  for (auto & arg : _arguments) {
    delete arg;
  }
}

std::string * SimpleCommand::tildeExpansion(std::string * argument) {
  std::string arg = *argument;
  if (arg[0] == '~') {
    if (arg.length() == 1) {
      argument->assign(getenv("HOME"));
      return argument;
    } else {
      if (arg[1] == '/') {
        std::string temparg = arg;
        temparg.erase(0,1);
        const char * tempargcc = temparg.c_str();
        argument->assign(strcat(strdup(getenv("HOME")), tempargcc));
        return argument;


      }
      char * username = (char *)malloc(100 * sizeof(char));
      std::string temp = arg;
      temp.erase(0,1);
      
      int i = 0;
      while (temp[i] != '/' && temp[i] != '\0') {
        username[i] = temp[i];
        i++;
      }
      username[i] = '\0';

      if (strlen(username) < strlen(temp.c_str())) {
        int pos = temp.find('/');
        std::string tempTwo = temp.substr(pos);
        argument->assign(strcat(getpwnam(username)->pw_dir, tempTwo.c_str()));
      } else {
        argument->assign(getpwnam(username)->pw_dir);
      }
      return argument;
      free(username);
    }
  }
  return NULL;
}

void SimpleCommand::insertArgument( std::string * argument ) {
  std::string * tilde = tildeExpansion(new std::string(*argument));
  if (tilde) {
    //printf("check\n");
    argument = tilde;
  }
 
  // simply add the argument to the vector
  _arguments.push_back(argument);
}

// Print out the simple command
void SimpleCommand::print() {
  for (auto & arg : _arguments) {
    std::cout << "\"" << *arg << "\" \t";
  }
  // effectively the same as printf("\n\n");
  std::cout << std::endl;
}
