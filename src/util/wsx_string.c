//
// Created by root on 4/3/16.
//

#include "wsx_string.h"
#include "../memop/manage.h"
#include <assert.h>
#include <limits.h>

#define WSX_STR_CAP_DEFAULT 32

static inline boolen       expand_size(String * self);
static inline boolen       Str_isempty_(String * self);
static inline unsigned int Str_get_capacity_fun(String * self);
static inline unsigned int Str_get_length_fun(String * self);
static inline void         Str_make_clear(String * self);
static char *       Str_make_append(String * self, const char * app_str, unsigned int str_len);
static boolen       Str_make_copy(String * self, String * dst);
static boolen       Str_make_move(String * self, String * dst);
static void         Str_constructor(String * self, const char * init_str);
static void         Str_destructor(String * self);
static inline char *       Str_find_substr(String * self, const char * to_be_find);
#if defined(WSX_DEBUG)
static inline void         Str_print(string_t self);
#endif
static funtable function_area =
        {
                .is_empty = Str_isempty_,
                .capacity = Str_get_capacity_fun,
                .length   = Str_get_length_fun,
                .append   = Str_make_append,
                .clear    = Str_make_clear,
                .copy_to  = Str_make_copy,
                .move_to  = Str_make_move,
                .init     = Str_constructor,
                .destroy  = Str_destructor,
                .has      = Str_find_substr,
#if defined(WSX_DEBUG)
                .print    = Str_print,
#endif
        };

static void Str_constructor(String * restrict self, const char * restrict init_str)
{
    if (NULL == self)
        assert(0);
    if (NULL == init_str)
        init_str = "";
    unsigned int init_str_len = strlen(init_str);
    self->cap   = WSX_STR_CAP_DEFAULT > init_str_len ? WSX_STR_CAP_DEFAULT : (init_str_len + 1);
    self->empty = 1;
    self->len   = 0;
    self->str   = NULL;
    expand_size(self);
    unsigned int len = strlen(init_str);
    strncpy(self->str, init_str, len);
    self->len = len;
    return;
}

static void Str_destructor(String * self)
{
    if (NULL == self) {
        return;
    }
    Free(self->str);
    self->use = NULL;
}

static inline boolen Str_isempty_(String * self)
{
    return (self->empty == 1) ? 1 : 0;
}

static inline unsigned int Str_get_capacity_fun(String * self)
{
    return self->cap;
}

static inline unsigned int Str_get_length_fun(String * self)
{
    return self->len;
}

static inline void Str_make_clear(String * self)
{
    (self->str)[0] = '\0';
    self->len = 0;
    self->empty = 1;
}

static boolen Str_make_copy(String * self, String * dst)
{
    if (self == dst) return 1;
    if (NULL == self || NULL == dst) return 0;
    unsigned int capacity = dst->cap;
    /* Try to make expansion until the Area size is big enough to fill the self->str
     * or Use out of the Memory we can manage in Heap.
     * */
    while (capacity < self->len) {
        expand_size(dst);
        capacity = dst->cap;
    }
    strncpy(dst->str, self->str, self->len);
    dst->len = self->len;
    dst->empty = 0;
    return 1;
}

static boolen Str_make_move(String * self, String * dst) {
    if (self == dst) return 1;
    if (NULL == self || NULL == dst) assert(0);
    memcpy(dst, self, sizeof(String)); /* Move the self to the dst */
    memset(self, 0, sizeof(String));   /* Clear the self For the safe */
}

static inline char * Str_find_substr(String * restrict self, const char * restrict to_be_find)
{
    if (NULL == to_be_find) return NULL;
    if (0 == strlen(to_be_find)) return NULL;
    return strstr(self->str, to_be_find);
}

static char * Str_make_append(String * restrict self, const char * restrict app_str, unsigned int str_len)
{
    unsigned int capcity = self->cap;
    unsigned int len     = self->len;
    boolen error;
    while (capcity - len < str_len + 1) {
        error = expand_size(self);
        capcity = self->cap;
    }
    strncat(self->str, app_str, str_len);
    self->len += str_len;
    if (self->str[self->len - 1] != '\0') {
        self->str[self->len] = '\0';
        self->len += 1;
    }
}

static inline boolen expand_size(String * self) {
    unsigned int capacity = self->cap;
    if (UINT_MAX == capacity) assert(0);
    unsigned int comfortable = WSX_STR_CAP_DEFAULT * 2 - 1; /* 0x1F */
    while ( comfortable <= capacity && comfortable != UINT_MAX) {
        comfortable <<= 1;
        comfortable |= 1;
    }
    capacity = comfortable;
    char * tmp = Malloc(capacity);
    if (NULL == tmp) assert(0);
    strncpy(tmp, self->str, self->len);
    Free(self->str);
    self->str = tmp;
    self->cap = capacity;
    return 1;
}

string_t make_Strings(const char * init_str) {
    string_t ret = Malloc(sizeof(String));
    ret->use = &function_area;
    ret->use->init(ret, init_str);
    return ret;
}

void dele_Strings(string_t deleter) {
    deleter->use->destroy(deleter);
    Free(deleter);
}

int wsx_rstrncmp(const char * str1, const char * str2, int nbytes) {
    if (nbytes < 0)
        return -1;
    if (NULL == str1 || NULL == str2)
        return -2;
    int str1_byte = strlen(str1);
    int str2_byte = strlen(str2);
    if (str1_byte < nbytes || str2_byte < nbytes)
        return -3;
    for (int i = 0; i < nbytes; ++i) {
        if (str1[--str1_byte] != str2[--str2_byte])
            return 1;
    }
    return 0;
}

#if defined(WSX_DEBUG)
static inline void Str_print(string_t self)
{
    fprintf(stderr, "String:\"%s\"\t length:%d \t capaticy: %d\n", self->str, self->len, self->cap);
}
#endif