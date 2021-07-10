#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "code_analyzer.h"
#include <unistd.h>

FILE *fin; 

int line_num = 1; /*line number of the current instruction */
int data_hazards[5]; /*an array to store the line number where data hazards are found*/
int data_hazard_count = 0; /*a counter for data hazards */

int init_stalls[4] = {0,0,0,0}; /* an array to keep the initial stalls before instruction fetch */
int stalls[4] = {0,0,0,0}; /* an array to keep the actual stalls needed after the instruction fetch */
int data_hazard_bool[4] = {0,0,0,0}; /* an array to keep track if there was a data hazard in the instructions values can be only 0 or 1 */

/*Options to run the program */
int no_solution =0;
int stall_solution = 0;
int forwarding = 0;

int main(int argc, char **argv){


    FILE *fin; 

    if( argc != 3){
        printf("usage: %s filename \"solution method\" ", argv[0]);
    }

    else{

        fin = fopen(argv[1],"r");

        if( fin==0 ){
            printf("Could not open file\n");
        }

        if(strcmp(argv[2],"no solution") == 0){
            no_solution = 1;
        }

        else if(strcmp(argv[2],"stall") == 0){
            printf("You have choosen stall solution\n\n");
            stall_solution = 1;
        }

        else if(strcmp(argv[2],"forwarding") == 0){
            printf("You have choose forwarding solution\n\n");
            forwarding = 1;
        }

        else {
            printf("Please enter an appropriate solution\n");
            return -1;
        }
    }
    
    char line[256];

       /*To display the stages of each instruction at the end of all instruction executions */
    char* curr_ins;
    char* src_reg = malloc(3);    

    while(fgets(line,256,fin)){

        /* Detect a new line that would mean end of file and we need to exit */
        if(line[0]=='\n'){
            break;
        }

        char *pipe_stages = malloc(32);
        char* curr_line = malloc(18);

        /* fetch the instruction */
        curr_ins = instruction_fetch(line);

        /* fetch the registers and analyze for data hazards */
        src_reg = register_fetch(curr_ins,line); 

        strcat(curr_line,line);
        
        //printf("The line number is %d\n", line_num);

        int i;

        /*Give initial stalls */
        if(stall_solution == 1 && line_num > 1){

            //printf("The value of data_hazard_bool[%d] is %d\n", line_num-1 ,data_hazard_bool[line_num-1]);
            if(data_hazard_bool[line_num - 2] == 0){
                init_stalls[line_num - 1] = init_stalls[line_num-2] + 1;
                for(i=0; i < init_stalls[line_num-1]; i++){
                    strcat(pipe_stages,"S  ");
                }
            }
            else if(data_hazard_bool[line_num - 2] == 1 && line_num > 2){
                if(stalls[line_num-2] == 1 ){
                    init_stalls[line_num-1] = init_stalls[line_num-2] + 2;
                }
                else{
                    init_stalls[line_num-1]= init_stalls[line_num-2] + 3;
                }
                for(i=0; i < init_stalls[line_num-1]; i++){
                    strcat(pipe_stages,"S  ");
                }
            }
        }

        else if(no_solution == 1 &&  line_num > 1){
            for(int i= 0; i < line_num-1; i++){
                strcat(pipe_stages,"S  ");
            }
        }

        /*Check for forwarding solution */
        else if(forwarding == 1 && line_num > 1){
            if(data_hazard_bool[line_num-2] == 0){
                init_stalls[line_num -1] = init_stalls[line_num-2] +1;
                for(int i=0; i < init_stalls[line_num-1]; i++){
                    strcat(pipe_stages,"S  ");
                }
            }
            else if(data_hazard_bool[line_num-2] == 1){
                init_stalls[line_num-1] = init_stalls[line_num-2] + 2;
                for(int i=0; i < init_stalls[line_num-1]; i++){
                    strcat(pipe_stages,"S  ");
                }
            }
        }

        /*Store the IF stage of the pipeline in the pipe_stages string */
        strcat(pipe_stages,"IF ");

        /* Check if there is a data hazard and give appropriate stalls*/
        if(stall_solution == 1 && data_hazard_bool[line_num - 1] == 1){
            for(int i=0; i < stalls[line_num-1]; i++){
                strcat(pipe_stages,"S  ");
            }
        }

        /*Chec if forwarding is turned on */
        else if(forwarding == 1 && data_hazard_bool[line_num -1] == 1){
            for(int i=0; i < stalls[line_num-1]; i++){
                strcat(pipe_stages,"S  ");
            }
        }
    
        strcat(pipe_stages,"ID ");
        strcat(pipe_stages,"EX ");
        strcat(pipe_stages,"M  ");
        strcat(pipe_stages,"W  \n");   

        /*print the current line*/
        printf("%s",curr_line);

        /*Print the finalized pipeline stages */
        printf("%s\n", pipe_stages);

        line_num++;

    }

    printf("\nThe total data hazard(s) found are %d \n", data_hazard_count);
    
    /*print out the line number where dependencies are found */

    for(int i=0; i < data_hazard_count; i++)
         printf("data hazard found in line: %d \n", data_hazards[i]);

    /*close the file */
    fclose(fin);

	return 0;
}