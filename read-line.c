/*
 * CS252: Systems Programming
 * Purdue University
 * Example that shows how to read one line with simple editing
 * using raw terminal.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define MAX_BUFFER_LINE 2048

extern void tty_raw_mode(void);

// Buffer where line is stored
int line_length;
char line_buffer[MAX_BUFFER_LINE];
char right_side_buffer[MAX_BUFFER_LINE];
int right_side_length;

// Simple history array
// This history does not change. 
// Yours have to be updated.
int history_index = -1;
int historyPointer = -1;
char ** history;
int history_length = 128;
//int history_length = sizeof(history)/sizeof(char *);

void read_line_print_usage()
{
  char * usage = "\n"
    " ctrl-?       Print usage\n"
    " Backspace    Deletes last character\n"
    " up arrow     See last command in the history\n";

  write(1, usage, strlen(usage));
}

/* 
 * Input a line with some basic editing.
 */
char * read_line() {
  if (!history) {
    history = (char**)malloc(sizeof(char*) * history_length);
  }
  
  int mouse = 0;

  // Set terminal in raw mode
  tty_raw_mode();

  line_length = 0;

  // Read one line until enter is typed
  while (1) {

    // Read one character in raw mode.
    char ch;
    read(0, &ch, 1);

    if (ch>=32 && ch <= 126) {
      // It is a printable character. 

      // Do echo
      write(1,&ch,1);

      // If max number of character reached return.
      if (line_length==MAX_BUFFER_LINE-2) break; 

      // add char to buffer.
      mouse++;
      line_buffer[line_length]=ch;
      line_length++;


    }
    else if (ch==10) {
      // <Enter> was typed. Return line
      if (history_index < history_length) {
        history_index++;
        historyPointer = history_index;
        history[history_index] = strdup(line_buffer, line_length);
      }
      
      // Print newline
      write(1,&ch,1);
      mouse = 0;
      break;
    }
    else if (ch == 31) {
      // ctrl-?
      read_line_print_usage();
      line_buffer[0]=0;
      break;
    } else if (ch == 4) {
      // ctrl d
      if (line_length == 0) {
        continue;
      }

      char c = mouse--;
      write(1, &mouse, 1);

      ch = ' ';
      write (1, &ch, 1);
      mouse--;
      line_length--;


    }
    else if (ch == 8 || ch == 127) {
      // <backspace> was typed. Remove previous character read.
      if (mouse > 0) {
        // Go back one character
        ch = 8;
        write(1,&ch,1);
     
        // Write a space to erase the last character read
        ch = ' ';
        write(1,&ch,1);

        // Go back one character
        ch = 8;
        write(1,&ch,1);
        mouse--;
        // Remove one character from buffer
        line_length--;
      }
    
    // ctrl E was pressed 
    } else if (ch == 5) {
      while (mouse < line_length) {
        char temp = 27;
        write(1, &temp, 1);
        temp = 91;
        write(1, &temp, 1);
        temp = 67;
        write(1, &temp, 1);
        mouse++;
      }
    }
    // ctrl A was pressed
    else if (ch==1) {
      while (mouse > 0) {
        char temp = 27;
        write(1, &temp, 1);
        temp = 91;
        write(1, &temp, 1);
        temp = 68;
        write(1, &temp, 1);
        mouse--;
      }
    }

    else if (ch==27) {
      // Escape sequence. Read two chars more
      //
      // HINT: Use the program "keyboard-example" to
      // see the ascii code for the different chars typed.
      //
      char ch1; 
      char ch2;
      read(0, &ch1, 1);
      read(0, &ch2, 1);
      if (ch1==91 && ch2==65) {
	      // Up arrow. Print next line in history.

	      // Erase old line
	      // Print backspaces
        int i = 0;
        for (i =0; i < line_length; i++) {
          ch = 8;
          write(1,&ch,1);
        }

        // Print spaces on top
        for (i =0; i < line_length; i++) {
          ch = ' ';
          write(1,&ch,1);
        }

        // Print backspaces
        for (i =0; i < line_length; i++) {
          ch = 8;
          write(1,&ch,1);
        }	

        // Copy line from history
        if (historyPointer >= 0) {
          strcpy(line_buffer, history[historyPointer]);
          line_length = strlen(line_buffer);
          historyPointer--;
          mouse = strlen(line_buffer);
       }

        // echo line
        write(1, line_buffer, line_length);
      
      } else if (ch1==91 && ch2==67) {
        if (mouse < line_length) {
          char temp = 27;
          write(1, &temp, 1);
          temp = 91;
          write(1, &temp, 1);
          temp = 67;
          write(1, &temp, 1);
          mouse++;
        }
      } else if (ch1==91 && ch2==66) {
        for (int i = 0; i < line_length; i++) {
          ch = 8;
          write(1, &ch, 1);
        }
        for (int i = 0; i < line_length; i++) {
          ch = ' ';
          write(1, &ch, 1);
        }
        for (int i = 0; i < line_length; i++) {
          ch = 8;
          write(1, &ch, 1);
        }
        if (historyPointer < history_index) {
          historyPointer++;
          strcpy(line_buffer, history[historyPointer]);
          line_length = strlen(line_buffer);
          mouse = strlen(line_buffer);
        }

        write(1, line_buffer, line_length);
      } else if (ch1 == 91 && ch2 == 68) {
        if (mouse > 0) {
          char temp = 27;
          write(1, &temp,1);
          temp = 91;
          write(1, &temp,1);
          temp = 68;
          write(1, &temp,1);
          mouse--;

        }
      }
      
    }

  }

  // Add eol and null char at the end of string
  line_buffer[line_length]=10;
  line_length++;
  line_buffer[line_length]=0;

  return line_buffer;
}

