/* Main Program
*/

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "scheduler.h"
#include "list.h"

// constants to deal with command lines argument
#define COMMAND_LINE_0 0
#define COMMAND_LINE_1 1
#define COMMAND_LINE_2 2
#define COMMAND_LINE_3 3
#define COMMAND_LINE_4 4
#define TRIGGER_CHALLENGE 6
#define CHALLENGE 1
#define NON_CHALLENGE 0

// Read input from files and save them to double linked lists
void read_input(list_t *list, char *filename);

// Main Function to start
int main(int argc, char *argv[]) {
    char *filename;
    int cpu_num;
    list_t *incoming = make_empty_list();



    // Determine the order of command line arguments
    if (strcmp(argv[COMMAND_LINE_1], "-p")==0) {
        cpu_num = argv[COMMAND_LINE_2][COMMAND_LINE_0] - '0';
        filename = argv[COMMAND_LINE_4];
    }
    else {
        filename = argv[COMMAND_LINE_2];
        cpu_num = argv[COMMAND_LINE_4][COMMAND_LINE_0] - '0';
    }

    // We should use the improved algorithms to reduce makespan
    

    // Read and save input from file
    read_input(incoming, filename);

    // Challenge output
    if (argc == TRIGGER_CHALLENGE) {
        scheduling(incoming, cpu_num, CHALLENGE);
    }
    // Normal output
    else {
        scheduling(incoming, cpu_num, NON_CHALLENGE);
    }
    return 0;
}


// Read input from files and save them to double linked lists
void read_input(list_t *list, char *filename) {
    assert(list!=NULL);
    FILE *fp; 
    fp = fopen(filename, "r");
    char *line = NULL;
    size_t len;
    ssize_t read;
    int arrive;
    int pid;
    int time;
    node_t *curr;
    node_t *old;
    int inserted = 0;
    

    if (fp==NULL) {
        exit(EXIT_FAILURE);
    }

    // read information from file line by line
    while ((read = getline(&line, &len, fp)) != -1) {
        // allocate each line's information to a node
        char *token = strtok(line, " ");
        arrive = atoi(token);
        token = strtok(NULL, " ");
        pid = atoi(token);
        token = strtok(NULL, " ");
        time = atoi(token);
        token = strtok(NULL, " ");

        process_t process;
        // determine whether it is parallelisable
        if (token[0] == 'p') {
            process.sub = -1;
            process.sub_process = make_empty_list();
        }
        else {
            process.sub = 0;
            process.sub_process = NULL;
        }

        process.arrive = arrive;
        process.pid = pid;
        process.remain = time;
        process.time = time;
        //Haven't been allocated to any cpu yet
        process.cpu = -1;
        process.sub_id = -1;

        // insert process to list according to arrive time, remain time and pid
        if (is_empty_list(list)) {
            insert_at_head(list, process);
        }
        
        else {
            curr = list->head;
            while (curr) {
                process_t one = curr->process;
        
                if (process.arrive < one.arrive || 
                (process.arrive == one.arrive && process.remain < one.remain) ||
                (process.arrive == one.arrive && process.remain == one.remain &&
                 process.pid < one.pid)) {
                    inserted = 1;

                    node_t *new_node;
                    new_node = (node_t*)malloc(sizeof(*new_node));
                    new_node->process = process;
            
                    // add to the linked list
                    if (curr->prev!=NULL) {
                        old = curr->prev;
                        old->next = new_node;
                        new_node->next = curr;
                        curr->prev = new_node;
                    }
                    // the new process is of the highest priority
                    else {
                        list->head = new_node;
                        new_node->prev = NULL;
                        new_node->next = curr;
                        curr->prev = new_node;
                    }
                    break;   
                }
                curr=curr->next;
            } 
            if (inserted == 0) {
                insert_at_foot(list, process);
            }       
            inserted = 0;    
        }
        
    }
    

    fclose(fp);

}