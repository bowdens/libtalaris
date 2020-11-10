# libtalaris

Libtalaris is a C library that makes adding interactive command line interfaces easy. An example program is included in example.c

## Installation

Clone this repository using `git clone https://www.github.com/bowdens/libtalaris`
Run `make` to compile the example, then `./example` to test it out.

You can also run `make libtalaris.a` to create the .a file, which you can copy across to your project to easily use the library for your project; just remember to add the include at the top of your .c file(s) `#include "libtalaris.h"`.

Note: Since this library uses `readline.h`, you will need to include the -lreadline flag when compiling your project if you use libtaralis.

MinGW Note: Because MinGW does not include any of the *nix `sys` headers, a drop-in replacement is included in the `windeps` folder in this distribution. Windows also lacks `fork()`, so the code in the example that uses this is disabled when compiling with on Windows.

## Usage

Make sure to add the line `#include "libtaralis.h" in your .c file after copying libtalaris.a into your folder. Also don't forget to use the -lreadline flag when compiling in gcc.

#### Parsers
Each program needs to have at least one `LT_parser` object. Create this with the line 
```c
LT_Parser *parser = lt_create_parser();
```

#### Adding commands
You can add commands to each particular `LT_Parser` object with the `lt_add_commands` function. First create an array of `LT_command`s like this:
```c
LT_Command commands[] = {
  {"Command Name", "Command Description" "Extra Command Description", LT_UNIV, callback_function, NULL},
  {"Command Name2", "Command Description2" "Extra Command Description2", LT_UNIV, callback_function2, NULL},
  {0}
}

lt_add_commands(parser, commands);
```

Each parser has 2 default commands: exit, which will call `exit(0)`, and help, which will print all shown commands (see the state flags section for more).
The default commands can be removed with `
```c
lt_remove_command(LT_Parser *parser, char *command)
```
, or the callback can be changed by using `lt_get_command(LT_Parser *parser, char *command);` like this:
```c
lt_get_command(parser, "exit")->callback = different_function;
```

See below for more information on the `STATE_FLAGS` (such as `LT_UNIV`) and the callback functions.

#### Executing commands

Executing commands is easy. After adding commands, simply call `lt_input(LT_Parser *parser, char *matches)`. 
Libtalaris will accept input from `stdin` and execute the appropriate command based on what the user entered.

For example, to continuously accept user input, this code fragment can be used:
```c
while(lt_input(commander, NULL) != LT_CALL_FAILED);
```
If an unknown command was entered, libtalaris will execute whatever callback function is given at `parser->unfound`. By default this prints a message advising the user to type help, but it can be changed by changing the function pointer associated with parser->unfound.

`lt_input` will return `LT_CALL_FAILED` if the `LT_Parser` is `NULL`, or if the end of input was reached (ie `Control-D` was pressed)
It will also return `LT_COMMAND_UNFOUND` if there was no command associated with what the user entered.
`lt_input` will otherwise return whatever the callback function returns otherwise. So it is a good idea to avoid returning `LT_COMMAND_UNFOUND` and `LT_CALL_FAILED` (#defined to -99 and -98) in your callbacks.

You can also you `lt_call(LT_Parser, string)` to execute a command in the same way as if the user typed in the string.

#### Callbacks
Each command should have a callback function associated with it (if it is set to `NULL`, nothing will be executed when the user enters that command.

Callbacks must have the following prototype:
```c
int callback(int argc, char **argv, LT_Parser *parser);
```

When the command is executed by `lt_input` or `lt_call`, it will pass the number of arguments in argc, and the arguments themselves in argv in the exact same was as it works in `main(int argc, char **argv);`.
A pointer to the parser that executed the callback is also passed in, for flexibility if you are using multiple parsers and need to change the functionality of the callback depending on which parser called it.

Whatever is returned by the callback will be returned by `lt_call` or `lt_input`. This can be used for error catching. You should avoid returning -98 and -99 because that is what `LT_COMMAND_UNFOUND` and `LT_CALL_FAILED` is #defined to

Examples of callback functions are provided in the example.c file

#### State flags
There are three bits in the help flag. It determines what the default help function will show, and whether `lt_call` will execute the command when entered.
The left most bit determines whether the default help function will show the command in the list, ie when then user types `help`.
The next bit determines whether the default help function will show the extended help when the user types `help command`. If this is set to zero, `help command` will print "command not found"
The final bit determines whether `lt_call` will execute the command when it is entered by the user. If a command which has this bit set to zero is entered, it will act as if the command was unknown.

For most commands you'll probably want all bits set, so it will show help and extended help, and allow for execution. For this reason, this is #defined as `LT_UNIV`, with all three bits set.
To access each bit, you can use `LT_HELP`, `LT_SPEC`, and `LT_EXEC` for help, extended help, and execution respectively. You can also use `LT_HIDE` for a state flag with all bits set to zero

For instance, if you wanted a command that would show extended help and can be executed, you bitwise OR the bits like this:
```c
{"command", "Cannot be seen in help", "But will show this extended help if 'help command' is run", LT_SPEC | LT_EXEC, callback, NULL}
```



This is a revamped version of [input-handler](https://www.github.com/bowdens/input-handler), created by @bowdens
