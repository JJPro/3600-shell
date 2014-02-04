/**
 * CS3600, Spring 2013
 * Project 1 Starter Code
 * (c) 2013 Alan Mislove
 *
 * You should use this (very simple) starter code as a basis for 
 * building your shell.  Please see the project handout for more
 * details.
 */

#include "3600sh.h"
#include "lib.h"

#define USE(x) (x) = (x)
extern pool_t* pool;
extern bool background;
extern char prev_dir[];

int main(int argc, char*argv[]) {
  // Code which sets stdout to be unbuffered
  // This is necessary for testing; do not change these lines
  USE(argc);
  USE(argv);
  setvbuf(stdout, NULL, _IONBF, 0);
  background = false;
  strcpy(prev_dir, getenv("PWD"));
  
  // Main loop that reads a command and executes it
  while (1) {         
    // You should issue the prompt here
    prompt();
    // You should read in the command and execute it here
    int argcount = 0;
    char ** args = (char**) calloc(sizeof(char**), ARGS_SIZE);
    while (1){
      fflush(stdout);
      fflush(stdin);
      int arg_num = getargs(&argcount, args);
      if (arg_num == -1) {
        if (argcount > 0)
          parse_args(argcount, args);
        do_exit();
      }
      if (*args == NULL) {
        prompt();
        continue;
      }
      parse_args(argcount, args);
      memset(args, 0, (sizeof args) * ARGS_SIZE);
      argcount = 0;
      prompt();
    }

    do_exit(); // exit on EOF
  }

  return 0;
}

// Function which exits, printing the necessary message
//
void do_exit() {
  printf("So long and thanks for all the fish!\n");
  while (wait(NULL) > 0){}
  exit(0);
}


/* parse the args list for special keywords, such as exit, cd, >, <, 2>, &  */
void parse_args(int argc, char* args[]){
  if (strcmp(*args, "exit") == 0) // exit the program when first cmd is exit
    do_exit();
    
  if (argc >= 1 &&
      strcmp(args[0], "cd") == 0) // change working directory
  {
      char* dest;
      if (argc == 1 ||
          strcmp(args[1], "~") == 0)
          dest = getenv("HOME");
      else
      {
          if (strcmp(args[1], "-") == 0){
              char tmp[200];
              getcwd(tmp, 199);
              if ( change_dir(prev_dir) != 0 )
                  fprintf(stderr, "Error: failed to change directory.\n");
              strcpy(prev_dir, tmp);
              return;
          }
          else
              dest = args[1];
      }
      getcwd(prev_dir, 199);
      if ( change_dir(dest) != 0 )
          fprintf(stderr, "Error: failed to change directory.\n");
      return;
  }
  // detect stream redirections:
  int red_in_pos = 0;
  int red_out_pos = 0;
  int red_err_pos = 0;
    
    fflush(stdin);fflush(stdout); fflush(stderr);
  // keep a copy of original file descriptors of stdin, stdout, stderr
    int ori_fds[3];
    ori_fds[0] = dup(0);
    ori_fds[1] = dup(1);
    ori_fds[2] = dup(2);

  int i = 0;
  int newfile[3] = {-1, -1, -1};

  for (i=0; i < argc; i++){
    if (strcmp(args[i], ">") == 0){

        red_out_pos = i+1;
        if (red_out_pos >= argc){
            fprintf(stderr, "Error: Invalid syntax.\n");
            return;
        }
        if ((strcmp(args[red_out_pos], "<") == 0) ||
            (strcmp(args[red_out_pos], ">") == 0) ||
            (strcmp(args[red_out_pos], "2>") == 0))
        {
            fprintf(stderr, "Error: Invalid syntax.\n");
            return;
        }
        newfile[1] = open(args[red_out_pos], O_TRUNC | O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
        if (newfile[1] == -1){
            fprintf(stderr, "Error: Unable to open redirection file.\n");
            return;
        }
        dup2(newfile[1], 1);
        close(newfile[1]);

        // TODO: shift-array
        shift_elements(args, &argc, i, 2);
        i = i-1; // repoint the counter

    } else if (strcmp(args[i], "<") == 0){

        red_in_pos = i+1;
        if (red_in_pos >= argc){
            fprintf(stderr, "Error: Invalid syntax.\n");
            return;
        }
        if ((strcmp(args[red_in_pos], "<") == 0) ||
            (strcmp(args[red_in_pos], ">") == 0) ||
            (strcmp(args[red_in_pos], "2>") == 0))
        {
            fprintf(stderr, "Error: Invalid syntax.\n");
            return;
        }
        
        if ( (red_in_pos+1 <= argc-1) &&
      	     (strcmp(args[red_in_pos+1], "<") != 0) &&
            (strcmp(args[red_in_pos+1], ">") != 0) &&
            (strcmp(args[red_in_pos+1], "2>") != 0))
        {
	    fprintf(stderr, "Error: Invalid syntax.\n");
            return;
        }
        
        newfile[0] = open(args[red_in_pos], O_RDONLY);
        if (newfile[0] == -1){
            fprintf(stderr, "Error: Unable to open redirection file.\n");
            return;
        }
        dup2(newfile[0], 0);
        close(newfile[0]);
        
        // TODO: shift-array
        shift_elements(args, &argc, i, 2);
        i = i-1; // repoint the counter
    } else if (strcmp(args[i], "2>") == 0){

        red_err_pos = i+1;
        if (red_err_pos >= argc){
            fprintf(stderr, "Error: Invalid syntax.\n");
            return;
        }
        if ((strcmp(args[red_err_pos], "<") == 0) ||
            (strcmp(args[red_err_pos], ">") == 0) ||
            (strcmp(args[red_err_pos], "2>") == 0))
        {
            fprintf(stderr, "Error: Invalid syntax.\n");
            return;
        }
        newfile[2] = open(args[red_err_pos], O_TRUNC | O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
        if (newfile[2] == -1){
            fprintf(stderr, "Error: Unable to open redirection file.\n");
            return;
        }
        dup2(newfile[2], 2);
        close(newfile[2]);
        
        // TODO: shift-array
        shift_elements(args, &argc, i, 2);
        i = i-1; // repoint the counter
    } else if (strcmp(args[i], "&") == 0){
        if (i == argc-1 ){
          background = true;
          args[i] = NULL;
          argc = i;
        } else {
            fprintf(stderr, "Error: Invalid syntax.\n");
            // abort this line of command
            return;

        }
    }
  }
    

  /** detect more than one stdin or stdout or stderr redirection **/
  int count_red_in, count_red_out, count_red_err;
  count_red_in = count_red_out = count_red_err = 0;
  for(int i=0; i< argc; i++){
    if (strcmp(args[i], "<") == 0)
      count_red_in++;
    else if (strcmp(args[i], ">") == 0)
      count_red_out++;
    else if (strcmp(args[i], "2>") == 0)
      count_red_err++;
  }
  if (count_red_in > 1 || count_red_out > 1 || count_red_err >1)
  {
	fprintf(stderr, "Error: Invalid syntax.\n");
        return;
  }
  /** Done detecting .... **/
    
    
  runcmd(args[0], args, background);
  background = false; // reset background
  /**** restore file descriptors ****/
    fflush(stdin);fflush(stdout); fflush(stderr);
    dup2(ori_fds[0], 0);
    dup2(ori_fds[1], 1);
    dup2(ori_fds[2], 2);
    close(ori_fds[0]);
    close(ori_fds[1]);
    close(ori_fds[2]);
}
