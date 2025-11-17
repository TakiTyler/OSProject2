#include <stdint.h>

typedef struct command_data{
    char specific_command[10];
    char name[50];
    uint32_t salary; // if a command doesn't require a salary, will be set to 0
    int priority;
} command;

typedef struct command_node{
    command data;
    struct command_node *next;
} commandNode;

extern commandNode *command_list_head;

// gives the current timestamp
long long current_timestamp();

// breaks down the commands.txt into operatable commands
void parse_commands();

// executes the command given
void *execute_command(void *command);

// logs events that happen, given a message & priority
void log_event(const char *message, int priority);

// Reverses a linked list
void reverse_list();

// makes a thread wait until its turn
void wait_turn();