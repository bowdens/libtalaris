#ifndef __LTALARIS
#define __LTALARIS

#include "uthash.h"

#define LT_CALL_FAILED -99
#define LT_COMMAND_NOT_FOUND -98

#define LT_HIDE 00
#define LT_HELP 01
#define LT_SPEC 02
#define LT_EXEC 04
#define LT_UNIV LT_HELP | LT_SPEC | LT_EXEC

#define LT_IS_HELP(a)(a & LT_HELP)
#define LT_IS_SPEC(a)(a & LT_SPEC)
#define LT_IS_EXEC(a)(a & LT_EXEC)
#define LT_IS_SHOW(a)(a & (LT_HELP | LT_SPEC))

typedef char lt_state;
/*
 * 1st bit: show in help
 * 2nd bit: show in specific help
 * 3rd bit: execute command
 * 00 -> 07
 */

typedef enum lt_verbosity {
    lt_normal,
    lt_warning,
    lt_verbose
} lt_verbosity;

typedef struct lt_commander LT_Commander;

typedef int(*lt_callback)(int, char**, LT_Commander*);

typedef struct lt_command {
    char *key;
    char *help;
    char *help_extended;
    lt_state state;
    lt_callback callback;
    UT_hash_handle hh;
} LT_Command;

typedef struct lt_commander {
    LT_Command *commands;
    lt_verbosity verbosity;
    lt_callback unfound;
    int argc;
    char **argv;
    char *prompt;
} LT_Commander;

LT_Commander *lt_create_commander(void);
int lt_add_commands(LT_Commander*, LT_Command*);
int lt_add_command(LT_Commander*, char*, char*, char*, lt_callback callback);
int lt_remove_command(LT_Commander*, char*);
LT_Command* lt_get_command(LT_Commander*, char*);
int lt_call(LT_Commander*, char*);
int lt_input(LT_Commander*, char **matching_commands);
int lt_cleanup(LT_Commander*);
void lt_print_commander(LT_Commander*);

#endif
