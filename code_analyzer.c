
#include "code_analyzer.h"

#include <stdio.h>
#include <stdlib.h>
#include<string.h>


/*function to fetch the instruction  */
char* instruction_fetch(char*line){
    
    char* ins;

    if(strstr(line,"add") && !(strstr(line,"addi")) ){
        ins = "add";
    }

    else if(strstr(line,"sub")){
        ins = "sub";
    }

    else if(strstr(line,"lw")){
        ins = "lw";
    }

    else if(strstr(line,"sw")){
        ins="sw";
    }

    else{

        printf("Invalid instruction\n");
        ins = "invalid";
    }
    return ins;
} 

/*function to get the registers being used by the specified instruction ins,
  add and sub instructions are R-format so they have 3 registers per instruction
  lw and sw instructions are I-format so they have 2 registers per instruction
 */
char* register_fetch(char* ins, char*line)
{
	/* registers name are two characters length */
	char* des_reg = malloc(3); /* destination registers */
	char* operand_1 = malloc(3); /* first operand register */
	char* operand_2 = malloc(3); /* second operand register */
	
	int str_length = strlen(line);
	int current_reg = 1; /*a counter to keep track of the current register */

	struct nlist *np;

	/* Transforming the line number to a string to properly add it to the hash table */
	char* curr_line = malloc(1);
	sprintf(curr_line,"%d",line_num);

	if(strcmp(ins,"add") == 0){
		/* Loop to find registers */
		for(int i = 0; i < str_length; i++){
			
			if (line[i] == '$' && current_reg == 1){
				/* get the register name */
				des_reg[0] = line[i+1];
				des_reg[1] = line[i+2];
				//printf("The source register is %s\n", des_reg);

				current_reg++;

			}

			/*Check for the first operand register */
			else if (line[i] == '$' && current_reg == 2){
				/* get the register name */
				operand_1[0] = line[i+1];
				operand_1[1] = line[i+2];
				//printf("The first operand is %s \n", operand_1);

				/* Check for a Read After Write(RAW) data hazard */
				if((np = lookup(operand_1)) == NULL ){ /*It's not in the hash table so no need to worry */
					current_reg++;
				}
				
				/* register is in hash-table check for RAW hazard */
				else{
					int prev_line_num = atoi(np->def);
					int diff_hazard = (line_num) - prev_line_num;

					if(no_solution == 1 && diff_hazard <= 2){
						current_reg++;
						data_hazard_bool[line_num-1]= 1;
						data_hazards[data_hazard_count] = line_num;
						data_hazard_count++;

					}

					/* Check to see if forwarding is turned on */
					else if(forwarding == 1 && diff_hazard ==1){
						data_hazards[data_hazard_count] = line_num;
						data_hazard_count++;
						current_reg++;
						data_hazard_bool[line_num -1] = 1;
						stalls[line_num-1] = 1;
					}


					else if(diff_hazard <= 2 && forwarding == 0){

						current_reg++;
						data_hazard_bool[line_num-1]= 1;

						if(diff_hazard == 2 && stalls[line_num-2] != 2){
							stalls[line_num-1] = 1;
							data_hazards[data_hazard_count] = line_num;
							data_hazard_count++;
						}
						else if(diff_hazard == 1)
						{
							stalls[line_num-1] = 2;
							data_hazards[data_hazard_count] = line_num;
							data_hazard_count++;
						}
						else{
							data_hazard_bool[line_num-1] =0;
						}
						
					}
				}
			}
			/*Check for the second operand register*/
			else if (line[i] == '$' && current_reg == 3){
				/* get the register name*/
				operand_2[0] = line[i+1];
				operand_2[1] = line[i+2];
				//printf("The second operand is %s \n", operand_2);

				/* check that operand 1 and operand 2 register are not the same */

				if(strcmp(operand_1,operand_2) == 0)
					current_reg++;
				

				else if((np=lookup(operand_2)) == NULL) /*It's not in the hash table so no need to worry */
					current_reg++;

				/* register is in hash-table check for RAW hazard */				
				else { 
					int prev_line_num = atoi(np->def);
					int diff_hazard = (line_num) - prev_line_num;

					if(no_solution == 1 && diff_hazard <= 2){
						data_hazards[data_hazard_count] = line_num;
						data_hazard_count++;
						current_reg++;
						data_hazard_bool[line_num -1] = 1;
					}
					
					/* Check to see if forwarding is turned on */
					else if(forwarding == 1 && diff_hazard ==1){
						data_hazards[data_hazard_count] = line_num;
						data_hazard_count++;
						current_reg++;
						data_hazard_bool[line_num -1] = 1;
						stalls[line_num-1] = 1;
					}

					/* We need to check 2 instructions back for data hazard validity */
					else if(diff_hazard <= 2 && forwarding == 0){

						current_reg++;
						data_hazard_bool[line_num-1] = 1;
						if(diff_hazard == 2 && stalls[line_num - 2] != 2){
							stalls[line_num-1] = 1;
							data_hazards[data_hazard_count] = line_num;
							data_hazard_count++;
						}
						else if(diff_hazard == 1)
						{
							stalls[line_num-2] = 2;
							data_hazards[data_hazard_count] = line_num;
							data_hazard_count++;
						}
						else{
							data_hazard_bool[line_num-1] =0;
						}
						
					}
				}
			}
			else if(current_reg == 4){

				/* First check that the forwarding option is not "on" */

				if(forwarding == 1){
					break;
				} 

				/* add or update hash table with the destination register */
				else if((np=lookup(des_reg)) == NULL){
					np = install(des_reg,curr_line);
					break;
				}

				else{
					np -> def = curr_line;
					break;
				}
			}
		}
	}

	else if(strcmp(ins,"lw") == 0){
		/* Loop to find registers */
		for(int i = 0; i < str_length; i++ ){
			if (line[i] == '$' && current_reg == 1){

				des_reg[0] = line[i+1];
				des_reg[1] = line[i+2];

				//printf("The source register is %s\n", des_reg);

				current_reg ++;

 				if((np=lookup(des_reg)) == NULL){
					np = install(des_reg,curr_line); /* add to hash table */ 
				}

				else{ /* update hash table */
					np->def = curr_line;
				}
			}

			/* lw has only 2 registers */
			else if (line[i] == '$' && current_reg == 2){
				
				operand_1[0] = line[i+1];
				operand_1[1] = line[i+2];

				//printf("The second register is %s\n", operand_1);

				/* Check if forwarding is turned on */
				if(forwarding == 1){
					break;
				}
				/* Check for RAW data hazard */
				if((np=lookup(operand_1)) == NULL){ /* no hazard */
					current_reg++;
				}

				else {

					int prev_line_num = atoi(np->def);
					int diff_hazard = (line_num) - prev_line_num;

					/* we need to check 2 instructions back for data hazard validity */
					if (diff_hazard <= 2){
						data_hazards[data_hazard_count] = line_num;
						data_hazard_count++;
						current_reg++;
						data_hazard_bool[line_num-1] = 1;
						if(diff_hazard == 2)
							stalls[line_num-1] = 1;
						else
						{
							stalls[line_num-1] = 2;
						}
						
					}

					else {
						current_reg++;
					}
				}
			}
			else if (current_reg == 3){
				break;
			}
	}
	}

	else if(strcmp(ins,"sw") == 0){
		for(int i=0; i < str_length; i++){

			if(line[i] == '$' && current_reg == 1){

				des_reg[0] = line[i+1];
				des_reg[1] = line[i+2];

				//printf("The source register is %s\n", des_reg);
				
				if((np=lookup(des_reg)) == NULL)
					current_reg++; /* no data hazard */

				else {
					int prev_line_num = atoi(np->def);
					int diff_hazard = (line_num) - prev_line_num;

					/* Check to see if forwarding is turned on */
					if(forwarding == 1 && diff_hazard ==1){
						data_hazards[data_hazard_count] = line_num;
						data_hazard_count++;
						current_reg++;
						data_hazard_bool[line_num -1] = 1;
						stalls[line_num-1] = 1;
					}

					/* we need to check 2 instructions back for data hazard validity */
					else if (diff_hazard <= 2 && forwarding == 0){
						data_hazards[data_hazard_count] = line_num;
						data_hazard_count++;
						current_reg++;
						data_hazard_bool[line_num-1] = 1;
						if(diff_hazard == 2)
							stalls[line_num-1] = 1;
						else
						{
							stalls[line_num-1] = 2;
						}
					}

					else{
						current_reg++;
					}
				}
			}
			else if(line[i] == '$' && current_reg == 2){

				operand_1[0] = line[i+1];
				operand_1[1] = line[i+2];

				current_reg++;
				/* Check for RAW data hazard */
				if((np=lookup(operand_1)) == NULL) /* no hazard */
					np = install(operand_1,curr_line);

				else {
					int prev_line_num = atoi(np->def);
					int diff_hazard = (line_num) - prev_line_num;
					/* we need to check 2 instructions back for data hazard validity */
					if (diff_hazard <= 2){
						data_hazards[data_hazard_count] = line_num;
						data_hazard_count++;
						current_reg++;
						data_hazard_bool[line_num-1] = 1;
						if(diff_hazard == 2)
							stalls[line_num-1] = 1;
						else
						{
							stalls[line_num-1] = 2;
						}
					}
					/* update hash table */
					np->def = curr_line;
				}
			}
			else if(current_reg==3){
				break;
			}
		}
	}

	else if(strcmp(ins,"sub") == 0){
		for (int i =0; i < str_length; i++){
			/* to keep track of the data hazards in this instruction */
			if(line[i] == '$' && current_reg == 1){

				des_reg[0] = line[i+1];
				des_reg[1]= line[i+2];

				//printf("The source register is %s\n", des_reg);
				current_reg ++;
			}

			else if(line[i] == '$' && current_reg == 2){

				operand_1[0] = line[i+1];
				operand_1[1] = line[i+2];

				//printf("The first operand is %s \n", operand_1);

				/*Check for a Read After Write(RAW) data hazard */
				if((np=lookup(operand_1)) == NULL) /* not in the hash table so no hazard */
					current_reg++;

				else {
					int prev_line_num = atoi(np->def);
					int diff_hazard = (line_num) - prev_line_num;

					if(no_solution == 1 && diff_hazard <= 2){
						current_reg++;
						data_hazard_bool[line_num-1] = 1;
						data_hazards[data_hazard_count] = line_num;
						data_hazard_count++;
					}

					/* Check to see if forwarding is turned on */
					else if(forwarding == 1 && diff_hazard ==1){
						data_hazards[data_hazard_count] = line_num;
						data_hazard_count++;
						current_reg++;
						data_hazard_bool[line_num -1] = 1;
						stalls[line_num-1] = 1;
					}
					/* We need to check 2 instructions back for data hazard validity */
					else if (diff_hazard <= 2 && forwarding == 0){
						
						current_reg++;
						data_hazard_bool[line_num-1] = 1;

						if(diff_hazard == 2 && stalls[line_num - 2] != 2){
							stalls[line_num-1] = 1;
							data_hazards[data_hazard_count] = line_num;
							data_hazard_count++;
						}
						else if(diff_hazard == 1)
						{
							stalls[line_num-1] = 2;
							data_hazards[data_hazard_count] = line_num;
							data_hazard_count++;
						}
						else{
							data_hazard_bool[line_num-1] =0;
						}
						
					}
				}
			}
			else if(line[i] == '$' && current_reg == 3){

				operand_2[0] = line[i+1];
				operand_2[1] = line[i+2];

				//printf("The second operand is %s \n", operand_2);

				/* Check for duplication of operands*/
				if(strcmp(operand_1,operand_2) == 0)
					current_reg++;

				else if((np=lookup(operand_2)) == NULL)
					current_reg++;

				else {
					int prev_line_num = atoi(np->def);
					int diff_hazard = (line_num)- prev_line_num;

					if(no_solution == 1 && diff_hazard <= 2){
						data_hazards[data_hazard_count] = line_num;
						data_hazard_count++;
						current_reg++;
						data_hazard_bool[line_num -1] = 1;

					}

					/* Check to see if forwarding is turned on */
					else if(forwarding == 1 && diff_hazard ==1){
						data_hazards[data_hazard_count] = line_num;
						data_hazard_count++;
						current_reg++;
						data_hazard_bool[line_num -1] = 1;
						stalls[line_num-1] = 1;
					}

					/* We need to check 2 instructions back for data hazard validity */
					else if (diff_hazard <= 2 && forwarding == 0){

						current_reg++;
						data_hazard_bool[line_num-1] = 1;
						if(diff_hazard == 2 && stalls[line_num - 2] != 2){
							stalls[line_num-1] = 1;
							data_hazards[data_hazard_count] = line_num;
							data_hazard_count++;
						}
						else if(diff_hazard == 1)
						{
							stalls[line_num-1] = 2;
							data_hazards[data_hazard_count] = line_num;
							data_hazard_count++;
						}
						else{
							data_hazard_bool[line_num-1] =0;
						}
						
					}
				}
				
			}
			else if(current_reg == 4){
				/*Check if the forwarding option is "on" */
				if(forwarding == 1){
					break;
				}
				/* we will check if the first register is in the hash table*/
				else if((np=lookup(des_reg)) == NULL){
					np = install(des_reg,curr_line);
					break;
				}

				else{
					/*update the hash table */
					np ->def = curr_line;
					break;
				}
			}
		}
	}

	char *return_str = malloc(3);

	return_str[0] = des_reg[0];
	return_str[1] = des_reg[1];
	return_str[2] = '\0';

	return return_str;
}

/* form hash value for string s */
unsigned int hash(char* s)
{
	int hashval;

	for(hashval = 0; *s != '\0';)
		hashval += *s++;

	return (hashval % HASHSIZE);
}

/*look for s in hashtab */
struct nlist *lookup(char* s)
{
	struct nlist *np;

	for(np = hashtab[hash(s)]; np!=NULL; np= np->next)
		if(strcmp(s,np->regname)==0)
			return(np); /*found it*/
	return (NULL); /*not found */
}

//todo: add strdup()into function
struct nlist *install(char * name, char* def)
{
	struct nlist *np;
	unsigned hashval;

	if((np=lookup(name))==NULL) { /*not found */
		np = (struct nlist*) malloc(sizeof(*np));
		if(np==NULL || (np->regname = strdup(name)) == NULL){
			return(NULL);
		}
		hashval = hash(name);
		np->next = hashtab[hashval];
		hashtab[hashval]= np;
	}
	else{
		free((void *)np->def); /*free previous definition */
	}
	if( (np->def = strdup(def) )== NULL)
		return NULL;
	return (np);

}
