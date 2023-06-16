#include <cstdio>

#include "shell.hh"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <regex.h>
#include <pwd.h>

bool Shell::flag = true;
bool c = false;
int yyparse(void);

void Shell::prompt() {
  if (flag && (isatty(0))) {
    printf("myshell>");
    fflush(stdout);
  } else {
    flag == true;
  }
 
}
void Shell::setFlag(bool f) {
  Shell::flag = f;
}
extern "C" void ctrlC(int signal) {
  //printf("\n");
  //Shell::prompt();

}

extern "C" void killzombie(int singal) {
  int pid = wait3(0,0,NULL);
  while(waitpid(-1, NULL, WNOHANG) > 0); 
  //Shell::prompt();
}



int main() {
  // control c handling
  struct sigaction signalAction;
  signalAction.sa_handler = ctrlC;
  sigemptyset(&signalAction.sa_mask);
  signalAction.sa_flags = SA_RESTART;
 
  int error = sigaction(SIGINT, &signalAction, NULL);
  if (error) {
    perror("sigaction");
    exit(-1);
  }

  struct sigaction signalAction2;
  signalAction2.sa_handler = killzombie;
  sigemptyset(&signalAction2.sa_mask);
  signalAction2.sa_flags = SA_RESTART;

  int err = sigaction(SIGCHLD, &signalAction2, NULL);
  if (err) {
    perror("sigaction");
    exit(-1);
  }   
 
  Shell::prompt();
  yyparse();
  
}

Command Shell::_currentCommand;

