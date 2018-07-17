#include "wordsplit.h"
#include "utlist.h"
#include <ctype.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct ws_list {
    char *word;
    struct ws_list *next, *prev;
} WS_List;

char *str_iterate(char *str) {
    assert(str != NULL);
    while(*str != '\0' && isspace(*str)) str++;
    if(*str == '\0') return NULL;
    int in_quote = 0;
    int mid_word = 0;
    if(*str == '"'){
        in_quote = 1;
        str++;
    }
    for(int i = 0; str[i] != '\0' && !isspace(str[i]); i++) {
        if(str[i] == '"'){
            in_quote = 1;
            mid_word = 1;
            break;
        }
    }

    if(mid_word) {
        while(*str != '"') {
            if(*str == '\0') return NULL;
            str++;
        }
        str++;
    }
    if(in_quote) {
        while(*str != '"') {
            if(*str == '\0') return NULL;
            str++;
        }
    }
    while(!isspace(*str)) {
        if(*str == '\0') return NULL;
        str++;
    }
    while(isspace(*str)) {
        if(*str == '\0') return NULL;
        str++;
    }
    return str;
}

char *end_of_word(char *str) {
    assert(str != NULL);
    char *next_word = str_iterate(str);
    if(next_word == NULL) {
        while(*str != '\0') str++;
        return str;
    }
    next_word--;
    while(isspace(*next_word)) {
        next_word--;
    }
    next_word++;
    return next_word;
}

void shuffle_down(char *str) {
    assert(str);
    char *next = str+1;
    while(*str != '\0') {
        *str = *next;
        str++;
        next++;
    }
}

void remove_quotes(char *str) {
    assert(str);
    while(*str != '\0') {
        if(*str == '"') {
            shuffle_down(str);
        } else {
            str++;
        }
    }
}

WS_List *string_split(char *str) {
    if(str == NULL) return NULL;
    WS_List *words = NULL;
    while(isspace(*str)) str++;
    int len = strlen(str)-1;
    for(int i = strlen(str)-1; i <= 0 && isspace(str[i]); i--) {
        str[i] = '\0';
    }
    char *start = str;
    char *end;

    while(start != NULL && *start != '\0') {
        char *copy = strdup(start);
        end = end_of_word(copy);
        assert(end != NULL);
        *end = '\0';
        remove_quotes(copy);

        WS_List *item = malloc(sizeof(WS_List));
        assert(item);
        item->word = copy;
        DL_APPEND(words, item);
        start = str_iterate(start);
    }
    return words;
}

void ws_free(WS_List *list) {
    WS_List *word, *tmp;
    DL_FOREACH_SAFE(list, word, tmp) {
        DL_DELETE(list, word);
        free(word->word);
        free(word);
    }
}

int ws_split(char *str, char ***words) {
    assert(str && words);
    WS_List *ws = string_split(str);
    WS_List *tmp;
    int size = 0;
    DL_COUNT(ws, tmp, size);

    *words = malloc(sizeof(char*) * size);
    int count = 0;
    DL_FOREACH(ws, tmp) {
        (*words)[count++] = strdup(tmp->word);
    }

    ws_free(ws);
    return size;
}

int ws_len(char *str) {
    if(str == NULL) return -1;
    int count = 0;
    while((str = str_iterate(str)) != NULL) count++;
    return count;
}
