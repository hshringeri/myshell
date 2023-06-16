
/*
 *
 * CS-252 
 * shell.l: lexical analyzer for shell
 * You have to extend it.
 *
 */

%{

#include <cstring>
#include "y.tab.hh"
#include "shell.hh"
#include <iostream>
#include <string>
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
#include "command.hh"


using namespace std;
////////////  Start added code ///////////


extern "C" char * read_line();

int mygetc(FILE * f) {
  static char *p;
  char ch;

     if (!isatty(0)) {
  // stdin is not a tty. Call real getc
    return getc(f);
  }

  // stdin is a tty. Call our read_line.
  if (p==NULL || *p == 0) {
    char * s = read_line();
    p = s;
  }

  ch = *p;
  p++;

  return ch;
}

#undef getc
#define getc(f) mygetc(f)

/////////// End added code ///////////


static  void yyunput (int c,char *buf_ptr  );

void myunputc(int c) {
  unput(c);
}

%}

%option noyywrap

%%

"source ".* {
  char * filename = strdup(&yytext[7]);
  FILE * file = fopen(filename, "r");
  if (file == NULL) {
    perror("source file error");
  }
  Shell::setFlag(false);
  yypush_buffer_state(yy_create_buffer(file, YY_BUF_SIZE));
  yyparse();
  Shell::setFlag(true);
  yypop_buffer_state();
  Shell::prompt();
  
}

\$\([^\n\$]*\) {
  string command = strdup(yytext);
  command.erase(0,1);
  command.erase(0,1);
  command.erase(command.length()-1,1);
  command[command.length()] = '\0';
  
  char copy[command.length() + 1];
  for (int i = 0; i < command.length(); i++) {
    copy[i] = command[i];
  }
  copy[command.length()] = '\0';
  int ret;
  int tempIn = dup(0);
  int tempOut = dup(1);
  int pin[2], pout[2];
  pipe(pin); 
  pipe(pout);

  write(pin[1], copy, strlen(copy));
  write(pin[1], "\n", 1);
  write(pin[1], "exit", 4);
  write(pin[1], "\n", 1);
  close(pin[1]);
  dup2(pin[0],0);
  close(pin[0]);
  dup2(pout[1],1);
  close(pout[1]);
  ret = fork();
  
  if (ret == 0) {
    execvp("/proc/self/exe", NULL);
   
    exit(1);
  } else if (ret < 0) {
    perror("fork");
    exit(1);
  }
 
  dup2(tempIn,0);
  dup2(tempOut,1);
  close(tempIn);
  close(tempOut);

  char chr;
  char * buffer = (char *)malloc(4096);
  int count  = 0;
  while (read(pout[0], &chr, 1)) {
    if (chr == '\n') {
      buffer[count++] = ' ';
      continue;
    } else {
      buffer[count++] = chr;
    }
  }
  buffer[count] = '\0';
  //dup2(pout[0], 1);
  close(pout[0]);

  count = strlen(buffer) - 1;

  while (count >=0) {
    myunputc(buffer[count]);
    count--;
  }
  //free(buffer);
  buffer = NULL;
 





}




\n {
  return NEWLINE;
}
[ \t] {
  /* Discard spaces and tabs */
}
">" {
  return GREAT;
}
"<" { 
  return LESS;
}
">>" {
  return GREATGREAT;
}
"|" {
  return PIPE;
}
"&" {
  return AMPERSAND;
}
">&" {
  return GREATAMPERSAND;
}
">>&" {
  return GREATGREATAMPERSAND;
}
"2>" {
  return TWOGREAT; 
}


\"[^\n\"]*\" {
   /* Case that word is quoted */
  char * dupStr = strdup(yytext);
  dupStr++;
  dupStr[strlen(dupStr)-1] = '\0';
  yylval.cpp_string = new std::string(dupStr);
  return WORD;

 }

[^ \t\n|><&]*\\[^\t\n]* {
  char * yyDup = (char*)malloc(strlen(yytext) + 1);
  int flag = 0;
  int dupIndex = 0;
  int length = strlen(yytext);
  

  for (int i = 0; i < length; i++) {
    if (flag == 0 && yytext[i] == '\\') {
      flag = 1;
    } else if (flag == 1) {
      yyDup[dupIndex] = yytext[i];
      flag = 0;
      dupIndex++;
    } else {
      yyDup[dupIndex] = yytext[i];
      dupIndex++;
    }
  }

  yyDup[dupIndex] = '\0';
  yylval.cpp_string = new std::string(yyDup);
  return WORD;
 
}
[^ \|>\t\n][^ \|>\t\n]* {
  /* Assume that file names have only alpha chars */
  yylval.cpp_string = new std::string(yytext);
  return WORD;
}

