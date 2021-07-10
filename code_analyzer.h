#ifndef CODE_ANALYZER_H_
#define CODE_ANALYZER_H_

#include <stdio.h>
#include<string.h>
#include <unistd.h>

#define INITIAL_ALLOC 512
#define HASHSIZE 100 

extern int line_num;
extern int forwarding;
extern int no_solution;

extern int data_hazards[5];
extern int data_hazard_count;
extern int data_hazard_bool[4];
extern int stalls[4];


struct nlist { /*basic table entry */
	char* regname;
	char* def; /* This will be the line number when a data dependency occured last for a given register name */
	struct nlist * next;
};

static struct nlist *hashtab[HASHSIZE]; /*pointer table */

unsigned int hash(char* s); /* form hash value for string s */

struct nlist *lookup(char* s); /*look for s in hashtab */

struct nlist *install(char * name, char* def); /*put name and def in hash table */


char* instruction_fetch(char*line); /*function to fetch the instruction  */

char* register_fetch(char* ins, char*line); /*function to get the register being used by the instruction */


#endif /* CODE_ANALYZER_H_ */
