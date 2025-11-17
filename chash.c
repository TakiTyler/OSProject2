#include "chash.h"
#include "hashtable.h"
#include "rwlock.h"
#include "sys/time.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define MAX_LINE_SIZE 100
commandNode *command_list_head = NULL;

rwlock_t rwlock;
pthread_mutex_t log_mutex;

int current_turn = 1;
pthread_mutex_t mutex_turn;
pthread_cond_t cond_turn;

// GIVEN BY PROF
long long current_timestamp() {  
    struct timeval te;  
    gettimeofday(&te, NULL); // get current time  
    long long microseconds = (te.tv_sec * 1000000) + te.tv_usec; // calculate milliseconds  
    return microseconds;  
}

void wait_turn(int priority){
    // wait until the command's proper turn
    pthread_mutex_lock(&mutex_turn);
    while(priority != current_turn){
        pthread_cond_wait(&cond_turn, &mutex_turn);
    }
    // update global counter
    current_turn++;
    pthread_cond_broadcast(&cond_turn);
    pthread_mutex_unlock(&mutex_turn);
}

void reverse_list(){
    commandNode *current = command_list_head;
    commandNode *previous;
    commandNode *next;

    while(current != NULL){
        next = current->next;

        // reverse the linking
        current->next = previous;
        previous = current;
        current = next;
    }

    // at this point, previous is the new head
    command_list_head = previous;
       
}

void parse_commands(){
    FILE *filePtr = fopen("commands.txt", "r");

    // in case we don't open the file
    if(filePtr == NULL){
        fprintf(stdout, "ERROR: Unable to open commands.txt");
        return;
    }

    char fileLine[MAX_LINE_SIZE];
    char stringDupeBuffer[MAX_LINE_SIZE];
    char *endPtr;
    char *token;

    while(fgets(fileLine, MAX_LINE_SIZE, filePtr) != NULL){
        // at this point, we've gotten a line
        // we will assume that a line will always be under 100 lines
        fprintf(stdout, "\nWhole line = %s", fileLine);

        // allocate memory for the new node
        commandNode *newNode = (commandNode*)malloc(sizeof(commandNode));
        
        // turn the last line into a string end character
        fileLine[strcspn(fileLine, "\n")] = '\0';

        strcpy(stringDupeBuffer, fileLine);
        token = strtok(stringDupeBuffer, ",");
        // token initially is the command that we need to look at
        // copy command
        strcpy(newNode->data.specific_command, token);

        if(strcmp(token, "insert") == 0){
            fprintf(stdout, "--Insert--\n");
            // expects 3 more tokens: name, salary, priority

            // name
            token = strtok(NULL, ",");
            strcpy(newNode->data.name, token);   

            // salary
            token = strtok(NULL, ",");
            newNode->data.salary = strtoul(token, &endPtr, 10); // unsigned long longs

            // priority
            token = strtok(NULL, ",");
            newNode->data.priority = strtoul(token, &endPtr, 10);

        }
        else if(strcmp(token, "delete") == 0){
            // expects 2 more tokens: name, priority
            fprintf(stdout, "--Delete--\n");

            // name
            token = strtok(NULL, ",");
            strcpy(newNode->data.name, token);   
            
            // priority
            token = strtok(NULL, ",");
            newNode->data.priority = strtol(token, &endPtr, 10);

            // setting salary to 0
            newNode->data.salary = 0;
        }
        else if(strcmp(token, "search") == 0){
            // expects 2 more tokens: name, priority
            fprintf(stdout, "--Search--\n");

            // name
            token = strtok(NULL, ",");
            strcpy(newNode->data.name, token);   
            
            // priority
            token = strtok(NULL, ",");
            newNode->data.priority = strtol(token, &endPtr, 10);

            // setting salary to 0
            newNode->data.salary = 0;
        }
        else if(strcmp(token, "print") == 0){
            // expects 1 more token: priority
            fprintf(stdout, "--Print--\n");

            // priority
            token = strtok(NULL, ",");
            newNode->data.priority = strtol(token, &endPtr, 10);

            // setting name & salary to 0
            newNode->data.name[0] = '\0';
            newNode->data.salary = 0;
        }

        newNode->next = command_list_head;
        command_list_head = newNode;

        fprintf(stdout, "End of line\n");
    }

    // right now, the commands are NOT stored in order
    // it's inefficient, but lets reverse the linked list AFTER creating it
        // yes, not great BUT it works
    reverse_list();

    commandNode *temp = command_list_head;
    
    while(temp != NULL){
        fprintf(stdout, "\n--NEW NODE--\n");
        fprintf(stdout, "Command = %s\n", temp->data.specific_command);
        fprintf(stdout, "Name = %s\n", temp->data.name);
        fprintf(stdout, "Salary = %d\n", temp->data.salary);
        fprintf(stdout, "Priority = %d\n", temp->data.priority);
        temp = temp->next;
    }

    fclose(filePtr);

    return;
}

void *execute_command(void *command_arg){
    command *passed_command = (command *)command_arg;
    char *log_string = malloc(MAX_LINE_SIZE+1); // assume 100 charater limit
    uint32_t command_hash;

    log_event("WAITING FOR MY TURN", passed_command->priority);

    command_hash = jenkins_hash(passed_command->name);

    if(strcmp(passed_command->specific_command, "insert") == 0){
        
        // create the output string corresponding to insert (not actually printing yet)
        snprintf(log_string, MAX_LINE_SIZE+1, "INSERT,%u,%s,%u", command_hash, passed_command->name, passed_command->salary);

        // wait until the command's proper turn
        wait_turn(passed_command->priority);

        // run the command
        rwlock_acquire_writelock(&rwlock);
        log_event("AWAKENED FOR WORK", passed_command->priority);
        log_event("WRITE LOCK ACQUIRED", passed_command->priority);
        log_event(log_string, passed_command->priority); // log command
        insert(passed_command->name, command_hash, passed_command->salary);
        rwlock_release_writelock(&rwlock);
        log_event("WRITE LOCK RELEASED", passed_command->priority);
    }
    else if(strcmp(passed_command->specific_command, "delete") == 0){

        // create the output string corresponding to delete (not actually printing yet)
        snprintf(log_string, MAX_LINE_SIZE+1, "DELETE,%u,%s", command_hash, passed_command->name);

        // wait until the command's proper turn
        wait_turn(passed_command->priority);

        // run the command
        rwlock_acquire_writelock(&rwlock);
        log_event("AWAKENED FOR WORK", passed_command->priority);
        log_event("WRITE LOCK ACQUIRED", passed_command->priority);
        log_event(log_string, passed_command->priority); // log command
        delete(passed_command->name, command_hash);
        rwlock_release_writelock(&rwlock);
        log_event("WRITE LOCK RELEASED", passed_command->priority);
    }
    else if(strcmp(passed_command->specific_command, "update") == 0){

        // create the output string corresponding to update (not actually printing yet)
        snprintf(log_string, MAX_LINE_SIZE+1, "UPDATE,%u,%s,%u", command_hash, passed_command->name, passed_command->salary);
        
        // wait until the command's proper turn
        wait_turn(passed_command->priority);

        // run the command
        rwlock_acquire_writelock(&rwlock);
        log_event("AWAKENED FOR WORK", passed_command->priority);
        log_event("WRITE LOCK ACQUIRED", passed_command->priority);
        log_event(log_string, passed_command->priority); // log command
        updateSalary(passed_command->name, command_hash, passed_command->salary);
        rwlock_release_writelock(&rwlock);
        log_event("WRITE LOCK RELEASED", passed_command->priority);
    }
    else if(strcmp(passed_command->specific_command, "search") == 0){
        
        // create the output string corresponding to search (not actually printing yet)
        snprintf(log_string, MAX_LINE_SIZE+1, "SEARCH,%u,%s", command_hash, passed_command->name);

        // wait until the command's proper turn
        wait_turn(passed_command->priority);

        // run the command
        rwlock_acquire_readlock(&rwlock);
        log_event("AWAKENED FOR WORK", passed_command->priority);
        log_event("READ LOCK ACQUIRED", passed_command->priority);
        log_event(log_string, passed_command->priority); // log command
        search(passed_command->name, command_hash);
        rwlock_release_readlock(&rwlock);
        log_event("READ LOCK RELEASED", passed_command->priority);
    }
    else if(strcmp(passed_command->specific_command, "print") == 0){
        
        // create the output string corresponding to print (not actually printing yet)
        snprintf(log_string, MAX_LINE_SIZE+1, "PRINT");
        
        // wait until the command's proper turn
        wait_turn(passed_command->priority);
        
        // run the command
        rwlock_acquire_readlock(&rwlock);
        log_event("AWAKENED FOR WORK", passed_command->priority);
        log_event("READ LOCK ACQUIRED", passed_command->priority);
        log_event(log_string, passed_command->priority); // log command
        print(passed_command->priority);
        rwlock_release_readlock(&rwlock);
        log_event("READ LOCK RELEASED", passed_command->priority);
    }
}

void log_event(const char *message, int priority){
    pthread_mutex_lock(&log_mutex);

    FILE *log_file = fopen("hash.log", "a"); // append mode to not erase previous data

    if(log_file != NULL){
        long long timestamp = current_timestamp();

        //// sample log output ////
        /*
        <timestamp>: THREAD <priority>,<command and parameters>
        1721428978841092: THREAD 0 INSERT,2569965317,Hideo Kojima,45000   
        1721428978841096: THREAD 0 WRITE LOCK ACQUIRED   
        1721428978841098: THREAD 0 WRITE LOCK RELEASED
        */

        fprintf(log_file, "%lld: THREAD %d %s\n", timestamp, priority, message);

        fclose(log_file);
    }
    else{
        // error handling in case the file doesn't exist
        fprintf(stdout, "ERROR: output.txt not found");
    }

    pthread_mutex_unlock(&log_mutex);
}

int main(){

    rwlock_init(&rwlock);

    pthread_mutex_init(&log_mutex, NULL);
    pthread_mutex_init(&mutex_turn, NULL);
    pthread_cond_init(&cond_turn, NULL);

    parse_commands();

    // separating parse command outputs
    fprintf(stdout, "\n\n\n");

    pthread_t thread_id[100];
    int thread_counter = 0;

    commandNode *temp = command_list_head;

    while(temp != NULL){
        pthread_create(&thread_id[thread_counter], NULL, execute_command, (void *)&temp->data);
        thread_counter++;
        temp = temp->next;
    }

    for(int i = 0; i < thread_counter; i++){
        pthread_join(thread_id[i], NULL);
    }

    print();

    return 0;
}