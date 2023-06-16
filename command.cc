/*
 * CS252: Shell project
 *
 * Template file.
 * You will need to add more code here to execute the command table.
 *
 * NOTE: You are responsible for fixing any bugs this code may have!
 *
 * DO NOT PUT THIS PROJECT IN A PUBLIC REPOSITORY LIKE GIT. IF YOU WANT 
 * TO MAKE IT PUBLICALLY AVAILABLE YOU NEED TO REMOVE ANY SKELETON CODE 
 * AND REWRITE YOUR PROJECT SO IT IMPLEMENTS FUNCTIONALITY DIFFERENT THAN
 * WHAT IS SPECIFIED IN THE HANDOUT. WE OFTEN REUSE PART OF THE PROJECTS FROM  
 * SEMESTER TO SEMESTER AND PUTTING YOUR CODE IN A PUBLIC REPOSITORY
 * MAY FACILITATE ACADEMIC DISHONESTY.
 */

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

#include "command.hh"
#include "shell.hh"


Command::Command() {
    // Initialize a new vector of Simple Commands
    _simpleCommands = std::vector<SimpleCommand *>();

    _outFile = NULL;
    _inFile = NULL;
    _errFile = NULL;
    _background = false;
    _append = 0;
    _ambiguity = false;
    
}

std::string * Command:: checkEnvExpansion(std::string * argument) {
    //printf("check\n");
    std::string command = *argument;
    const char * arg = argument->c_str();
    //printf("copied arg: %s\n", arg);
    const char * dollar = strchr(arg, '$');
    const char * openBrace = strchr(arg, '{');
    int dollarPos = command.find('$');
    //printf("%d\n", dollarPos);
    //char *copy = (char*)malloc((dollarPos + 1) * sizeof(char));
    //int dollarPos = command.find('$');
    
    if (dollar && openBrace) {
        char *copy = (char*)malloc((1024) * sizeof(char));
        //printf("check\n");
        for (int i = 0; i < dollarPos; i++) {
            copy[i] = command[i];
        }
        copy[dollarPos] = '\0';
        //printf("check\n");

        while (dollar) {
            //printf("while check\n");
            std::string dollarcc = std::string(dollar);
            dollarcc.erase(0,1);
            dollarcc.erase(0,1);
            //printf("dollar1: %s\n",dollar);
            int pos = dollarcc.find('}');
            //printf("%d\n", pos);
            char *env = (char*)malloc((1024) * sizeof(char));
            int i = 0;
            while (dollarcc[i] != '}') {
                env[i] = dollarcc[i];
                i++;
            }
            //printf("dollar2: %s\n",dollar);
            //printf("%s\n", env);
            if (strcmp(env, "$") == 0) {
                int pid = getpid();
                char * mypid = (char*)malloc(100 * sizeof(char));
                sprintf(mypid, "%d", pid);
                strcat(copy, mypid);
                free(mypid);
            } else if (strcmp(env,"?") == 0) {
                int laststatus = WEXITSTATUS(Command::lastStatus);
                char * myLaststatus = (char*)malloc(10 * sizeof(char));
                sprintf(myLaststatus, "%d", laststatus);
                strcat(copy, myLaststatus);
                free(myLaststatus);
            } else if (strcmp(env,"!") == 0) {
                int lastPID = Command::lastPid;
                char * myLastPid = (char*)malloc(10 * sizeof(char));
                // check if not child process
                if (lastPID != 0) {
                    sprintf(myLastPid, "%d", lastPID);
                } else {
                    sprintf(myLastPid,"");
                }
                strcat(copy, myLastPid);
                free(myLastPid);
            }else if (strcmp(env,"SHELL") == 0) {
                char * path = (char *)malloc(1024 * sizeof(char));
                realpath("/proc/self/exe", path);
                strcat(copy, path);
                free(path);
            
            } else if (strcmp(env, "_") == 0) {
                char *backPid = (char *)malloc(10 * sizeof(char));
                if (Command::lastArg != NULL) {
                    sprintf(backPid, "%s", Command::lastArg);
                } else {
                    sprintf(backPid,"");
                }
                strcat(copy, backPid);
                free(backPid);
            } 
            else {
                env = getenv(const_cast<char*>(env));
                //printf("check\n");
                strcat(copy,env);
                //printf("check\n");
                //printf("%s\n",copy);
                //free(env);
                
            
            }
            //printf("dollar3: %s\n",dollar);

            std::string secondPart;
            int nextDollar = dollarcc.find('$');
            //printf("%d\n", nextDollar);
            //printf("%d\n", pos);
            if (dollarcc[pos + 1] != '$' && dollarcc[pos + 2] != '{') {
                if (nextDollar != -1) {
                    secondPart =  dollarcc.substr(pos + 1, nextDollar - (pos + 1));
                } else {
                    secondPart = dollarcc.substr(pos + 1);
                }
            }
            
            char *copyTwo = (char*)malloc((100) * sizeof(char));
            //printf("%s\n", copyTwo);
            for (int j = 0; j < secondPart.length(); j++) {
                copyTwo[j] = secondPart[j];
            }
            
            //printf("dollar4: %s\n",dollar);
            strcat(copy,copyTwo);
            //printf("%s\n", copy);
            //printf("copy : %s\n", copy);
            //printf("dollar5: %s\n", dollar);
            dollar++;
            dollar++;
            dollar++;
            //printf("dollar6 %s\n", dollar);
            dollar = strchr(dollar, '$');
            //printf("dollar7 %s\n", dollar);
            free(copyTwo);
            
        }
        //printf("hello\n");
        *argument = std::string(strdup(copy));
        //printf("%s\n", copy);
        free(copy);
        return argument;
        
    }
    return NULL;

}

void Command::insertSimpleCommand( SimpleCommand * simpleCommand ) {
    // add the simple command to the vector
    int size = simpleCommand->_arguments.size();
	for (int i = 0; i < size; i++) {
        std::string * arg = simpleCommand->_arguments[i];
		std::string * exp =  new std::string(*arg);

        std::string * expansion = checkEnvExpansion(exp);
        if(expansion) {
            //printf("check\n");
		    simpleCommand->_arguments[i] = expansion;
		}
	}
    _simpleCommands.push_back(simpleCommand);
}

void Command::clear() {
    // deallocate all the simple commands in the command vector
    for (auto simpleCommand : _simpleCommands) {
        delete simpleCommand;
    }

    // remove all references to the simple commands we've deallocated
    // (basically just sets the size to 0)
    _simpleCommands.clear();
    if (_outFile == _errFile) {
        _outFile = NULL;
        _errFile = NULL;
    }
    if ( _outFile ) {
        delete _outFile;
    }
    _outFile = NULL;

    if ( _inFile ) {
        delete _inFile;
    }
    _inFile = NULL;

    if ( _errFile ) {
        delete _errFile;
    }
    _errFile = NULL;

    _background = false;
}

void Command::print() {
    printf("\n\n");
    printf("              COMMAND TABLE                \n");
    printf("\n");
    printf("  #   Simple Commands\n");
    printf("  --- ----------------------------------------------------------\n");

    int i = 0;
    // iterate over the simple commands and print them nicely
    for ( auto & simpleCommand : _simpleCommands ) {
        printf("  %-3d ", i++ );
        simpleCommand->print();
    }

    printf( "\n\n" );
    printf( "  Output       Input        Error        Background\n" );
    printf( "  ------------ ------------ ------------ ------------\n" );
    printf( "  %-12s %-12s %-12s %-12s\n",
            _outFile?_outFile->c_str():"default",
            _inFile?_inFile->c_str():"default",
            _errFile?_errFile->c_str():"default",
            _background?"YES":"NO");
    printf( "\n\n" );
}

void Command::execute() {
    // Don't do anything if there are no simple commands
    if ( _simpleCommands.size() == 0 ) {
        Shell::prompt();
        return;
    }
    if (strcmp(const_cast<char*>((_simpleCommands[0]->_arguments[0])->c_str()), "cd") == 0) {
        if ((_simpleCommands[0]->_arguments.size() == 1)) {
            chdir(getenv("HOME"));
        } else if ((_simpleCommands[0]->_arguments.size() == 2)) {
            if ((chdir(_simpleCommands[0]->_arguments[1]->c_str()) != 0)) {
                fprintf(stderr, "cd: can't cd to %s\n", _simpleCommands[0]->_arguments[1]->c_str());
            }
        }
        clear();
        Shell::prompt();
        return;
    }
    //char ** environment = environ;
    for (int i = 0; i < _simpleCommands.size(); i++) {
        if (i == (_simpleCommands.size() - 1)) {
            std::vector<std::string *> args = _simpleCommands[i]->_arguments;
            int size = args.size();
            bool flag = false;
            for (int x = 0; x < size; x++) {
                std::string * str = args[x];
                if(!str->compare("2>") || !str->compare(">&") || !str->compare(">>") || !str->compare(">>&")){
                    Command::lastArg = strdup(args[x-1]->c_str());
                    flag = true;
                }
            }

            if (flag == false) {
                Command::lastArg = strdup(args[args.size() -1]->c_str());

            }

        }
        
        for (int j = 0; j <_simpleCommands[i]->_arguments.size(); j++) {
            
            if (strcmp(const_cast<char*>((_simpleCommands[i]->_arguments[j])->c_str()), "exit") == 0) {
               if (isatty(0)) {
                    printf("Goodbye! \n");
                }
                exit(1);
            }
            /*if (strcmp(const_cast<char*>((_simpleCommands[i]->_arguments[j])->c_str()), "printenv") == 0) {
                while(*environment != NULL){
                    printf("%s\n",*environment);
                    environment++;
                }
                fflush(stdout);
            }
           */
            if (strcmp(const_cast<char*>((_simpleCommands[i]->_arguments[j])->c_str()), "setenv") == 0) {
                
                int err = setenv((_simpleCommands[i]->_arguments[1])->c_str(), (_simpleCommands[i]->_arguments[2])->c_str(),1);
                if (err != 0) {
                    perror("setenv");
                    exit(1);
                    
                }
                clear();
                Shell::prompt();
                return;
               
            }
            if (strcmp(const_cast<char*>((_simpleCommands[i]->_arguments[j])->c_str()), "unsetenv") == 0) {
                
                int err = unsetenv((_simpleCommands[i]->_arguments[1])->c_str());
                if (err != 0) {
                    perror("unsetenv");
                    exit(1);
                    
                }
                clear();
                Shell::prompt();
                return;
               
            }
        }
    }



    if (_ambiguity != false) {
        printf("Ambiguous output redirect.\n");
    }

    
    // Print contents of Command data structure
    //print();

    // Add execution here
    // For every simple command fork a new process
    // Setup i/o redirection
    // and call exec
    int defInput = dup(0);
    int defOutput = dup(1);
    int defErr = dup(2);
    int input;
    int output;
    int error;
    int ret;
    // if error file exists create it
    if (_errFile) {
        if (_append == 1) {
            error = open(_errFile->c_str(), O_CREAT|O_WRONLY|O_APPEND, 0600);
        } else {
            error = open(_errFile->c_str(), O_CREAT|O_WRONLY|O_TRUNC, 0600);
        }
    } else {
        error = dup(defErr);
    }
    dup2(error, 2);
    close(error);
    // if input file exists create it
    if (_inFile) {
        input = open(_inFile->c_str(), O_RDONLY);
        
    } else {
        input = dup(defInput);
    }
    
    
    //std::cout <<  _simpleCommands.size();
    for (int i = 0; i < _simpleCommands.size(); i++) {
        //std::cout << " \n check 1b2\n";
        dup2(input, 0);
        close(input);
        //std::cout << " \n check 1b2 2\n";
        if ( i == _simpleCommands.size() - 1) { 
            //std::cout << " \n check 1b2 3\n";
            if (_outFile) {
                if (_append == 1) {
                    //std::cout << " \n app \n";
                    output = open(_outFile->c_str(), O_CREAT|O_WRONLY|O_APPEND, 0600);
                } else {
                    //std::cout << " \n trunc\n"; 
                    output = open(_outFile->c_str(), O_CREAT|O_WRONLY|O_TRUNC, 0600);
                }
            } else {
                output = dup(defOutput);
            }
        //std::cout << " \n check 1b2 4\n";
       
        } else {
            int fdpipe[2];
            pipe(fdpipe);
            output = fdpipe[1];
            input = fdpipe[0];
        }
        //std::cout << " \n check 1b2 5\n";
        dup2(output, 1);
        //std::cout << " \n check 1b2 6\n";
        close(output);
        //std::cout << " \n check 1b2 7\n";
        //std::cout << " \n check 1b1\n";
        extern char ** environ;
        ret = fork();
        if (ret == 0) {
            //std::cout << " \n check 1b1\n";
            if (strcmp(const_cast<char*>((_simpleCommands[0]->_arguments[0])->c_str()), "printenv") == 0) {
                char **environment = environ;
                while(*environment != NULL){
                    printf("%s\n",*environment);
                    environment++;
		        }
                
                
		        //fflush(stdout);
                close(defInput);
                close(defOutput);
                close(defErr);
		        exit(0);
            }
            // we must convert the simpleCommand arguments to a vector of char to pass through execvp
            int argSize = (_simpleCommands[i]->_arguments).size();
            std::cout << argSize;
            std::vector<char*> argc(argSize + 1, nullptr);
            for (int j = 0; j < argSize; j++) {
                //std::cout << " \n check 1b1\n";
                argc[j] = const_cast<char*>((_simpleCommands[i]->_arguments[j])->c_str());
                //std::cout << " \n check 1b1\n";
            }
            argc[argSize + 1] = NULL;
            //std::cout << " \n check 1b12\n";
            execvp(argc[0], argc.data());
            //std::cout << " \n check 1b1\n";
            // there was an error
            perror("\nexecvp\n");
            exit(1);
            
         } else if (ret < 0) {
             // there was an error in fork
             perror("fork");
             return;
         } 
    }
    int atty = isatty(0);
    //std::cout << isatty(0);
    
    dup2(defInput, 0);
    dup2(defOutput, 1);
    dup2(defErr, 2);
    close(defInput);
    close(defOutput);
    close(defErr);
    if (!_background) {
        int stat;
        waitpid(ret,&stat,0);
        Command::lastStatus = stat;
    } else {
        Command::lastPid = ret;
    }
    

    // Setup i/o redirection
    // and call exec

    // Clear to prepare for next command
    clear();
    Shell::prompt();
    // Print new prompt
    //std:: << isatty(0);
/*
    if ( atty == 1) {
        //std::cout<<"here";
        Shell::prompt();
    } else {
        Shell::setFlag(false);
    }
    */

    
}

SimpleCommand * Command::_currentSimpleCommand;
int Command::lastStatus;
int Command::lastPid;
char * Command::lastArg;
