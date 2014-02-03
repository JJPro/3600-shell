#define _BSD_SOURCE

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#define ARG_LEN 200

// extern bool background = false;


////// Usual operations /////////
void prompt();
int getargs(int * argc, char* args[]);
void runcmd(char *cmd, char* argv[], bool background);
void redirect(int old, int new);
void error_exit( int status, const char *error_msg );


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


// Note: 
// i++ on Terminating a string
// Reset j on Terminating a string 
// Reset n on Terminating a string

// #define Terminate 
// if ( j >= ARG_LEN * n -1){
// 	arg = (char*) calloc(ARG_LEN * (++n));
// 	add_to_pool(arg);
// } 
// *(arg+j) = '\0';
// i++;
// j = 0;
// n = 1;


// #define Append 
// if ( j >= ARG_LEN * n -1){
// 	arg = (char*) calloc(ARG_LEN * (++n));
// 	add_to_pool(arg);
// } 
// *(arg+j) = c;
// j++;


// #define Start_New 
// arg = (char*) malloc(ARG_LEN);
// add_to_pool(arg);
// *(arg+j) = c;
// j++;
