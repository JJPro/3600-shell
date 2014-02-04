#include "lib.h"

// issue a prompt
void prompt() {
  // get user 
  char *user = getenv("USER");
  // get host name
  char host[50];
  gethostname(host, 50);
  // get current working directory
  char *wd = getenv("PWD");

  printf("%s@%s:%s> ", user, host, wd);
}

/* run the given command with given argument list. 
 * Note:  
 * 	first argument in the list should be the path of the command
 * 	the argument list must be terminated with NULL as the last item.
 * 
 * background: decides whether this command should run in background
 */
void runcmd(char *cmd, char* argv[], bool background){
  pid_t p; 
  p = fork();
  if (p < 0)
    fprintf(stderr, "error on fork()\n");
  else if (p == 0){ // in child process
    // replace the child process with the new process
    execvp(cmd, argv);
    if (errno == ENOENT)
      fprintf(stderr, "Error: Command not found.\n"); // an error should be reported if we reach here
    else if (errno == EACCES)
      fprintf(stderr, "Error: Permission denied.\n");
    else 
      fprintf(stderr, "Error: Failed to execute %s.\n", cmd);
    exit(EXIT_FAILURE);
  } else { // in parent process
    if (background){ // run child process in background?
//      prompt();	// yes, issue a new prompt immediately without waiting for the process to end
    }else { // no
      // wait for child process to end 
      waitpid(p, NULL, 0);
      // issue a new prompt
      //prompt();
    }
  }
}

/* redirect file stream */
int redirect(int old, int new){
  // dup2 makes newfd the copy of oldfd, then closes the newfd, and returns newfd
    int file = dup2(new, old);
  if (file == -1){
    perror("dup2");
    exit(EXIT_FAILURE);
  }
    return file;
}

/* Terminate the program with an error message: */
void error_exit( int status, const char *error_msg ){
  fprintf(stderr, "%s\n", error_msg);
  exit(status);
}

/* delete [many] number of elements starting from index [start_index], from the array, and shift the rest elements of the array forward 
    This function will update the array and its length
 */
void shift_elements(char* array[], int *length, int start_index, int many){
    for (int i=start_index; i<(*length - many); i++){
        array[i] = array[i+many];
    }
    *length = *length - many;
    array[*length] = NULL; // move the NULL element as well

}

/* parse the input stream for argc and args
 * return:
 * 		0 for \n
 * 	   -1 for EOF
 *     -2 for error
 */ 
int getargs(int * argc, char* args[]){
  fflush(stdin);
  int c, i = 0; // NOTE: i++ on terminating a string
  bool in_word = false;
  bool escape_hint = false;

  char * arg = NULL;  // a placeholder for result strings
  int j = 0;          // index for char in arg
  int n = 1;          // # of ARG_LENs to be applied to be mem size of the string .

  while (((c=getchar()) != EOF) && c != '\n'){
    /* split line into tokens with delimiter */
    /* delimiters include: space and tab */
    /* does not include: \ , \t, \\, \&:: eat the backslash and leave the character in string */
    /* for other backslash(ed) chars, fprintf(stderr, "Error: Unrecognized escape sequence.")
     */
    if ( ( (c == ' ') || (c=='\t') ) && !escape_hint ) //pure whitespace characters
      {
        if (in_word){
          in_word = false;
          // TODO: terminate the current string
          if ( j >= ARG_LEN * n -1){
            arg = (char*) calloc(ARG_LEN, ++n);
            add_to_pool(arg);
            args[i] = arg;
          } 
          *(arg+j) = '\0';
          i++;
          j = 0;
          n = 1;
        } // not in a word, skip the character
      }
    else if ( ( (c == ' ') || (c=='t') ) && escape_hint ) // escaped whitespace character
      {
        escape_hint = false; // reset flag
        if (in_word){
          // TODO: append this char to string 
          if ( j >= ARG_LEN * n -1){
            arg = (char*) calloc(ARG_LEN, ++n);
            add_to_pool(arg);
            args[i] = arg;
          }
            
          if (c==' ')
              *(arg+j) = ' ';
          else
              *(arg+j) = '\t';
          j++;
        } else {
          // TODO: start new string with the char
          arg = (char*) malloc(ARG_LEN);
          add_to_pool(arg);
          args[i] = arg;
          *(arg+j) = c;
          j++;

        }
      }
    else if ( !( (c == ' ') || (c=='\t') ) && escape_hint ) // mgith be escape character if valid
      {
        escape_hint = false; // reset flag
        if ( (c == '\\') || (c == '&')){ // valid escape character
          if (in_word){
            // TODO: append this char to string 
            if ( j >= ARG_LEN * n -1){
              arg = (char*) calloc(ARG_LEN ,++n);
              add_to_pool(arg);
              args[i] = arg;
            } 
            *(arg+j) = c;
            j++;
          } else {
            // TODO: start a new string with the char
            arg = (char*) malloc(ARG_LEN);
            add_to_pool(arg);
            args[i] = arg;
            *(arg+j) = c;
            j++;

          }
          in_word = true;
        }else{
          fprintf(stderr, "Error: Unrecognized escape sequence.\n");
          *argc = 0; *args = NULL; // set argc to 0 to indicate an error
          // eat all rest char in this line
            while (( (c=getchar()) != EOF) && c != '\n'){}
            if ( c == EOF)
                return EOF;
            else
                return 0;
        }
      }
    else { // other character, might be backslash
      if ( c == '\\' ) // backslash character
        {
          if (!escape_hint){ // next char might be escaped if valid escape sequence, otherwise error
            escape_hint = true;
          } else { // another escape character.
            escape_hint = false; // reset the hint flag
            if (in_word){
              // TODO: append \ to the string 
              if ( j >= ARG_LEN * n -1){
                arg = (char*) calloc(ARG_LEN, ++n);
                add_to_pool(arg);
                args[i] = arg;
              } 
              *(arg+j) = '\\';
              j++;
            } else {
              // TODO: start a new string with \ . 
              arg = (char*) malloc(ARG_LEN);
              add_to_pool(arg);
              args[i] = arg;
              *(arg+j) = '\\';
              j++;
            }
            in_word = true;								
          }
        }
      else // pure normal character 
        {
          if (in_word){
            // TODO: append this char to string 
            if ( j >= ARG_LEN * n -1){
              arg = (char*) calloc(ARG_LEN, ++n);
              add_to_pool(arg);
              args[i] = arg;
            } 
            *(arg+j) = c;
            j++;
          } else {
            // TODO: start a new string with this char
            arg = (char*) malloc(ARG_LEN);
            add_to_pool(arg);
            args[i] = arg;
            *(arg+j) = c;
            j++;
          }
          in_word = true;
        }
    }
  }
  if (in_word){
    // TODO: Terminate the string.
    if ( j >= ARG_LEN * n -1){
      arg = (char*) calloc(ARG_LEN, ++n);
      add_to_pool(arg);
      args[i] = arg;
    } 
    *(arg+j) = '\0';
    i++;
    j = 0;
    n = 1;
  }
  *argc = i;
  if (c == '\n')
    return 0;
  else 
    return EOF;
}


/////////////////////////////////// define Pool operations /////////////////////////////////////////
/* initialize the pool */
void init_pool(){
  pool = (pool_t *)malloc(sizeof(pool_t));
  pool->head = NULL;
}
/* add another arg variable to the pool
 */
void add_to_pool(char * arg){

  // init the pool if necessary
  if (!pool)
    init_pool();

  // construct the pool item to be added.
  pool_item * current, *item = (pool_item *)malloc(sizeof(pool_item));
  item->arg = arg;
  strcpy(item->arg, arg);
  item->next = NULL;

  // add to pool
  if ( (current = pool->head) == NULL ) // pool is empty
    pool->head = item;
  else { // pool is not empty
    while ( current->next ) // go to the last item
      current = current->next;
    current->next = item;
  }
}

/* release all the variables in the pool 
 * the pool collects malloc[ed] variables during the process of parsing args. 
 * You should call this function to release/free all the variables after executing the command
 */
void free_pool(){
  pool_item * current, *tmp;

  if ( (current = pool->head) == NULL ) // pool is empty
    ;
  else { // pool is not empty
    while ( current->next ){
      tmp = current->next;
      // free the arg in pool items
      free(current->arg);
      // free the pool items
      free(current);
      current = tmp;
    }	
  }
  // free the pool and set it the NULL
  free(pool);
  pool = NULL;
}
