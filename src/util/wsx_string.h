//
// Created by root on 4/3/16.
//

#ifndef HTTPD3_WSX_STRING_H_H
#define HTTPD3_WSX_STRING_H_H

#include <string.h>

#define APPEND(str) str,(strlen(str)+1)
typedef char boolen;
typedef struct funtable funtable;

typedef struct String {
    char *       str; /* Store the string characters */
    funtable *   use; /* Function Storage-Area */
    unsigned int cap; /*  */
    unsigned int len; /* String Length */
    boolen       empty;

}String;

typedef boolen       (*isempty_fun)(String *);
typedef unsigned int (*get_capacity_fun)(String *);
typedef unsigned int (*get_length_fun)(String *);
/* self appendString appendStringLength */
typedef char * (*make_append)(String *, const char *, unsigned int);
/* Copy self to dst */
typedef boolen (*make_copy)(String * self, String * dst);
/* Move self to dst */
typedef boolen (*make_move)(String * self, String * dst);
/* Init could be NULL or other meaningful C-Style string */
typedef void (*constructor)(String *, const char * init);
/* clear the String , it does not mean destroy it, just clear the string. */
typedef void (*make_clear)(String *);
typedef void (*destructor)(String *);
typedef char * (*find_substr)(String *, const char *);
#if defined(WSX_DEBUG)
typedef void (*print_message)(String *);
#endif
struct funtable {
    isempty_fun      is_empty;
    get_capacity_fun capacity;
    get_length_fun   length;
    make_append      append;
    make_clear       clear;
    make_copy        copy_to;
    make_move        move_to;
    constructor      init;
    destructor       destroy;
    find_substr      has;
#if defined(WSX_DEBUG)
    print_message    print;
#endif
};

typedef String* string_t;
/* Make a String Object */
string_t make_Strings(const char * init_str);
/* Destroy the String which make from make_Strings */
void dele_Strings(string_t deleter);

#endif //HTTPD3_WSX_STRING_H_H
