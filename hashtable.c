#include "hashtable.h"
#include "sys/time.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

hashRecord *hash_table_head = NULL;

// provided hash function
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

void insert(const char *name, uint32_t salary){

    hashRecord *temp = search(name);
    
    if(temp == NULL){
        // insert at the head
        hashRecord *newRecord = (hashRecord *)malloc(sizeof(hashRecord));

        newRecord->hash = jenkins_hash(name);
        strncpy(newRecord->name, name, 49);
        newRecord->name[49] = '\0'; // guarantee we end with a null termination
        newRecord->salary = salary;
        newRecord->next = hash_table_head;

        hash_table_head = newRecord;

        // used for debugging
        fprintf(stdout, "INSERT: Inserted %s, %u\n", name, salary);
    }
    else{
        // already exists, ERROR
        fprintf(stdout, "INSERT: Duplicate entry %u, failed to insert.\n", temp->hash);
    }

    return;
}

void delete(const char *name){

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

            fprintf(stdout, "DELETE: Deleted record for %s\n", name);

            free(current);

            return;
        }

        previous = current;
        current = current->next;
    }

    // if current is null, the value doesn't exist
    fprintf(stdout, "DELETE: Missing entry, %u not deleted.\n", jenkins_hash(name));

    return;
}

void updateSalary(const char *name, uint32_t new_salary){

    hashRecord *temp = search(name);
    
    if(temp == NULL){
        // doesnt exist, ERROR
        fprintf(stdout, "UPDATE: Missing entry, update failed for %u\n\n", jenkins_hash(name));
    }
    else{
        // update
        uint32_t old_salary = temp->salary;
        temp->salary = new_salary;

        // used for debugging
        fprintf(stdout, "UPDATE: Updated record for %u, from %u to %u\n", jenkins_hash(name), old_salary, new_salary);
    }

    return;
}

hashRecord *search(const char *name){

    // log_event("Searching");

    hashRecord *current = hash_table_head;

    uint32_t givenKey = jenkins_hash(name);
    uint32_t currentKey;

    while(current != NULL){
        // get current key, compare current and given

        // if equal, return the hashRecord
        currentKey = current->hash;

        if(givenKey == currentKey){

            fprintf(stdout, "SEARCH: Found %s, %u\n", name, jenkins_hash(name));

            return current; // returns the pointer to the node
        }

        current = current->next;
    }

    fprintf(stdout, "SEARCH: Unable to find %s\n", name);
    return current; // should return a null pointer
}

void print(){

    // we need to SORT by the hash. since these are numbers, simple comparisons will do

    fprintf(stdout, "Current Database:\n");

    if(hash_table_head == NULL){
        fprintf(stdout, "EMPTY DATABASE\n");
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
        fprintf(stdout, "%u ", currentSorted->data.hash);
        fprintf(stdout, "%s ", currentSorted->data.name);
        fprintf(stdout, "%u\n", currentSorted->data.salary);

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

        // loop until we've found the next node
        while(current->next != NULL && hashNode->hash > current->next->data.hash){
            current = current->next;
        }

        newNode->next = current->next;
        current->next = newNode;
    }

    return sorted_head;
}