#pragma once
#include "defines.h"
#include <string.h>
#include <stddef.h>
#include <stdlib.h>

typedef char* string;

string stringNew(const char* init);
size_t stringLength(const string str);
size_t stringCapacity(const string str);
void stringFree(string str);
string stringCatStr(string str, const char *append);
string stringCatChar(string str, char append);
string stringDup(const string str);
string stringFmt(string str, const char *fmt, ...);
i32 stringCmp(const string a, const string b);
bool stringEq(const string a, const char *b);
void stringClear(string str);

string stringSubstring(const string str, size_t beg, size_t end);
bool stringHasPrefix(const string str, const char *prefix);

