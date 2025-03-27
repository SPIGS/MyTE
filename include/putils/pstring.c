#include "pstring.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdatomic.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    size_t length;
    size_t capacity;
} pStringHeader;


string stringNew(const char* init) {
    size_t length = (init == NULL) ? 0 : strlen(init);

    size_t capacity = 1;
    while (capacity < length + 1) {capacity *= 2;};

    pStringHeader *header = (pStringHeader*)malloc(sizeof(pStringHeader) + sizeof(char) * capacity);
    if (!header) return NULL;

    header->length = length;
    header->capacity = capacity;

    string str = (char *)(header + 1);

    if (init) {
        memcpy(str, init, length);
    }

    str[length] = '\0';
    return str;
}

static inline pStringHeader *stringHeader(const char* str) {
  return ((pStringHeader *)str - 1);
}

size_t stringLength(const string str) {
  return stringHeader(str)->length;
}

size_t stringCapacity(const string str) {
  return stringHeader(str)->capacity;
}

void stringFree(string str) {
  if (str)
    free(stringHeader(str));
}

static char *stringResize(string str, size_t new_cap) {
  if (!str) return NULL;

  pStringHeader *old_header = stringHeader(str);

  if (new_cap == old_header->capacity) return str;

  if (new_cap <= old_header->length) {
    new_cap = old_header->length + 1;
  }

  pStringHeader *new_header = realloc(old_header, sizeof(pStringHeader) + new_cap);
  if (!new_header) return NULL;

  new_header->capacity = new_cap;
  return (char*)(new_header + 1);
}

static string pStringGrow (string str, size_t add_len) {
  if (!str) return NULL;

  pStringHeader *header = stringHeader(str);
  size_t required = header->length + add_len + 1;

  if (required > header->capacity) {
    size_t new_cap = header->capacity * 2;
    if (new_cap < required) {
      new_cap = required;
    }

    string new_str = stringResize(str, required);
    if (!new_str) return NULL;

    return new_str;
  }
  return str;
}

string stringCatStr(string str, const char *append) {
  if (!str || !append) return str;

  size_t append_len = strlen(append);
  if (append_len == 0) return str;

  pStringHeader *header = stringHeader(str);

  str = pStringGrow(str, append_len);
  if (!str) return NULL;

  header = stringHeader(str);
  memcpy(str + header->length, append, append_len);

  header->length += append_len;
  str[header->length] = '\0';

  return str;
}

string stringCatChar(string str, char append) {
  if (!str) return NULL;
  // Use a small buffer to avoid calling sds_cat with a whole string
  char buf[2] = {append, '\0'};
  return stringCatStr(str, buf);
}

string stringDup(const string str) {
  if (!str) return NULL;

  return stringNew(str);
}

string stringFmt(string str, const char *fmt, ...) {
  va_list ap, cpy;
  va_start(ap, fmt);
  va_copy(cpy, ap);

  int required = vsnprintf(NULL, 0, fmt, ap) + 1;
  va_end(ap);

  if (required <= 0) {
    va_end(cpy);
    return NULL;
  }

  if (!str) {
    str = stringNew("");
    if (!str) {
      va_end(cpy);
      return NULL;
    }
  }

  str = pStringGrow(str, required);
  if (!str) {
    va_end(cpy);
    return NULL;
  }

  pStringHeader *header = stringHeader(str);

  vsnprintf(str, header->capacity, fmt, cpy);
  va_end(cpy);

  header->length = strlen(str);

  return str;
}

 
int stringCmp(const string a, const string b) {
  if (!a || !b) {
    if (!a && !b) return 0;
    return !a ? -1 : 1;
  }

  return strcmp(a,b);
}

bool stringEq(const string a, const char *b) {
  // both null is equal
  if (!a || !b) {
    return (!a && !b);
  }

  size_t len_a = stringLength(a);
  size_t len_b = strlen(b);

  if (len_a != len_b) {
    return false;
  }

  return memcmp(a, b, len_a) == 0;
}


string stringSubstring(const string str, size_t idx, size_t len) {
  if (!str || len == 0) return NULL;
  if (idx >= stringLength(str)) return NULL;
  
  string sub = stringNew("");
  sub = pStringGrow(sub, len);
  if (!sub) return NULL;

  pStringHeader *header = stringHeader(sub);
  memcpy(sub + header->length, str + idx, len);
  header->length += len;
  sub[header->length] = '\0';
  return sub;
}

void stringClear(string str) {
    if (!str) return;
    
    pStringHeader* header = stringHeader(str);
    header->length = 0;
    str[0] = '\0';
}
