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

int main(int argc, char*argv[]) {
  // Code which sets stdout to be unbuffered
  // This is necessary for testing; do not change these lines
  USE(argc);
  USE(argv);
  setvbuf(stdout, NULL, _IONBF, 0);
  background = false;
  
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
  exit(0);
}


/* parse the args list for special keywords, such as exit, >, <, 2>, &  */
void parse_args(int argc, char* args[]){
  if (strcmp(*args, "exit") == 0) // exit the program when first cmd is exit
    do_exit();
  // detect stream redirections:
  int red_in_pos = 0;
  int red_out_pos = 0;
  int red_err_pos = 0;
    
  // keep a copy of original file descriptors of stdin, stdout, stderr
    int ori_fds[3];
    ori_fds[0] = dup(STDIN_FILENO);
    ori_fds[1] = dup(STDOUT_FILENO);
    ori_fds[2] = dup(STDERR_FILENO);

  int i = 0;
  int redirected = 0;
  int fd;
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
        fd = open(args[red_out_pos], O_TRUNC | O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
        if (fd == -1){
            fprintf(stderr, "Error: Unable to open redirection file.\n");
            return;
        }
        
        // redirecting output to the file
        fflush(stdout);
//        close(STDOUT_FILENO);
        newfile[1] = redirect(STDOUT_FILENO, fd);
        close(fd);

        redirected = 1;
        // TODO: shift-array
        shift_elements(args, &argc, i, 2);

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
        fd = open(args[red_in_pos], O_RDONLY);
        if (fd == -1){
            fprintf(stderr, "Error: Unable to open redirection file.\n");
            return;
        }
//        close(STDIN_FILENO);
        newfile[0] = redirect(STDIN_FILENO, fd);
        close(fd);
        redirected = 1;
        // TODO: shift-array
        shift_elements(args, &argc, i, 2);
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
        fd = open(args[red_err_pos], O_TRUNC | O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
        if (fd == -1){
            fprintf(stderr, "Error: Unable to open redirection file.\n");
            return;
        }
//        close(STDERR_FILENO);
        newfile[2] = redirect(STDERR_FILENO, fd);
        close(fd);
        redirected = 1;
        // TODO: shift-array
        shift_elements(args, &argc, i, 2);
    } else if (strcmp(args[i], "&") == 0){
        if (i == argc-1 ){
          background = true;
          args[i] = NULL;
        } else {
            fprintf(stderr, "Error: Invalid syntax.\n");
            // abort this line of command
            return;

        }
    }
  }

  if (!redirected)
    runcmd(args[0], args, background);
  background = false; // reset background
    
  // close newfile
    for (int i=0; i< 3; i++){
        if (newfile[i] != -1)
            close(newfile[i]);
    }

  // reset stdin, stdout, stderr
  dup2(ori_fds[0], STDIN_FILENO);
  dup2(ori_fds[1], STDOUT_FILENO);
  dup2(ori_fds[2], STDERR_FILENO);
}
