#include "libtalaris.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

int echo(int argc, char **argv, LT_Parser *caller) {
    for(int i = 1; i < argc; i++) {
        printf("%s%s", argv[i], i != argc-1 ? " " : "\n");
    }
    return 0;
}

int cat(int argc, char **argv, LT_Parser *caller) {
    for(int i = 1; i < argc; i++) {
        FILE *fp = fopen(argv[i], "r");
        if(fp == NULL) continue;
        char buffer[1024];
        while(fgets(buffer, 1024, fp) != NULL) {
            printf("%s", buffer);
        }
    }
    return 0;
}

int quiet(int argc, char **argv, LT_Parser *caller) {
    printf("THIS IS QUIET\n");
    return 0;
}

int secret(int argc, char**argv, LT_Parser *caller) {
    printf("THIS IS A SECRET!\n");
    return 0;
}

int silent(int argc, char **argv, LT_Parser *caller) {
    printf("YOU SHOULD NEVER SEE THIS\n");
    return 0;
}

int is_num(char *str) {
    char *tmp;
    long val = strtol(str, &tmp, 10);

    return !((tmp == str) || (*tmp !='\0'));
}

int add(int argc, char **argv, LT_Parser *caller) {
    int sum = 0;
    for(int i = 1; i < argc; i++) {
        sum += atoi(argv[i]);
    }
    return sum;
}

int mul(int argc, char **argv, LT_Parser *caller) {
    int sum = 1;
    for(int i = 1; i < argc; i++) {
        if(is_num(argv[i])) sum *= atoi(argv[i]);
    }
    return sum;
}

int set(int argc, char **argv, LT_Parser *caller) {
    if(argc < 2) {
        return 0;
    }
    if(is_num(argv[1])) return atoi(argv[1]);
    printf("That was not a valid integer!\n");
    return INT32_MAX;
}

int exit_math(int argc, char **argv, LT_Parser *caller) {
    return LT_CALL_FAILED;
}

int math(int argc, char **argv, LT_Parser *caller) {
    LT_Parser *mathparser = lt_create_parser();
    LT_Command mathcoms[] = {
        {"add", "adds integers", "Usage: add INTEGER [INTEGER]...", LT_UNIV, add, NULL},
        {"sub", "subtracts integers", "Usage: sub INTEGER [INTEGER]...", LT_UNIV, add, NULL},
        {"mul", "multiplies integers", "Usage: mul INTEGER [INTEGER]...", LT_UNIV, mul, NULL},
        {"div", "divides integers", "Usage: div INTEGER [INTEGER]...", LT_UNIV, mul, NULL},
        {"reset", "sets the current stored value (default 0)", "Usage: reset [INTEGER]", LT_UNIV, set, NULL},
        {0}
    };
    char *matches[] = {"add", "sub", "mul", "div", "reset", NULL};
    lt_add_commands(mathparser, mathcoms);
    lt_get_command(mathparser, "exit")->callback = exit_math;

    char prompt[128];
    int total = 0;
    int val = 0;
    snprintf(prompt, 128, "%d\n# ", total);
    mathparser->prompt = prompt;
    while((val = lt_input(mathparser, matches)) != LT_CALL_FAILED) {
        if(val == INT32_MAX || mathparser->argc == 0 || val == LT_COMMAND_NOT_FOUND) continue;

        int old_total = total;
        char operation = '?';

        if(strncmp(mathparser->argv[0], "add", 4) == 0) {
            total += val;
            operation = '+';
        } else if(strncmp(mathparser->argv[0], "sub", 4) == 0) {
            total -= val;
            operation = '-';
        } else if(strncmp(mathparser->argv[0], "mul", 4) == 0) {
            total *= val;
            operation = '*';
        } else if(strncmp(mathparser->argv[0], "div", 4) == 0) {
            total /= val ? val : 1;
            operation = '/';
        } else if(strncmp(mathparser->argv[0], "reset", 4) == 0) {
            total = val;
        }

        if(operation == '?') {
            snprintf(prompt, 128, "%d\n# ", total);
        } else {
            snprintf(prompt, 128, "%d %c %d = %d\n# ", old_total, operation, val, total);
        }
        mathparser->prompt = prompt;
    }

    printf("Exiting mathematics mode\n");
    lt_cleanup(mathparser);
    return 0;
}

int mainexit(int argc, char **argv, LT_Parser *parser) {
    return LT_CALL_FAILED;
}

int arguments(int argc, char **argv, LT_Parser *parser) {
    for(int i = 0; i < argc; i++) {
        printf("\"%s\"%s", argv[i], i < argc-1 ? " " : "\n");
    }
}

#ifndef WINNT
int exec(int argc, char **argv, LT_Parser *parser) {
    if (argc < 2) {
        printf("You must specify a binary\n");
        return 0;
    }
    if (fork() == 0) {
        char *envp = {NULL};
        int res = execve(argv[1], &argv[1], &envp);
        if (res == -1) {
            perror("exec");
        }
        exit(errno);
    }
    wait(NULL);
    return 0;
}
#endif

int main(void) {
    LT_Parser *parser = lt_create_parser();
    LT_Command commands[] = {
        {"echo", "Echos whatever you write", "Usage: echo [WORD]...", LT_UNIV, echo,  NULL},
        {"cat", "Prints the contents of whichever file(s) you specify", "Usage: cat [FILE]...", LT_UNIV, cat, NULL},
        {"math", "Enters mathematics mode", "Usage: math", LT_UNIV, math, NULL},
        {"args", "Prints out each argument", "Usage: args [WORD]...", LT_UNIV, arguments, NULL},
        {"quiet", "This is a quiet command. You can't see it in help, but you can if you run `help quiet`, and you can run it", "Usage: quiet", LT_EXEC | LT_SPEC, quiet, NULL},
        {"secret", "This is a secret command. It does not show up in help, but you can run it", "Usage: secret", LT_EXEC, secret, NULL},
        {"silent", "This is a silent command. It does not show up in help, and you can not run it", "Usage: silent", LT_HIDE, silent, NULL},
        {"?", "A link to help", "Usage: ? [COMMAND]...", LT_EXEC | LT_SPEC, lt_help, NULL},
#ifndef WINNT        
        {"exec", "execute a binary", "Usage: exec [BINARY]", LT_UNIV, exec, NULL},
#endif
        {0}
    };


    char *matches[] = {"echo", 
                        "cat", 
                        "quiet", 
                        "help", 
                        "exit", 
                        "math", 
                        "args", 
#ifndef WINNT        
                        "exec", 
#endif                        
                        NULL};

    lt_add_commands(parser, commands);

    lt_get_command(parser, "exit")->callback = mainexit;

    int val = 0;
    do {
        val = lt_input(parser, matches);
    } while(val != LT_CALL_FAILED);
    lt_cleanup(parser);
    return 0;
}
