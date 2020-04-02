/*
 Created by Tom Bowden (@bowdens, tom@bowdens.me)

    MIT License

    Copyright (c) 2018 Tom Bowden

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.

 */


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

typedef struct lt_parser LT_Parser;

typedef int(*lt_callback)(int, char**, LT_Parser*);

typedef struct lt_command {
    char *key;
    char *help;
    char *help_extended;
    lt_state state;
    lt_callback callback;
    UT_hash_handle hh;
} LT_Command;

typedef struct lt_parser {
    LT_Command *commands;
    lt_verbosity verbosity;
    lt_callback unfound;
    int argc;
    char **argv;
    char *prompt;
} LT_Parser;

LT_Parser *lt_create_parser(void);
int lt_add_commands(LT_Parser*, LT_Command*);
int lt_add_command(LT_Parser*, char*, char*, char*, lt_callback);
int lt_remove_command(LT_Parser*, char*);
LT_Command* lt_get_command(LT_Parser*, char*);
int lt_call(LT_Parser*, char*);
int lt_input(LT_Parser*, char **);
int lt_cleanup(LT_Parser*);
void lt_print_parser(LT_Parser*);
int lt_help(int, char**, LT_Parser*);

#endif
