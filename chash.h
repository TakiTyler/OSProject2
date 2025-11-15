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

long long current_timestamp();

void parse_commands();

void *execute_command(void *command);

void log_event(const char *message, int priority);