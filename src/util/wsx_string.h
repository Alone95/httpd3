//
// Created by root on 4/3/16.
//

#ifndef HTTPD3_WSX_STRING_H_H
#define HTTPD3_WSX_STRING_H_H
/// README FIRST!
/// While Using the string_t, we should use the string_t instead of String
/// Because We should initialize it before using, so keep it away if it has
/// not make by make_Strings
/// It means that using the string_t carefully
/// To be Safety,
/// call the Interface make_Strings, and use the return value
/// Destroy if it has nothing to live by using dele_Strings
/// Both the two functions is manipulate the string_t or return it.

#include <string.h>
<<<<<<< HEAD
=======

>>>>>>> db4fb4b3960fd950fa0390ad442d635688a96b54
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
