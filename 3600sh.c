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
// extern bool background;

int main(int argc, char*argv[]) {
  // Code which sets stdout to be unbuffered
  // This is necessary for testing; do not change these lines
  USE(argc);
  USE(argv);
  setvbuf(stdout, NULL, _IONBF, 0); 
  
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


/* parse the args list for special keywords, such as exit, >, <, 2>  */
void parse_args(int argc, char* args[]){
  if (strcmp(*args, "exit") == 0) // exit the program when first cmd is exit
    do_exit();
  // detect stream redirections:
  int red_in_pos = 0;
  int red_out_pos = 0;
  int red_err_pos = 0;
  int i = 0;
  int redirected = 0;
  int pid;
  int fd;

  for (i=0; i < argc; i++){
    if (strcmp(args[i], ">") == 0){
      if ((pid = fork()) == 0) {
        red_out_pos = i+1;
        fd = open(args[red_out_pos], O_TRUNC | O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
        if (fd == -1) perror("open for reading");
        close(STDOUT_FILENO);
        redirect(STDOUT_FILENO, fd);
        args[i] = NULL;
        execvp(args[0], args);
      } else {
        waitpid(pid, 0, 0);
        close(fd);
        redirected = 1;
        //prompt();
        break;
      }
    } else if (strcmp(args[i], "<") == 0){
      if ((pid = fork()) == 0) {
        red_in_pos = i+1;
        fd = open(args[red_in_pos], O_RDONLY);
        if (fd == -1) perror("open for reading");
        close(STDIN_FILENO);
        redirect(STDIN_FILENO, fd);
        args[i] = NULL;
        execvp(args[0], args);
      } else {
        waitpid(pid, 0, WNOHANG);
        close(fd);
        redirected = 1;
        //prompt();
        break;
      }
    } else if (strcmp(args[i], "2>") == 0){
      if ((pid = fork()) == 0) {
        red_err_pos = i+1;
        fd = open(args[red_err_pos], O_TRUNC | O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
        if (fd == -1) perror("open for reading");
        close(STDERR_FILENO);
        redirect(STDERR_FILENO, fd);
        args[i] = NULL;
        execvp(args[0], args);
      } else {
        waitpid(pid, 0, 0);
        close(fd);
        redirected = 1;
        //prompt();
        break;
      }
    }
  }
  if (!redirected)
    runcmd(args[0], args, 0);
}
