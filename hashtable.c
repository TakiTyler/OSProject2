#include "hashtable.h"
#include "chash.h"
#include "sys/time.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

extern rwlock_t rwlock;

hashRecord *hash_table_head = NULL;

long long current_timestamp(){
    struct timeval te;
    gettimeofday(&te, NULL); // get current time
    long long microseconds = (te.tv_sec * 1000000) + te.tv_usec; // calculate milliseconds
    return microseconds;
}

uint32_t jenkins_hash(const char *key){
    size_t length = strlen(key);
    size_t i = 0;
    uint32_t hash = 0;
    const uint8_t* ukey = (const uint8_t*) key;

    while(i != length){
        hash += ukey[i++];
        hash += hash << 10;
        hash ^= hash >> 6;
    }

    hash += hash << 3;
    hash ^= hash >> 11;
    hash += hash << 15;

    return hash;
}

void insert(const char *name, uint32_t salary, int priority){

    rwlock_acquire_writelock(&rwlock);

    log_event("WRITE LOCK ACQUIRED", priority);

    hashRecord *temp = search(name, priority);
    
    if(temp == NULL){
        // insert at the head
        hashRecord *newRecord = (hashRecord *)malloc(sizeof(hashRecord));

        newRecord->hash = jenkins_hash(name);
        strncpy(newRecord->name, name, 49);
        newRecord->name[49] = '\0'; // guarantee we end with a null termination
        newRecord->salary = salary;
        newRecord->next = hash_table_head;

        hash_table_head = newRecord;

        printf("INSERT: Inserted %s, %u\n", name, salary);

    }
    else{
        // already exists, update
        uint32_t previousSalary = temp->salary;
        temp->salary = salary;

        printf("UPDATE: Updated record %u, name %s's salary from %u to %u\n", temp->hash, temp->name, previousSalary, temp->salary);
    }

    rwlock_release_writelock(&rwlock);

    log_event("WRITE LOCK RELEASED", priority);

    return;
}

void delete(const char *name, int priority){

    rwlock_acquire_writelock(&rwlock);

    log_event("WRITE LOCK ACQUIRED", priority);

    hashRecord *previous;
    hashRecord *current = hash_table_head;

    uint32_t givenKey = jenkins_hash(name);
    uint32_t currentKey;

    while(current != NULL){

        currentKey = current->hash;

        if(givenKey == currentKey){
            // value exists

            // if current is the head
            if(previous == NULL){
                hash_table_head = current->next;
            }
            else{ // current is NOT the head
                previous->next = current->next;
            }

            printf("DELETE: Deleted record for %s\n", name);

            rwlock_release_writelock(&rwlock);

            free(current);

            return;
        }

        previous = current;
        current = current->next;
    }

    // if current is null, the value doesn't exist
    printf("DELETE: Record for %s not found.\n", name);

    rwlock_release_writelock(&rwlock);

    log_event("WRITE LOCK RELEASED", priority);

    return;
}

hashRecord *search(const char *name, int priority){

    // log_event("Searching");

    hashRecord *current = hash_table_head;

    uint32_t givenKey = jenkins_hash(name);
    uint32_t currentKey;

    while(current != NULL){
        // get current key, compare current and given

        // if equal, return the hashRecord
        currentKey = current->hash;

        if(givenKey == currentKey){
            return current; // returns the pointer to the node
        }

        current = current->next;
    }

    // log_event("Done searching");

    return current; // should return a null pointer
}

void print(int priority){

    rwlock_acquire_readlock(&rwlock);

    log_event("READ LOCK ACQUIRED", priority);

    // we need to SORT by the hash. since these are numbers, simple comparisons will do

    if(hash_table_head == NULL){
        printf("Nothing to print, empty hash");
        log_event("READ LOCK RELEASED", priority);
        return;
    }

    sortedRecord *sorted_head = NULL;
    hashRecord *current = hash_table_head;
    hashRecord *next = NULL;

    while(current != NULL){
        next = current->next;

        sorted_head = insertion_sort(sorted_head, current);

        current = next;
    }

    // loop through sorted
    sortedRecord *currentSorted = sorted_head;

    // print sorted
    while(currentSorted != NULL){
        printf("%u", currentSorted->data.hash);
        printf("%s", currentSorted->data.name);
        printf("%u", currentSorted->data.salary);

        currentSorted = currentSorted->next;
    }

    // reset to head
    currentSorted = sorted_head;

    // free all the memory of the sorted
    while(currentSorted != NULL){
        sortedRecord *temp = currentSorted->next;
        free(currentSorted);
        currentSorted = temp;
    }

    rwlock_release_readlock(&rwlock);

    log_event("READ LOCK RELEASED", priority);

    return; 
}

sortedRecord *insertion_sort(sortedRecord *sorted_head, hashRecord *hashNode){

    sortedRecord *newNode = (sortedRecord *)malloc(sizeof(sortedRecord));
    newNode->data = *hashNode;

    // check for head value first
    if(sorted_head == NULL || hashNode->hash < sorted_head->data.hash){
        newNode->next = sorted_head;
        sorted_head = newNode;
    }
    else{
        sortedRecord *current = sorted_head;

        // 1 2 3 5
        // 4

        // loop until we've found the next node
        while(current->next != NULL && hashNode->hash > current->next->data.hash){
            current = current->next;
        }

        newNode->next = current->next;
        current->next = newNode;
    }

    return sorted_head;
}