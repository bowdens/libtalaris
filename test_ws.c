#include "wordsplit.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv) {
    char *strings[] = {
        "Hello",
        "Hello There",
        "Hello There People",
        "  Hello There  ",
        "   Hello   There   ",
        "\"Hello There\"",
        "\"Hello There\" People",
        "\"   Hello   There   \"   \"   People   \"   ",
        NULL
    };

    char *e1[] = {"Hello", NULL};
    char *e2[] = {"Hello", "There", NULL};
    char *e3[] = {"Hello", "There", "People", NULL};
    char *e4[] = {"Hello", "There", NULL};
    char *e5[] = {"Hello", "There", NULL};
    char *e6[] = {"Hello There", NULL};
    char *e7[] = {"Hello There", "People", NULL};
    char *e8[] = {"   Hello   There   ", "   People   ", NULL};
    char **expected[] = {e1, e2, e3, e4, e5, e6, e7, e8, NULL};
    int lens[] = {1, 2, 3, 2, 2, 1, 2, 2, -1};
    char **words;

    for(int s = 0; strings[s] != NULL; s++) {
        printf("\nrunning on '%s'\n", strings[s]);
        int size = newest_split(strings[s], &words);

        printf("num = %d, words are: ", size);
        for(int i = 0; i < size; i++) printf("'%s'%s", words[i], i==size-1 ? "\n" : " ");
        int passed = 1;
        if(lens[s] == size) {
            for(int i = 0; i < size; i++) {
                if(strcmp(expected[s][i], words[i]) != 0) {
                    passed = 0;
                }
            }
        } else {
            passed = 0;
        }
        if(passed == 0) {
            printf("FAILED! Expected size was %d, with words ", lens[s]);
            for(int w = 0; expected[s][w] != NULL; w++) printf("'%s'%s", expected[s][w], w == lens[s]-1 ? "\n" : " ");
        } else {
            printf("'%s' succeeded\n", strings[s]);
            printf("\n");
        }
    }
}
