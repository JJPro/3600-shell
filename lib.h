#define _BSD_SOURCE

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#define ARG_LEN 200

bool background;

////// Usual operations /////////
void prompt();
int getargs(int * argc, char* args[]);
void runcmd(char *cmd, char* argv[], bool background);
void shift_elements(char* array[], int *length, int start_index, int many);

///// Extra Features ////////
char prev_dir[200];
int change_dir(char* dest);



////// Define Pool structure to hold any number of variables /////
typedef struct pool_item_t{
	char * arg;
	struct pool_item_t * next;
} pool_item;

typedef struct {
	pool_item * head;
} pool_t;

pool_t* pool;

////// Pool operations /////////
void init_pool();
void add_to_pool(char * arg);
void free_pool();