#include "libtalaris.h"
#include "uthash.h"
#include "wordsplit.h"
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>

char **matching_commands = NULL;

int lt_help(int argc, char **argv, LT_Commander *com) {
    assert(com != NULL);
    if(argc == 1) {
        //The command 'help' only was called
        LT_Command *s, *tmp;
        HASH_ITER(hh, com->commands, s, tmp) {
            if(LT_IS_HELP(s->state)) {
                printf("%s", s->key);
                if(s->help) {
                    printf("\t%s\n",s->help);
                } else {
                    printf("\n");
                }
            }
        }
    } else {
        // show extended help for each command in argv
        for(int i = 1; i < argc; i++) {
            LT_Command *c = lt_get_command(com, argv[i]);
            if(c == NULL || !(LT_IS_SPEC(c->state))) {
                printf("Could not find command %s\n", argv[i]);
            } else {
                printf("%s\t%s\n", c->key, c->help == NULL ? "This command has no help text" : c->help);
                if(c->help_extended) printf("\t%s\n", c->help_extended);
            }
        }
    }
    return 0;
}

int lt_exit(int argc, char **argv, LT_Commander *com) {
    //do nothing
    exit(0);
}

int lt_unfound(int argc, char **argv, LT_Commander *com) {
    printf("The command '%s' was not found. Try typing 'help' to see a list of full commands\n", argc > 0 ? argv[0] : "");
    return 0;
}


LT_Commander *lt_create_commander(void) {
    LT_Commander *com = malloc(sizeof(LT_Commander));
    assert(com);
    com->commands = NULL;
    com->verbosity = lt_normal;

    com->argc = 0;
    com->argv = NULL;
    com->prompt = "> ";

    com->unfound = lt_unfound;

    lt_add_command(com, "help", "Shows this help", "Usage: help [command]", lt_help);
    lt_add_command(com, "exit", "Exits the program", "Usage: exit", lt_exit);

    return com;
}

LT_Command *lt_get_command(LT_Commander *com, char *command) {
    if(com == NULL || command == NULL) return NULL;
    LT_Command *c = NULL;
    HASH_FIND_STR(com->commands, command, c);
    return c;
}

int add_command_to_commander(LT_Commander *com, LT_Command *command) {
    assert(com);
    assert(command);
    if(lt_get_command(com, command->key) != NULL) {
        if(com->verbosity >= lt_warning) fprintf(stderr, "Warning: Could not add command '%s' because it already exists in this commander\n", command->key);
        return 1;
    }

    HASH_ADD_KEYPTR(hh, com->commands, command->key, strlen(command->key), command);
    return 0;
}

int lt_add_command(LT_Commander *com, char *command, char *help, char *help_extended, int (*callback)(int, char**, LT_Commander *)) {
    /*
     * Add a command to the commander
     * Cannot add two of the same command
     */
    assert(com != NULL);

    if(command == NULL || command[0] == '\0') return 1;

    LT_Command *c = malloc(sizeof(LT_Command));
    c->key = strdup(command);
    assert(c->key);
    c->help = strdup(help);
    assert(c->help);
    c->help_extended = strdup(help_extended);
    c->callback = callback;
    c->state = LT_UNIV;

    int retval = add_command_to_commander(com, c);
    if(retval != 0) {
        free(c->key);
        free(c->help);
        free(c->help_extended);
        free(c);
    }
    return retval;
}

int lt_add_commands(LT_Commander *com, LT_Command *commands) {
    assert(com);
    int count = 0;
    for(int i = 0; commands[i].key != NULL; i++) {
        LT_Command *c = malloc(sizeof(LT_Command));
        c->key = strdup(commands[i].key);
        assert(c->key);
        c->help = strdup(commands[i].help);
        assert(c->help);
        c->help_extended = strdup(commands[i].help_extended);
        assert(c->help_extended);
        c->state = commands[i].state;
        c->callback = commands[i].callback;
        count += (add_command_to_commander(com, c) == 0);
    }
    return count;
}

void free_command(LT_Command *c) {
    free(c->help);
    free(c->help_extended);
    free(c->key);
    free(c);
}

int lt_remove_command(LT_Commander *com, char *command) {
    assert(com != NULL);
    LT_Command *to_delete = lt_get_command(com, command);
    if(to_delete == NULL) return 1;
    HASH_DEL(com->commands, to_delete);
    free_command(to_delete);
    return 1;
}

int lt_call(LT_Commander *com, char *str) {
    /*
     * Parses the arguments in string and
     * executes the appropriate callback
     */
    if(com == NULL) return LT_CALL_FAILED;
    // free the old commands
    for(int i = 0; i < com->argc; i++) {
        free(com->argv[i]);
    }
    free(com->argv);
    com->argv = NULL;
    com->argc = 0;
    if(str == NULL) return LT_CALL_FAILED;

    com->argc = ws_split(str, &com->argv);
    if(com->verbosity >= lt_verbose) {
        printf("Collected %d arguments. They are:\n", com->argc);
        for(int i = 0; i < com->argc; i++) printf("'%s'%s", com->argv[i], i == com->argc-1 ? "\n" : " ");
    }

    char *command = com->argv[0];
    LT_Command *c = lt_get_command(com,command);
    int retval;
    if(c && LT_IS_EXEC(c->state)) {
        if(c->callback == NULL) {
            if(com->verbosity >= lt_warning) fprintf(stderr, "Warning: Command '%s' has no callback\n", c->key);
            retval = LT_CALL_FAILED;
        } else {
            retval = c->callback(com->argc, com->argv, com);
        }
    } else {
        if(com->unfound != NULL) {
            com->unfound(com->argc, com->argv, com);
        }
        retval = LT_COMMAND_NOT_FOUND;
    }

    return retval;
}

char **generate_command_list(LT_Commander *com) {
    //TODO: move the command list from here into the LT_Commander struct
    int count = HASH_COUNT(com->commands);
    char **command_list = malloc(sizeof(char*) * (count+1));
    assert(command_list);
    int c = 0;
    LT_Command *s, *tmp;
    HASH_ITER(hh, com->commands, s, tmp) {
        if(LT_IS_SHOW(s->state)) {
            command_list[c++] = strdup(s->key);
        }
    }
    command_list[c] = NULL;
    return command_list;
}

char *command_generator(const char *text, int state) {
    static int list_index, len;
    char *command;

    if(!state) {
        list_index = 0;
        len = strlen(text);
    }
    while((command = matching_commands[list_index++])) {
        if(strncmp(command, text, len) == 0) {
            return strdup(command);
        }
    }
    return NULL;
}

char **command_completion(const char *text, int start, int end) {
    rl_attempted_completion_over = 1;
    return rl_completion_matches(text, command_generator);
}

int lt_input(LT_Commander *com, char **_matching_commands) {
    /*
     * Reads from stdin and executes lt_call
     */
    if(com == NULL) return LT_CALL_FAILED;

    char *str = NULL;

    if(_matching_commands == NULL) {
        char *tmp[] = {NULL};
        matching_commands = tmp;
    } else {
        matching_commands = _matching_commands;
    }

    rl_attempted_completion_function = command_completion;

    str = readline(com->prompt);
    if(str == NULL) {
        free(str);
        for(int i = 0; i < com->argc; i++) {
            free(com->argv[i]);
        }
        free(com->argv);
        com->argv = NULL;
        com->argc = 0;

        printf("\n");
        free(str);
        return LT_CALL_FAILED;
    }
    if(com->argv == NULL || strcmp(str, com->argv[0]) != 0) {
        add_history(str);
    }

    matching_commands = NULL;

    int retval = lt_call(com, str);
    free(str);
    return retval;
}

int lt_cleanup(LT_Commander *com) {
    /*
     * Free a commander
     */
    if(com == NULL) return 0;

    for(int i = 0; i < com->argc; i++) {
        free(com->argv[i]);
    }

    int count = 0;
    int total = HASH_COUNT(com->commands);
    LT_Command *tmp, *s;
    HASH_ITER(hh, com->commands, s, tmp) {
        HASH_DEL(com->commands, s);
        free(s->help);
        free(s->help_extended);
        free(s->key);
        free(s);
        count++;
    }

    free(com);

    return 0;
}

void lt_print_commander(LT_Commander *com) {
    if(com == NULL) {
        printf("com: NULL\n");
        return;
    }
    void *ptr = com;
    int items = HASH_COUNT(com->commands);
    int argc = com->argc;
    char *str = com->argv ? com->argv[0] : NULL;
    lt_verbosity v = com->verbosity;
    printf("com (%p)\n\titems: %d\n\targc: %d\n\targv[0]: '%s'\n\tstate: %d\n", ptr, items, argc, str, v);
    printf("\tItems are:\n");
    LT_Command *s, *tmp;
    HASH_ITER(hh, com->commands, s, tmp) {
        printf("\t\t'%s' '%s' '%s' (%p)\n", s->key, s->help, s->help_extended, s);
    }
}
