/* Double linked list module */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "list.h"

// create an empty list
list_t
*make_empty_list(void) {
    list_t *list;
    list = (list_t*)malloc(sizeof(*list));
    assert(list!=NULL);
    list->head = list->foot = NULL;
    return list;
}

// determine whether it is a empty list
int
is_empty_list(list_t *list) {
    assert(list!=NULL);
    return list->head==NULL;
}

// free the list
void
free_list(list_t *list) {
    node_t *curr, *prev;
    assert(list!=NULL);
    curr = list->head;
    while (curr) {
        prev = curr;
        curr = curr->next;
        free(prev);
    }
    free(list);
}

// count the number of running (main) process in a list
int count_process(list_t *list) {
    int n=0;
    int not_count = 0;
    node_t *curr;
    assert(list!=NULL);
    curr = list->head;
    while (curr) {
        not_count = 1;
        // we don't want to count sub process
        if (curr->process.sub == 1) {
            not_count = 0;
        }
        // we don't want to count finished super process
        else if (curr->process.sub == -2) {
            not_count = 0;
        }
        if (not_count) {
            n+=1;
        }

        curr = curr->next;
        
    }
    return n;
}

// insert a new node at the start of linked list
list_t
*insert_at_head(list_t *list, process_t value) {
    node_t *new;
    new = (node_t*)malloc(sizeof(*new));
    assert(list!=NULL && new!=NULL);
    new->process = value;
    new->next = list->head;
    new->prev = NULL;

    // insert into empty list
    if (list->foot==NULL) {
        list->head = list->foot = new;
    }
    else {
        list->head->prev = new;
        list->head = new;
    }

    return list;
}

// insert a new node at the end of linked list
list_t
*insert_at_foot(list_t *list, process_t value) {
    node_t *new;
    new = (node_t*)malloc(sizeof(*new));
    assert(list!=NULL && new!=NULL);
    new->process = value;
    new->next = NULL;
    new->prev = list->foot;

    // insert into empty list
    if (list->foot==NULL) {
        list->head = list->foot = new;
    } else {
        list->foot->next = new;
        list->foot = new;
    }
    return list;
}

// remove the first element in the linked list and 
process_t dequeue(list_t *list) {
    assert (list!=NULL && list->head!=NULL);
    node_t *old_head = list->head;
    
    process_t process = old_head->process;
    // deleting the last element
    if (list->head->next == NULL) {
        list->head = NULL;
        list->foot = NULL;
    }
    else {
        list->head = old_head->next;
        list->head->prev = NULL;
    }
    free(old_head); 
    return process;
}

// get the first element in the linked list
process_t
get_head(list_t *list) {
    assert(list!=NULL && list->head!=NULL);
    return list->head->process;
}

// get the first element in the linked list
process_t
get_foot(list_t *list) {
    assert (list!=NULL && list->foot!=NULL);
    return list->foot->process;
}

// get the total remaining time of processes assigned in certain cpu
int remain_time_cpu(list_t *cpu) {
    int remain = 0;
    node_t *curr;
    curr = cpu->head;
    
    while (curr) {
        remain += curr->process.remain;
        curr=curr->next;
        
    }
    return remain;
}
