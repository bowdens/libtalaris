#include "libtalaris.h"
#include "uthash.h"
#include "wordsplit.h"
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>

char **matching_commands = NULL;

int lt_help(int argc, char **argv, LT_Parser *parser) {
    assert(parser != NULL);
    if(argc == 1) {
        //The command 'help' only was called
        LT_Command *s, *tmp;
        HASH_ITER(hh, parser->commands, s, tmp) {
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
            LT_Command *c = lt_get_command(parser, argv[i]);
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

int lt_exit(int argc, char **argv, LT_Parser *parser) {
    //do nothing
    exit(0);
}

int lt_unfound(int argc, char **argv, LT_Parser *parser) {
    printf("The command '%s' was not found. Try typing 'help' to see a list of full commands\n", argc > 0 ? argv[0] : "");
    return 0;
}


LT_Parser *lt_create_parser(void) {
    LT_Parser *parser = malloc(sizeof(LT_Parser));
    assert(parser);
    parser->commands = NULL;
    parser->verbosity = lt_normal;

    parser->argc = 0;
    parser->argv = NULL;
    parser->prompt = "> ";

    parser->unfound = lt_unfound;

    lt_add_command(parser, "help", "Shows this help", "Usage: help [command]", lt_help);
    lt_add_command(parser, "exit", "Exits the program", "Usage: exit", lt_exit);

    return parser;
}

LT_Command *lt_get_command(LT_Parser *parser, char *command) {
    if(parser == NULL || command == NULL) return NULL;
    LT_Command *c = NULL;
    HASH_FIND_STR(parser->commands, command, c);
    return c;
}

int add_command_to_parser(LT_Parser *parser, LT_Command *command) {
    assert(parser);
    assert(command);
    if(lt_get_command(parser, command->key) != NULL) {
        if(parser->verbosity >= lt_warning) fprintf(stderr, "Warning: Could not add command '%s' because it already exists in this parser\n", command->key);
        return 1;
    }

    HASH_ADD_KEYPTR(hh, parser->commands, command->key, strlen(command->key), command);
    return 0;
}

int lt_add_command(LT_Parser *parser, char *command, char *help, char *help_extended, int (*callback)(int, char**, LT_Parser *)) {
    /*
     * Add a command to the parser
     * Cannot add two of the same command
     */
    assert(parser != NULL);

    if(command == NULL || command[0] == '\0') return 1;

    LT_Command *c = malloc(sizeof(LT_Command));
    c->key = strdup(command);
    assert(c->key);
    c->help = strdup(help);
    assert(c->help);
    c->help_extended = strdup(help_extended);
    c->callback = callback;
    c->state = LT_UNIV;

    int retval = add_command_to_parser(parser, c);
    if(retval != 0) {
        free(c->key);
        free(c->help);
        free(c->help_extended);
        free(c);
    }
    return retval;
}

int lt_add_commands(LT_Parser *parser, LT_Command *commands) {
    assert(parser);
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
        count += (add_command_to_parser(parser, c) == 0);
    }
    return count;
}

void free_command(LT_Command *c) {
    free(c->help);
    free(c->help_extended);
    free(c->key);
    free(c);
}

int lt_remove_command(LT_Parser *parser, char *command) {
    assert(parser != NULL);
    LT_Command *to_delete = lt_get_command(parser, command);
    if(to_delete == NULL) return 1;
    HASH_DEL(parser->commands, to_delete);
    free_command(to_delete);
    return 1;
}

int lt_call(LT_Parser *parser, char *str) {
    /*
     * Parses the arguments in string and
     * executes the appropriate callback
     */
    if(parser == NULL) return LT_CALL_FAILED;
    // free the old commands
    for(int i = 0; i < parser->argc; i++) {
        free(parser->argv[i]);
    }
    free(parser->argv);
    parser->argv = NULL;
    parser->argc = 0;
    if(str == NULL) return LT_CALL_FAILED;

    parser->argc = ws_split(str, &parser->argv);
    if(parser->verbosity >= lt_verbose) {
        printf("Collected %d arguments. They are:\n", parser->argc);
        for(int i = 0; i < parser->argc; i++) printf("'%s'%s", parser->argv[i], i == parser->argc-1 ? "\n" : " ");
    }

    char *command = parser->argv[0];
    LT_Command *c = lt_get_command(parser,command);
    int retval;
    if(c && LT_IS_EXEC(c->state)) {
        if(c->callback == NULL) {
            if(parser->verbosity >= lt_warning) fprintf(stderr, "Warning: Command '%s' has no callback\n", c->key);
            retval = LT_CALL_FAILED;
        } else {
            retval = c->callback(parser->argc, parser->argv, parser);
        }
    } else {
        if(parser->unfound != NULL) {
            parser->unfound(parser->argc, parser->argv, parser);
        }
        retval = LT_COMMAND_NOT_FOUND;
    }

    return retval;
}

char **generate_command_list(LT_Parser *parser) {
    //TODO: move the command list from here into the LT_Parser struct
    int count = HASH_COUNT(parser->commands);
    char **command_list = malloc(sizeof(char*) * (count+1));
    assert(command_list);
    int c = 0;
    LT_Command *s, *tmp;
    HASH_ITER(hh, parser->commands, s, tmp) {
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

int lt_input(LT_Parser *parser, char **_matching_commands) {
    /*
     * Reads from stdin and executes lt_call
     */
    if(parser == NULL) return LT_CALL_FAILED;

    char *str = NULL;

    if(_matching_commands == NULL) {
        char *tmp[] = {NULL};
        matching_commands = tmp;
    } else {
        matching_commands = _matching_commands;
    }

    rl_attempted_completion_function = command_completion;

    str = readline(parser->prompt);
    if(str == NULL) {
        free(str);
        for(int i = 0; i < parser->argc; i++) {
            free(parser->argv[i]);
        }
        free(parser->argv);
        parser->argv = NULL;
        parser->argc = 0;

        printf("\n");
        free(str);
        return LT_CALL_FAILED;
    }
    if(parser->argv == NULL || strcmp(str, parser->argv[0]) != 0) {
        add_history(str);
    }

    matching_commands = NULL;

    int retval = lt_call(parser, str);
    free(str);
    return retval;
}

int lt_cleanup(LT_Parser *parser) {
    /*
     * Free a parser
     */
    if(parser == NULL) return 0;

    for(int i = 0; i < parser->argc; i++) {
        free(parser->argv[i]);
    }

    int count = 0;
    int total = HASH_COUNT(parser->commands);
    LT_Command *tmp, *s;
    HASH_ITER(hh, parser->commands, s, tmp) {
        HASH_DEL(parser->commands, s);
        free(s->help);
        free(s->help_extended);
        free(s->key);
        free(s);
        count++;
    }

    free(parser);

    return 0;
}

void lt_print_parser(LT_Parser *parser) {
    if(parser == NULL) {
        printf("parser: NULL\n");
        return;
    }
    void *ptr = parser;
    int items = HASH_COUNT(parser->commands);
    int argc = parser->argc;
    char *str = parser->argv ? parser->argv[0] : NULL;
    lt_verbosity v = parser->verbosity;
    printf("parser (%p)\n\titems: %d\n\targc: %d\n\targv[0]: '%s'\n\tstate: %d\n", ptr, items, argc, str, v);
    printf("\tItems are:\n");
    LT_Command *s, *tmp;
    HASH_ITER(hh, parser->commands, s, tmp) {
        printf("\t\t'%s' '%s' '%s' (%p)\n", s->key, s->help, s->help_extended, s);
    }
}
