#include <stdint.h>
#include <stdbool.h>

// structure for a linked list
typedef struct hash_struct{
    uint32_t hash;
    char name[50];
    uint32_t salary;
    struct hash_struct *next; // next pointer
} hashRecord;

extern hashRecord *hash_table_head;

// for sorting the nodes by hash when we print
typedef struct sorted_hashes{
    hashRecord data;
    struct sorted_hashes *next;
} sortedRecord;

// structure for the hash
uint32_t jenkins_hash(const char *key);

// structure for insert function
void insert(const char *name, uint32_t hash, uint32_t salary);

// structure for delete function
void delete(const char *name, uint32_t hash);

// structure for updating
void updateSalary(const char *name, uint32_t hash, uint32_t new_salary);

// structure for search function
hashRecord *search(const char *name, uint32_t hash, bool log);

// structure for print function
void print();

// for printing
sortedRecord *insertion_sort(sortedRecord *sorted_head, hashRecord *newNode);