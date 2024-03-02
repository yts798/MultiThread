#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "scheduler.h"
#include "list.h"

#define MAXINT 2147483647
#define HUNDRED 100.0
#define LONGTIME 50

int
scheduling(list_t *incoming, int cpu_num, int is_challenge) {
    // variables that control the scheduling process
    int curr_time = 0;
    list_t *cpu_list[cpu_num];
    list_t *super_process;
    list_t *finish_process;
    list_t *small_process;
    process_t display;
    int all_finish = 0;
    int remaining = 0;
    int is_sub = 0;
    int more_process;
    int sub_num;
    int insert_to = -1;
    int new_id[cpu_num];
    int old_id[cpu_num];
    int has_free = 0;

    // variables that control the performance stat.
    double total_turnaround = 0.0;
    double turnaround;
    double ave_turnaround;
    double total_overhead = 0.0;
    double overhead;
    double ave_overhead;
    double max_overhead = -1.0;
    int process_num = 0;

    // initialise a linked list to save parallelisable 
    super_process = make_empty_list();
    finish_process = make_empty_list();
    small_process = make_empty_list();
    
    // initialise array of double linked list.
    for (int i = 0; i < cpu_num; i++) {
        cpu_list[i] = make_empty_list();
    }
    
    // initialise id arrays
    set_initial_id(new_id, cpu_num);
    set_initial_id(old_id, cpu_num);

    more_process = !is_empty_list(incoming);

    // check if there is still process to add
    while (more_process) {
        // check if it is time to allocate incoming process
        while (!is_empty_list(incoming) && curr_time == 
        get_head(incoming).arrive) {
            process_t new = dequeue(incoming);
            // create subprocess
            if (new.sub == -1 ) {
                insert_at_head(super_process, new);
                sub_num = create_subprocess(cpu_list, incoming, &new, cpu_num, 
                is_challenge);
                if (is_challenge) {
                    process_num -= (sub_num - 1);
                }
                else {
                    process_num += 1;
                }
            }

            // add a new process to cpu with shortest remaining time
            else {
                for (int i = 0; i < cpu_num; i++) {
                    if (remain_time_cpu(cpu_list[i])==0) {
                        has_free = 1;
                    }
                }
                if (is_challenge && !has_free && new.remain < LONGTIME) {
                    insert_at_foot(small_process, new);
                }
            
                else {
                    process_num += 1;

                

                    // find the cpu with the shortest remaining time and update it
                    insert_to = shortest_remaining(cpu_list, cpu_num);
                    new.cpu = insert_to;
                    add_to_running(cpu_list[insert_to], &new, insert_to);
                }
            }
        }

        //check interrupt for each cpu and report them
        for (int i = 0; i < cpu_num; i++) {
            if (!is_empty_list(cpu_list[i])) {
                new_id[i] = get_head(cpu_list[i]).pid;
                if (new_id[i]!=old_id[i]) {
                    display = get_head(cpu_list[i]);
                    report_interrupt(&display, curr_time);
                }
            }
        }

        curr_time += 1;

        // check if any program has finished
        for (int i = 0; i < cpu_num; i++) {
            if (!is_empty_list(cpu_list[i])) {
                cpu_list[i]->head->process.remain -= 1;
                // the program has finished
                if (!cpu_list[i]->head->process.remain) {
                    display = dequeue(cpu_list[i]);

                    // a sub_process finishes
                    if (display.sub == 1) {
                        is_sub = 1;
                        // check whether all other sub_process has finished
                        all_finish = check_super(super_process, &display, i);
                        if (all_finish) {
                            display = get_process(super_process, display.pid);
                            is_sub = 0;
                        }
                    }
                    // a non-sub_process finishes and add to report list.
                    if (is_sub == 0 || all_finish != 0) {
                        turnaround = curr_time - display.arrive;
                        total_turnaround += turnaround;
                        overhead = turnaround/display.time;
                        total_overhead += overhead;
                        if (overhead > max_overhead) {
                            max_overhead = overhead;
                        }
                        
                        insert_at_foot(finish_process, display);



                    }
                }
            }
            all_finish = 0;
            is_sub = 0;
            
        }

        for (int i = 0; i < cpu_num; i++)  {
            remaining += count_process(cpu_list[i]);
        }
        remaining += count_process(super_process);
        remaining += just_arrive(incoming, curr_time);

        if (!is_empty_list(finish_process)) {
            node_t *curr = finish_process->head;
            while (curr) {
                process_t report = dequeue(finish_process);
                report_finish(&report, curr_time, remaining);
                curr = curr->next;
            }
        }

        remaining = 0;

        // update old id;
        for (int i = 0; i < cpu_num; i++) {
            old_id[i] = new_id[i];
        }
        has_free = 0;

        while (is_empty_list(incoming) && !is_empty_list(small_process)) {
            process_t small = dequeue(small_process);
            insert_to = shortest_remaining(cpu_list, cpu_num);
            small.cpu = insert_to;
            add_to_running(cpu_list[insert_to], &small, insert_to);
        }
        more_process = update_remaining(cpu_list, cpu_num, incoming);
        if (!is_empty_list(small_process)) {
            more_process = 1;
        }
        

    }
    ave_turnaround = total_turnaround/process_num;
    ave_overhead = total_overhead/process_num;

    printf("Turnaround time %d\n", math_ceil(ave_turnaround));
    printf("Time overhead %.3g %.3g\n", round_2_decimal(max_overhead), 
    round_2_decimal(ave_overhead));
    printf("Makespan %d\n", curr_time);

    return curr_time;
}

// find the cpu with the shortest remaining time
int shortest_remaining(list_t *cpu_list[], int cpu_num) {
    int shortest_num = 0;
    int shortest_remain = MAXINT;
    int one_remain;

    // try to find a free cpu;
    for (int i = 0; i < cpu_num; i++) {
        if (is_empty_list(cpu_list[i])) {
            return i;
        }
    }

    // find a cpu with shortest remaining time
    for (int i = 0; i < cpu_num; i++) {
        one_remain = remain_time_cpu(cpu_list[i]);
        if (one_remain < shortest_remain) {
            shortest_remain = one_remain;
            shortest_num = i;
        }
    }
    return shortest_num;
}

// intiaialise id array to -1
void set_initial_id(int *id, int cpu_num) {
    for (int i = 0; i < cpu_num; i++) {
        id[i] = -1;
    }
}
// create sub_process for parallelisable process 
int create_subprocess(list_t **cpu_list, list_t *incoming, process_t *super, int cpu_num,
int is_challenge) {
    int sub_num = 0;
    double total_time = (double) super->time;
    // calculate the number of required sub_process
    if (super->time <= cpu_num) {
        sub_num = super->time;
    }
    else {
        sub_num = cpu_num;
    }

    process_t sub_list[sub_num];
    

    // calculate the execution time of sub_process
    int exec_time = math_ceil(total_time/sub_num + 1);

    // fill in information for sub_process
    for (int i = 0; i < sub_num; i++) {
        sub_list[i].arrive = super->arrive;
        sub_list[i].pid = super->pid;
        sub_list[i].remain = exec_time;
        sub_list[i].sub = 1;
        sub_list[i].sub_process = NULL;
        sub_list[i].time = exec_time;
        sub_list[i].sub_id = i;
    }

    if (cpu_num == 2) {
        for (int i = 0; i < 2; i++) {
            sub_list[i].cpu = i;
            add_to_running(cpu_list[i], &sub_list[i], cpu_num);
            insert_at_foot(super->sub_process, sub_list[i]);
        }
    }
    else {
         int remain[cpu_num], order[cpu_num];
         
         for (int i = 0; i < cpu_num; i++) {
             remain[i] = remain_time_cpu(cpu_list[i]);
             order[i] = i;
         }
         insertion_sort_with_order(remain, order, cpu_num);
         for (int i = 0; i < sub_num; i++) {
             sub_list[i].cpu = order[i];
             add_to_running(cpu_list[order[i]], &sub_list[i], cpu_num);
             insert_at_foot(super->sub_process, sub_list[i]);
         }

         
    }

   

    // insert sub_process to incoming list and parent
    if (is_challenge) {
        for (int i = 0; i < sub_num; i++) {
            insert_at_head(incoming, sub_list[sub_num-i-1]);
            // insert_at_foot(super->sub_process, sub_list[i]);
        }   
    }
    
     

    return sub_num;
}



// check whether all other sub_process has finished
int check_super(list_t *list, process_t *sub, int curr_cpu)  {
    node_t *curr;
    assert(list!=NULL);
    curr = list->head;
    int all_finish = 1;
     
    // locate current super process
    while (curr->process.pid != sub->pid) {
        curr=curr->next;
    }

    // check whether all sub_process has finished
    node_t *sub_curr = curr->process.sub_process->head;
    
    
    
    while (sub_curr) {
        if (sub_curr->process.cpu == curr_cpu) {
            sub_curr->process.remain = 0;
        }
        if (sub_curr->process.remain != 0) {
            all_finish = 0;
        }

        sub_curr = sub_curr->next;
    }
    
    return all_finish;
}

// get one process with required pid in a linked list
process_t get_process(list_t *list, int id) {
    node_t *curr;
    assert(list!=NULL);
    curr = list->head;
    while (curr->process.pid != id) {
        curr=curr->next;
    }
    // mark it as a finished super program
    curr->process.sub = -2;
    
    return curr->process;

}





// add a new process to a cpu, according to its remaining time and id
void add_to_running(list_t *running, process_t *new, int cpu_num) {
    // the cpu is available now
    if (is_empty_list(running)) {
        insert_at_head(running, *new);
        return; 
    }

    node_t *curr;
    node_t *old;
    int inserted = 0;

    curr = running->head;

    // find its position according to remaining time
    while (curr) {
        process_t one = curr->process;
        
        // find suitable solution
        if (new->remain < one.remain || (new->remain == one.remain && 
        new->pid < one.pid)) {
            inserted = 1;

            node_t *new_node;
            new_node = (node_t*)malloc(sizeof(*new_node));
            new_node->process = *new;
            
            // add to the linked list
            if (curr->prev!=NULL) {
                old = curr->prev;
                old->next = new_node;
                new_node->next = curr;
                curr->prev = new_node;
            }
            // the remaining time of new process is the shortest
            else {
                running->head = new_node;
                new_node->prev = NULL;
                new_node->next = curr;
                curr->prev = new_node;
            }
            break;   
        }
        curr=curr->next;
    }
    
    // new process's remaining is the longest
    if (inserted == 0) {
        insert_at_foot(running, *new);
    }
    return;
}

// report an interrupt for certain cpu
void report_interrupt(process_t *process, int curr) {
    if (process->sub == 1) {
        printf("%d,%s,pid=%d.%d,remaining_time=%d,cpu=%d\n", curr, "RUNNING", 
        process->pid, process->sub_id, process->remain, process->cpu);
    }
    else {
        printf("%d,%s,pid=%d,remaining_time=%d,cpu=%d\n", curr, "RUNNING",
        process->pid, process->remain, process->cpu);
    }
}

// report a process's finish
void report_finish(process_t *process, int curr, int remaining) {
    if (process->sub < 1) {
        printf("%d,%s,pid=%d,proc_remaining=%d\n", curr, "FINISHED", 
        process->pid, remaining);
    }
}


// check if there is any unfinished program
int update_remaining(list_t *cpu_list[], int cpu_num, list_t *incoming) {
    // check whether there is unallocated program
    if (!is_empty_list(incoming)) {
        return 1;
    }
    // check whether there is running program
    for (int i = 0; i < cpu_num; i++) {
        if (!is_empty_list(cpu_list[i])) {
            return 1;
        }
    }
    return 0;
}

// compute math ceil int value of a given double
int 
math_ceil(double num_1) {
    int num_2;
    num_2 = num_1;
    if (num_2 == num_1) {
        return num_2;
    }
    else {
        return num_2 + 1;
    }
}

// round a number to its 2 demical places
double 
round_2_decimal(double num) {
    num *= HUNDRED;
    int n = (int)(num < 0 ? (num - 0.5) : (num + 0.5));
    return n/HUNDRED;
}

// insertion sort and update order
void insertion_sort_with_order(int *remain, int *order, int cpu_num) {
    int i, j, num;
    for (i = 1; i < cpu_num; i++) {
        j = i - 1;
        while (j>0 && (remain[j+1] < remain[j])) {
            num = remain[j+1];
            remain[j] = remain[j+1];
            remain[j+1] = num;
            num = order[j+1];
            order[j] = order[j+1];
            order[j+1] = num;
        }
    }
}

// count at certain time how many new process has just arrive
int just_arrive(list_t *incoming, int curr_time) {
    int num = 0;
    if (is_empty_list(incoming)) {
        return 0;
    }
    node_t *curr = incoming->head;
    while(curr) {
        if (curr->process.arrive == curr_time) {
            num += 1;
        }
        curr = curr->next;
    }

    return num;
}
    

    
    
    
    
    


