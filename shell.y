
/*
 * CS-252
 * shell.y: parser for shell
 *
 * This parser compiles the following grammar:
 *
 *	cmd [arg]* [> filename]
 *
 * you must extend it to understand the complete shell grammar
 *
 */

%code requires 
{
#include <string>
#include <regex.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <algorithm> 
#include <vector>

#if __cplusplus > 199711L
#define register      // Deprecated in C++11 so remove the keyword
#endif
}

%union
{
  char        *string_val;
  // Example of using a c++ type in yacc
  std::string *cpp_string;
}

%token <cpp_string> WORD
%token NOTOKEN GREAT NEWLINE GREATGREAT PIPE AMPERSAND LESS LESSLESS GREATAMPERSAND GREATGREATAMPERSAND TWOGREAT
%error-verbose

%{
//#define yylex yylex
#include <cstdio>
#include "shell.hh"
void expandWildcardsifNecessary(std::string * argument);
void expandWildcards(char * prefix, char * suffix,  std::vector<std::string> &argList);
bool compare(std::string x, std::string y);
void yyerror(const char * s);
int yylex();
bool f = false;
int check = 0;

%}

%%

goal:
  commands
  ;

commands:
  command
  | commands command
  ;

command: simple_command
       ;

simple_command:	
  pipe_list io_modifier_list background_optional NEWLINE {
    //printf("   Yacc: Execute command\n");
    Shell::_currentCommand.execute();
  }
  | NEWLINE 
  | error NEWLINE { yyerrok; }
  ;

pipe_list:
  pipe_list PIPE command_and_args
  | command_and_args
  ;

command_and_args:
  command_word argument_list {
    Shell::_currentCommand.insertSimpleCommand( Command::_currentSimpleCommand );
  }
  ;

argument_list:
  argument_list argument
  | /* can be empty */
  ;

argument:
  WORD {
    //printf("   Yacc: insert argument \"%s\"\n", $1->c_str());
    if (strcmp($1->c_str(), "${?}") == 0) {
      //printf("check\n");
      Command::_currentSimpleCommand->insertArgument( $1 );

    } else {
    
  
    expandWildcardsifNecessary($1);
    }
  }
  ;

command_word:
  WORD {
    //printf("   Yacc: insert command \"%s\"\n", $1->c_str());
    Command::_currentSimpleCommand = new SimpleCommand();
    Command::_currentSimpleCommand->insertArgument( $1 );
    
  }
  ;
  
io_modifier_list:
  io_modifier_list iomodifier_opt
  | /*empty*/
  ;

background_optional:
  AMPERSAND {Shell::_currentCommand._background = true;
  //std::cout << Shell::_currentCommand._background;
  }
  
  | /*empty*/
  ;

iomodifier_opt:
  GREAT WORD {
    //printf("   Yacc: insert output \"%s\"\n", $2->c_str());
    if (Shell::_currentCommand._outFile != NULL) {
      Shell::_currentCommand._ambiguity = true;
    }
    Shell::_currentCommand._outFile = $2;
    
  }
  | GREATGREAT WORD {
    if (Shell::_currentCommand._outFile != NULL) {
      Shell::_currentCommand._ambiguity = true;
    }
    Shell::_currentCommand._outFile = $2;
    Shell::_currentCommand._append = 1;
    
  }

  | GREATGREATAMPERSAND WORD {
    if (Shell::_currentCommand._outFile != NULL) {
      Shell::_currentCommand._ambiguity = true;
    }
     Shell::_currentCommand._outFile = $2;
     Shell::_currentCommand._errFile = $2;
     Shell::_currentCommand._append = 1;
  }
  | GREATAMPERSAND WORD {
    if (Shell::_currentCommand._outFile != NULL) {
      Shell::_currentCommand._ambiguity = true;
    }
    if (Shell::_currentCommand._errFile != NULL) {
      Shell::_currentCommand._ambiguity = true;
    }
     Shell::_currentCommand._outFile = $2;
     Shell::_currentCommand._errFile = $2;
  }
  | LESS WORD {
    if (Shell::_currentCommand._inFile != NULL) {
      Shell::_currentCommand._ambiguity = true;
    }
    Shell::_currentCommand._inFile = $2;
  }
  | TWOGREAT WORD {
    if (Shell::_currentCommand._errFile != NULL) {
      Shell::_currentCommand._ambiguity = true;
    }
     Shell::_currentCommand._errFile = $2;
  }
 

  ;

%%

void
yyerror(const char * s)
{
  fprintf(stderr,"%s", s);
}

bool compare(std::string x, std::string y) {
  return x.compare(y)<0;
}
void expandWildcardsifNecessary(std::string * argument) {
  char * arg = strdup(argument->c_str());
  //bool f = false;
 
  //printf("arg: %s\n", arg);
  char * star = strchr(arg, '*');
  char * question = strchr(arg, '?');
  if (star == NULL && question == NULL) {
    //printf("%s\n", argument->c_str());
    Command::_currentSimpleCommand->insertArgument(argument);
    return;
  }

  std::vector<std::string> argList;
  char * empty = (char *)"";
  if (arg[0] == '/') {
    expandWildcards((char *)"/", (char *)(arg + 1), argList);
    if (!f) {
    Command::_currentSimpleCommand->insertArgument(argument);
    }
  } else {
    expandWildcards(empty, (char*) arg, argList);
    
  }
  std::sort(argList.begin(), argList.end(), compare);
  
  for (auto arg1: argList) {
    std::string * arg2 = new std::string(arg1);
    
    Command::_currentSimpleCommand->insertArgument(arg2);
  }
  free(arg);
}

void expandWildcards(char * prefix, char * suffix,  std::vector<std::string> &argList) {
  
  if (suffix[0] == '\0') { // suffix is empty, put prefix in argList
    argList.push_back(std::string(prefix));
    return;
  }
 
  
  char * newDir = (char *)malloc(1024 * sizeof(char));
  if (prefix[0] == 0) {
    if (suffix[0] == '/') {
      suffix = suffix + 1;
      sprintf(newDir, "/");
    }
  } else {
      if (prefix[strlen(prefix) - 1] !=  '/') {
        sprintf(newDir, "%s/", prefix);
      } else {
        sprintf(newDir, "%s", prefix);
      }
  }
  char * newDirDup = strdup(newDir);

  

  char * buffer = (char *)malloc(1024 * sizeof(char));
  //get the next part of the suffix
  
  char * s = strchr(suffix, '/');
  
  //std::string suffixcc = new std::string(suffix);
  //int slashPos = suffixcc.find('/');
  if (s != NULL) { // copy up to first '/'
    strncpy(buffer, suffix, s + 1 - suffix);
    buffer[s + 1-suffix] = 0;
    suffix = s + 1;
  } else { // end of line
    strcpy(buffer, suffix);
    suffix = suffix + strlen(suffix);
  }
  
  // now we need to update the prefix 
  int prefixLength = strlen(prefix);
  char * updatedPrefix = (char*)malloc(1024 * sizeof(char));
  char * bufferStar = strchr(buffer, '*');
  char * bufferQuestion = strchr(buffer, '?');
  // if there is no * or ? add prefix and buffer to the new prefix and expand the wildcards again
  if (bufferStar == NULL && bufferQuestion == NULL) {
    sprintf(updatedPrefix, "%s%s", newDirDup, buffer);
    expandWildcards(updatedPrefix, suffix, argList);
    return;
  }


  // convert wildcard to regex 
  char * bufTemp = strdup(buffer);
  if (strchr(buffer, '/') != NULL) {
    bufTemp[strlen(bufTemp) - 1] = '\0';
  }
  char * reg = (char*)malloc(2*strlen(bufTemp) + 10);
  char * a = bufTemp;
  
  //printf("%s\n",a);
  char * r = reg;
  *r = '^'; r++;
  while (*a) {
    if (*a == '*') { *r = '.'; r++; *r='*'; r++;}
    else if (*a == '?') { *r = '.'; r++;}
    else if (*a == '.') {*r = '\\'; r++; *r='.';r++;}
    else {*r=*a; r++;}
    a++;
  }
  *r='$'; r++; *r=0;
  //printf("%s\n",reg);
  
  regex_t preg;
  regmatch_t match;
  int expbuf = regcomp(&preg, reg, REG_EXTENDED|REG_NOSUB);
  /*if (expbuf==NULL) {
    perror("compile");
    return;
  }
  */
  char * d;
  
  if (prefixLength == 0) {
    d = ".";
  } else {
    d = newDir;
  }
  DIR * dir = opendir(d);

  if (dir == NULL) {
    return;
  }
 

  struct dirent * ent;
  while ((ent = readdir(dir))!= NULL) {
    char * dirName = ent->d_name;
    if (regexec(&preg, dirName, 1, &match, 0) == 0) {
      f = true;
      check = 1;
      //if (prefix[strlen(prefix) - 1] !=  '/') {
        //sprintf(updatedPrefix, "%s/%s", prefix, dirName);
      //} else {
         sprintf(updatedPrefix, "%s%s", newDirDup, dirName);
        
      //}
      //expandWildcards(updatedPrefix, suffix, argList);  
      
      if (reg[1] == '.') {
        if (dirName[0] != '.') {
          expandWildcards(updatedPrefix, suffix, argList);
        }
      } else {
        expandWildcards(updatedPrefix, suffix, argList);
      } 
    } 
  }
  closedir(dir);
  free(reg);
  



}

#if 0
main()
{
  yyparse();
}
#endif
